#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <random>
#include <sstream>
#include <memory>
#include <algorithm>
#include <iomanip>
#include <chrono>
#include <cmath>

// -------------------------------------------------------------------
// 1. Configuration for Automaton Generation
// -------------------------------------------------------------------
struct AutomatonConfig {
    size_t min_states = 4;
    size_t max_states = 6;
    std::vector<std::string> alphabet;
    double transition_density = 0.25;
    bool allow_epsilon = false;
    bool allow_self_loops = true;
    double self_loop_probability = 0.2;
    bool allow_bidirectional = false;
    double bidirectional_probability = 0.0;
};

// -------------------------------------------------------------------
// 2. State Class
// -------------------------------------------------------------------
class State {
public:
    std::string id;
    bool is_start = false;
    bool is_final = false;

    State(std::string id, bool start = false, bool final = false)
        : id(std::move(id)), is_start(start), is_final(final) {}

    std::string getNeo4jType() const {
        if (is_start && is_final) return ":START_FINAL_STATE";
        if (is_start) return ":START_STATE";
        if (is_final) return ":FINAL_STATE";
        return ":STATE";
    }
};

// -------------------------------------------------------------------
// 3. Complex Automaton Class
// -------------------------------------------------------------------
class ComplexAutomaton {
private:
    std::string id_;
    std::vector<State> states_;
    std::map<std::string, std::map<std::string, std::set<std::string>>> delta_;

public:
    explicit ComplexAutomaton(std::string id) : id_(std::move(id)) {}

    const std::string& getId() const { return id_; }
    const std::vector<State>& getStates() const { return states_; }
    const auto& getDelta() const { return delta_; }

    void generateRandom(const AutomatonConfig& config, std::mt19937& rng) {
        size_t num_states = config.min_states;

        states_.clear();
        delta_.clear();

        // Step 1: Instantiate states
        for (size_t i = 0; i < num_states; ++i) {
            std::string state_id = "q" + std::to_string(i);
            bool is_start = (i == 0);
            states_.emplace_back(state_id, is_start, false);
        }

        // Step 2: Set final accepting states (~15% of total states)
        if (num_states > 1) {
            size_t num_final = std::max(size_t(1), static_cast<size_t>(num_states * 0.15));
            std::vector<size_t> non_start_indices;
            for (size_t i = 1; i < num_states; ++i) {
                non_start_indices.push_back(i);
            }
            
            std::shuffle(non_start_indices.begin(), non_start_indices.end(), rng);
            for (size_t i = 0; i < num_final && i < non_start_indices.size(); ++i) {
                states_[non_start_indices[i]].is_final = true;
            }
        }

        // Step 3: Construct linear backbone to guarantee reachability
        // (q0 -> q1 -> ... -> qN-1)
        for (size_t i = 0; i < num_states - 1; ++i) {
            std::uniform_int_distribution<size_t> sym_dist(0, config.alphabet.size() - 1);
            std::string sym = config.alphabet[sym_dist(rng)];
            delta_[states_[i].id][sym].insert(states_[i + 1].id);
        }

        // Step 4: Stochastic edge generation based on transition density
        std::uniform_real_distribution<double> prob_dist(0.0, 1.0);
        std::uniform_int_distribution<size_t> target_dist(0, num_states - 1);

        for (const auto& from_state : states_) {
            for (const auto& symbol : config.alphabet) {
                if (prob_dist(rng) < config.transition_density) {
                    const auto& to_state = states_[target_dist(rng)];
                    delta_[from_state.id][symbol].insert(to_state.id);
                }
            }
        }

        // Step 5: Add self-loops stochastically
        if (config.allow_self_loops) {
            std::uniform_real_distribution<double> loop_prob(0.0, 1.0);
            std::uniform_int_distribution<size_t> sym_dist(0, config.alphabet.size() - 1);
            
            for (const auto& from_state : states_) {
                if (loop_prob(rng) < config.self_loop_probability) {
                    std::string sym = config.alphabet[sym_dist(rng)];
                    delta_[from_state.id][sym].insert(from_state.id);
                }
            }
        }
    }

    std::map<std::string, std::map<std::string, std::set<std::string>>>
    getGroupedTransitions() const {
        std::map<std::string, std::map<std::string, std::set<std::string>>> grouped;
        
        for (const auto& [from_id, sym_map] : delta_) {
            for (const auto& [symbol, targets] : sym_map) {
                for (const auto& to_id : targets) {
                    grouped[from_id][to_id].insert(symbol);
                }
            }
        }
        return grouped;
    }

    // Helper to get metrics for a single automaton
    struct Metrics {
        size_t num_states;
        size_t num_transitions;
        size_t num_final;
        double avg_degree;
    };

    Metrics getMetrics() const {
        size_t final_count = 0;
        for (const auto& st : states_) {
            if (st.is_final) final_count++;
        }

        size_t total_transitions = 0;
        for (const auto& [from_id, sym_map] : delta_) {
            for (const auto& [sym, targets] : sym_map) {
                total_transitions += targets.size();
            }
        }

        double avg_degree = static_cast<double>(total_transitions) / states_.size();
        return {states_.size(), total_transitions, final_count, avg_degree};
    }
};

// -------------------------------------------------------------------
// 4. Cypher Exporter Class
// -------------------------------------------------------------------
class CypherExporter {
public:
    static std::string exportToCypher(const std::vector<ComplexAutomaton>& automata) {
        std::stringstream ss;

        ss << "// ===================================================================\n";
        ss << "// GENERATED CYPHER SCRIPT FOR AUTOMATA INGESTION IN NEO4J\n";
        ss << "// Number of automata: " << automata.size() << "\n";
        ss << "// ===================================================================\n\n";
        ss << "MATCH (n) DETACH DELETE n;\n\n";

        for (const auto& aut : automata) {
            ss << "// --- Automaton: " << aut.getId() << " ---\n";

            std::string start_node_var = aut.getId() + "_start_indicator";
            ss << "CREATE (" << start_node_var << ":START_NODE {name: 'start', automaton_id: '"
               << aut.getId() << "'});\n";

            for (const auto& st : aut.getStates()) {
                std::string var_name = aut.getId() + "_" + st.id;
                ss << "CREATE (" << var_name << st.getNeo4jType()
                   << " {name: '" << st.id << "', automaton_id: '" << aut.getId() << "'});\n";
            }

            for (const auto& st : aut.getStates()) {
                if (st.is_start) {
                    ss << "MATCH (from:START_NODE {automaton_id: '" << aut.getId() << "'}), "
                       << "(to {name: '" << st.id << "', automaton_id: '" << aut.getId() << "'})\n";
                    ss << "CREATE (from)-[:TRANSITION {symbols: 'Start', automaton_id: '"
                       << aut.getId() << "'}]->(to);\n";
                    break;
                }
            }

            auto grouped = aut.getGroupedTransitions();
            for (const auto& [from_id, to_map] : grouped) {
                for (const auto& [to_id, symbols] : to_map) {
                    std::string symbols_str;
                    for (auto it = symbols.begin(); it != symbols.end(); ++it) {
                        if (it != symbols.begin()) symbols_str += ",";
                        symbols_str += *it;
                    }
                    
                    bool is_self_loop = (from_id == to_id);
                    
                    ss << "MATCH (from {name: '" << from_id << "', automaton_id: '"
                       << aut.getId() << "'}), "
                       << "(to {name: '" << to_id << "', automaton_id: '" << aut.getId() << "'})\n";
                    ss << "CREATE (from)-[:TRANSITION {symbols: '" << symbols_str
                       << "', automaton_id: '" << aut.getId() << "'";
                    if (is_self_loop) {
                        ss << ", type: 'self-loop'";
                    }
                    ss << "}]->(to);\n";
                }
            }
        }
        return ss.str();
    }
};

// -------------------------------------------------------------------
// Helper: Format number with thousands separator
// -------------------------------------------------------------------
std::string formatNumber(size_t number) {
    std::string str = std::to_string(number);
    int insert_pos = static_cast<int>(str.length()) - 3;
    while (insert_pos > 0) {
        str.insert(insert_pos, ",");
        insert_pos -= 3;
    }
    return str;
}

// -------------------------------------------------------------------
// Helper: Compute mean and std dev
// -------------------------------------------------------------------
struct Stats {
    double mean;
    double stddev;
};

Stats computeStats(const std::vector<double>& values) {
    if (values.empty()) return {0.0, 0.0};
    
    double sum = 0.0;
    for (double v : values) sum += v;
    double mean = sum / values.size();
    
    double sq_sum = 0.0;
    for (double v : values) sq_sum += (v - mean) * (v - mean);
    double stddev = std::sqrt(sq_sum / values.size());
    
    return {mean, stddev};
}

// -------------------------------------------------------------------
// Struct to store results for each scale
// -------------------------------------------------------------------
struct ScaleResults {
    std::string scale_label;
    std::vector<size_t> states;
    std::vector<size_t> transitions;
    std::vector<size_t> finals;
    std::vector<double> avg_degrees;
};

// -------------------------------------------------------------------
// 5. Main Function
// -------------------------------------------------------------------
int main(int argc, char* argv[]) {
    std::cout << "===================================================================\n";
    std::cout << "   Automaton Benchmark Generator & Topological Profiler (C++17)   \n";
    std::cout << "===================================================================\n\n";

    // --- Parse command line arguments ---
    int num_replications = 20;  // Default
    if (argc > 1) {
        num_replications = std::atoi(argv[1]);
        if (num_replications < 1) num_replications = 1;
        if (num_replications > 1000) num_replications = 1000;
    }
    std::cout << "Number of replications per scale: " << num_replications << "\n\n";

    // --- Configuration ---
    const std::vector<size_t> target_scales = {10, 100, 1000, 5000, 10000};
    const std::vector<std::string> alphabet = {"a", "b"};
    
    // Generate a fixed set of seeds for reproducibility
    std::vector<unsigned int> seeds;
    for (int i = 0; i < num_replications; ++i) {
        seeds.push_back(1000 + i); // 1000, 1001, 1002, ...
    }

    // Store results for each scale
    std::map<size_t, ScaleResults> all_results;

    // --- Run experiments ---
    std::cout << "Generating automata...\n" << std::flush;
    
    for (size_t scale : target_scales) {
        ScaleResults scale_results;
        scale_results.scale_label = "NFA-" + std::to_string(scale);
        
        std::cout << "  Scale " << scale << ": " << std::flush;
        
        for (int rep = 0; rep < num_replications; ++rep) {
            // Each replication uses a different seed
            std::mt19937 rng(seeds[rep]);
            
            AutomatonConfig config;
            config.min_states = scale;
            config.max_states = scale;
            config.alphabet = alphabet;
            config.transition_density = 0.25;
            config.allow_epsilon = false;
            config.allow_self_loops = true;
            config.self_loop_probability = 0.2;
            config.allow_bidirectional = false;

            std::string id = "NFA-" + std::to_string(scale) + "-" + std::to_string(rep);
            ComplexAutomaton aut(id);
            aut.generateRandom(config, rng);
            
            auto metrics = aut.getMetrics();
            scale_results.states.push_back(metrics.num_states);
            scale_results.transitions.push_back(metrics.num_transitions);
            scale_results.finals.push_back(metrics.num_final);
            scale_results.avg_degrees.push_back(metrics.avg_degree);
            
            // Progress indicator
            if ((rep + 1) % 10 == 0 || rep == num_replications - 1) {
                std::cout << "." << std::flush;
            }
        }
        std::cout << " done (" << num_replications << ")\n";
        
        all_results[scale] = std::move(scale_results);
    }

    // --- Compute statistics ---
    std::cout << "\n===================================================================\n";
    std::cout << "                    AGGREGATED RESULTS\n";
    std::cout << "===================================================================\n\n";
    
    std::cout << std::left 
              << std::setw(18) << "Scale"
              << std::setw(15) << "States"
              << std::setw(25) << "Transitions (mean ± std)"
              << std::setw(25) << "Final States"
              << std::setw(18) << "Avg Degree" << "\n";
    std::cout << std::string(101, '-') << "\n";

    // Store aggregated data for LaTeX table
    std::vector<ScaleResults> all_aggregated;
    
    for (size_t scale : target_scales) {
        const auto& res = all_results[scale];
        
        // Convert to double for statistics
        std::vector<double> trans_double(res.transitions.begin(), res.transitions.end());
        std::vector<double> final_double(res.finals.begin(), res.finals.end());
        std::vector<double> degree_double(res.avg_degrees.begin(), res.avg_degrees.end());
        
        Stats trans_stats = computeStats(trans_double);
        Stats final_stats = computeStats(final_double);
        Stats degree_stats = computeStats(degree_double);
        
        // States are constant, so no std dev needed
        size_t states = res.states.empty() ? 0 : res.states[0];
        
        std::cout << std::left 
                  << std::setw(18) << res.scale_label
                  << std::setw(15) << states
                  << std::setw(25) << (std::to_string(static_cast<int>(trans_stats.mean)) + " ± " + 
                                       std::to_string(static_cast<int>(trans_stats.stddev)))
                  << std::setw(25) << (std::to_string(static_cast<int>(final_stats.mean)) + " ± " + 
                                       std::to_string(static_cast<int>(final_stats.stddev)))
                  << std::setw(18) << (std::to_string(degree_stats.mean).substr(0, 4) + " ± " + 
                                       std::to_string(degree_stats.stddev).substr(0, 4)) << "\n";
    }
    
    std::cout << std::string(101, '-') << "\n\n";

    // --- Export aggregated results to CSV ---
    std::ofstream csv_file("benchmark_results.csv");
    if (csv_file.is_open()) {
        csv_file << "scale,replication,states,transitions,final_states,avg_degree\n";
        for (size_t scale : target_scales) {
            const auto& res = all_results[scale];
            for (size_t i = 0; i < res.states.size(); ++i) {
                csv_file << scale << "," 
                         << i << ","
                         << res.states[i] << ","
                         << res.transitions[i] << ","
                         << res.finals[i] << ","
                         << res.avg_degrees[i] << "\n";
            }
        }
        csv_file.close();
        std::cout << "SUCCESS: Full results saved -> benchmark_results.csv\n";
    }

    // --- Export LaTeX table ---
    std::stringstream latex_ss;
    latex_ss << "\\begin{table}[htbp]\n";
    latex_ss << "\\caption{Aggregated Topological Parameters (mean $\\pm$ std dev, $n = " << num_replications << "$ replications per scale)}\n";
    latex_ss << "\\label{tab:storage_footprint}\n";
    latex_ss << "\\centering\n";
    latex_ss << "\\small\n";
    latex_ss << "\\begin{tabular}{ccccc}\n";
    latex_ss << "\\toprule\n";
    latex_ss << "\\textbf{Scale} & \\textbf{States ($|Q|$)} & \\textbf{Transitions ($|\\delta|$)} & \\textbf{Final States ($|F|$)} & \\textbf{Avg Degree ($\\bar{d}$)} \\\\\n";
    latex_ss << "\\midrule\n";

    for (size_t scale : target_scales) {
        const auto& res = all_results[scale];
        std::vector<double> trans_double(res.transitions.begin(), res.transitions.end());
        std::vector<double> final_double(res.finals.begin(), res.finals.end());
        std::vector<double> degree_double(res.avg_degrees.begin(), res.avg_degrees.end());
        
        Stats trans_stats = computeStats(trans_double);
        Stats final_stats = computeStats(final_double);
        Stats degree_stats = computeStats(degree_double);
        
        size_t states = res.states.empty() ? 0 : res.states[0];
        
        latex_ss << std::left 
                 << std::setw(12) << ("NFA-" + std::to_string(scale) + " & ")
                 << std::setw(10) << (std::to_string(states) + " & ")
                 << std::setw(20) << (std::to_string(static_cast<int>(trans_stats.mean)) + " $\\pm$ " + 
                                      std::to_string(static_cast<int>(trans_stats.stddev)) + " & ")
                 << std::setw(20) << (std::to_string(static_cast<int>(final_stats.mean)) + " $\\pm$ " + 
                                      std::to_string(static_cast<int>(final_stats.stddev)) + " & ")
                 << std::fixed << std::setprecision(2) << degree_stats.mean << " $\\pm$ " 
                 << std::fixed << std::setprecision(2) << degree_stats.stddev << " \\\\\n";
    }

    latex_ss << "\\bottomrule\n";
    latex_ss << "\\end{tabular}\n";
    latex_ss << "\\end{table}\n";

    std::cout << "\n===================================================================\n";
    std::cout << "               GENERATED LATEX CODE FOR TABLE\n";
    std::cout << "===================================================================\n\n";
    std::cout << latex_ss.str() << "\n";

    std::ofstream tex_file("table_aggregated.tex");
    if (tex_file.is_open()) {
        tex_file << latex_ss.str();
        tex_file.close();
        std::cout << "SUCCESS: LaTeX table saved -> table_aggregated.tex\n";
    }

    // --- Optionally export a single representative Cypher file (for reproducibility) ---
    // (Re-use first replication of each scale)
    std::cout << "\nGenerating sample Cypher script for first replication...\n";
    std::vector<ComplexAutomaton> sample_automata;
    for (size_t scale : target_scales) {
        std::mt19937 rng(1000); // First seed
        AutomatonConfig config;
        config.min_states = scale;
        config.max_states = scale;
        config.alphabet = alphabet;
        config.transition_density = 0.25;
        config.allow_epsilon = false;
        config.allow_self_loops = true;
        config.self_loop_probability = 0.2;
        config.allow_bidirectional = false;
        
        ComplexAutomaton aut("NFA-" + std::to_string(scale));
        aut.generateRandom(config, rng);
        sample_automata.push_back(std::move(aut));
    }
    
    std::string cypher_script = CypherExporter::exportToCypher(sample_automata);
    std::ofstream cypher_file("sample_automata.cypher");
    if (cypher_file.is_open()) {
        cypher_file << cypher_script;
        cypher_file.close();
        std::cout << "SUCCESS: Sample Cypher script -> sample_automata.cypher\n";
    }

    std::cout << "\n===================================================================\n";
    std::cout << "                        COMPLETED\n";
    std::cout << "===================================================================\n";
    std::cout << "Files created:\n";
    std::cout << "  - benchmark_results.csv   (all raw data)\n";
    std::cout << "  - table_aggregated.tex    (LaTeX table)\n";
    std::cout << "  - sample_automata.cypher  (Cypher script for first replication)\n";

    return 0;
}
