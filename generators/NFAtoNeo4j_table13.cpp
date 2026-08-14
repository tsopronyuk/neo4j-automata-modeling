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
    bool allow_bidirectional = false; // Disabled to prevent O(N^2) memory explosion
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

        // Step 3: Construct linear backbone to guarantee connectivity (q0 -> q1 -> ... -> qN-1)
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
// Helper: Format large numbers with thousands separators
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

struct BenchmarkMetric {
    std::string scale_label;
    size_t num_states;
    size_t num_transitions;
    size_t num_final;
    double avg_degree;
};

// -------------------------------------------------------------------
// 5. Main Function
// -------------------------------------------------------------------
int main() {
    std::cout << "===================================================================\n";
    std::cout << "   Automaton Benchmark Generator & Topological Profiler (C++17)   \n";
    std::cout << "===================================================================\n\n";

    // Set fixed seed for deterministic reproducibility
    std::mt19937 rng(42); 

    const std::vector<size_t> target_scales = {10, 100, 1000, 5000, 10000};
    std::vector<ComplexAutomaton> benchmark_automata;
    std::vector<BenchmarkMetric> metrics;

    // Output console header
    std::cout << std::left 
              << std::setw(18) << "Benchmark Scale"
              << std::setw(15) << "States (|Q|)"
              << std::setw(20) << "Transitions (|delta|)"
              << std::setw(20) << "Final States (|F|)"
              << std::setw(15) << "Avg Degree (d_bar)" << "\n";
    std::cout << std::string(88, '-') << "\n";

    for (size_t scale : target_scales) {
        std::string id = "DFA-" + std::to_string(scale);
        std::vector<std::string> alphabet = {"a", "b"};

        AutomatonConfig config;
        config.min_states = scale;
        config.max_states = scale;
        config.alphabet = alphabet;
        config.transition_density = 0.25;
        config.allow_epsilon = false;
        config.allow_self_loops = true;
        config.self_loop_probability = 0.2;
        config.allow_bidirectional = false; // Kept false to maintain sparse topology

        ComplexAutomaton aut(id);
        aut.generateRandom(config, rng);

        // Calculate topological metrics
        size_t num_states = aut.getStates().size();
        size_t final_count = 0;
        for (const auto& st : aut.getStates()) {
            if (st.is_final) final_count++;
        }

        size_t total_transitions = 0;
        for (const auto& [from_id, sym_map] : aut.getDelta()) {
            for (const auto& [sym, targets] : sym_map) {
                total_transitions += targets.size();
            }
        }

        double avg_degree = static_cast<double>(total_transitions) / num_states;

        // Print row to console
        std::cout << std::left 
                  << std::setw(18) << id
                  << std::setw(15) << num_states
                  << std::setw(20) << total_transitions
                  << std::setw(20) << final_count
                  << std::setw(15) << std::fixed << std::setprecision(2) << avg_degree << "\n";

        metrics.push_back({id, num_states, total_transitions, final_count, avg_degree});
        benchmark_automata.push_back(std::move(aut));
    }

    std::cout << std::string(88, '-') << "\n\n";

    // Export Cypher file
    std::string cypher_script = CypherExporter::exportToCypher(benchmark_automata);
    std::ofstream file("benchmark_automata.cypher");
    if (file.is_open()) {
        file << cypher_script;
        file.close();
        std::cout << "SUCCESS: Cypher script exported -> benchmark_automata.cypher\n";
    }

    // Export LaTeX code for Table 13
    std::stringstream latex_ss;
    latex_ss << "\\begin{table}[htbp]\n";
    latex_ss << "\\caption{Topological Parameters and Structural Metrics of Benchmark Automata}\n";
    latex_ss << "\\label{tab:storage_footprint}\n";
    latex_ss << "\\centering\n";
    latex_ss << "\\small\n";
    latex_ss << "\\begin{tabular}{ccccc}\n";
    latex_ss << "\\toprule\n";
    latex_ss << "\\textbf{Benchmark Scale} & \\textbf{States ($|Q|$)} & \\textbf{Transitions ($|\\delta|$)} & \\textbf{Final States ($|F|$)} & \\textbf{Avg Degree ($\\bar{d}$)} \\\\\n";
    latex_ss << "\\midrule\n";

    for (const auto& m : metrics) {
        latex_ss << std::left 
                 << std::setw(12) << (m.scale_label + " & ")
                 << std::setw(10) << (formatNumber(m.num_states) + " & ")
                 << std::setw(12) << (formatNumber(m.num_transitions) + " & ")
                 << std::setw(10) << (formatNumber(m.num_final) + " & ")
                 << std::fixed << std::setprecision(2) << m.avg_degree << " \\\\\n";
    }

    latex_ss << "\\bottomrule\n";
    latex_ss << "\\end{tabular}\n";
    latex_ss << "\\end{table}\n";

    std::cout << "\n===================================================================\n";
    std::cout << "               GENERATED LATEX CODE FOR TABLE 13                   \n";
    std::cout << "===================================================================\n\n";
    std::cout << latex_ss.str() << "\n";

    std::ofstream tex_file("table_13.tex");
    if (tex_file.is_open()) {
        tex_file << latex_ss.str();
        tex_file.close();
        std::cout << "SUCCESS: LaTeX snippet saved -> table_13.tex\n";
    }

    return 0;
}
