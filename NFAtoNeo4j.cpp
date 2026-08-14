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
// 1. Configuration for automaton generation
// -------------------------------------------------------------------
struct AutomatonConfig {
    size_t min_states = 4;
    size_t max_states = 6;
    std::vector<std::string> alphabet;
    double transition_density = 0.25;
    bool allow_epsilon = true;
    bool allow_self_loops = true;
    double self_loop_probability = 0.3;  // Probability for each state to have a self-loop
    bool allow_bidirectional = true;
    double bidirectional_probability = 0.3;  // Probability for each pair to be bidirectional
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
        std::uniform_int_distribution<size_t> state_dist(config.min_states, config.max_states);
        size_t num_states = state_dist(rng);

        states_.clear();
        delta_.clear();

        // 1. Create states
        for (size_t i = 0; i < num_states; ++i) {
            std::string state_id = "q" + std::to_string(i);
            bool is_start = (i == 0);
            states_.emplace_back(state_id, is_start, false);
        }

        // Set final states (non-start) - count depends on total number of states
        if (num_states > 1) {
            size_t max_final = std::max(size_t(1), num_states / 3);
            max_final = std::min(max_final, num_states / 2);
            std::uniform_int_distribution<size_t> num_final_dist(1, max_final);
            size_t num_final = num_final_dist(rng);
            
            std::vector<size_t> non_start_indices;
            for (size_t i = 1; i < num_states; ++i) {
                non_start_indices.push_back(i);
            }
            
            std::shuffle(non_start_indices.begin(), non_start_indices.end(), rng);
            for (size_t i = 0; i < num_final && i < non_start_indices.size(); ++i) {
                states_[non_start_indices[i]].is_final = true;
            }
        }

        // 2. Build reachability backbone (q0 -> q1 -> ... -> qN-1)
        for (size_t i = 0; i < num_states - 1; ++i) {
            std::uniform_int_distribution<size_t> sym_dist(0, config.alphabet.size() - 1);
            std::string sym = config.alphabet[sym_dist(rng)];
            delta_[states_[i].id][sym].insert(states_[i + 1].id);
        }

        // 3. Add random transitions first (this creates the base structure)
        std::uniform_real_distribution<double> prob_dist(0.0, 1.0);
        std::uniform_int_distribution<size_t> target_dist(0, num_states - 1);

        auto active_alphabet = config.alphabet;
        if (config.allow_epsilon) {
            active_alphabet.push_back("ε");
        }

        // Add random transitions
        for (const auto& from_state : states_) {
            for (const auto& symbol : active_alphabet) {
                if (prob_dist(rng) < config.transition_density) {
                    const auto& to_state = states_[target_dist(rng)];
                    delta_[from_state.id][symbol].insert(to_state.id);
                }
            }
        }

        // 4. Add self-loops randomly
        if (config.allow_self_loops) {
            std::uniform_real_distribution<double> loop_prob(0.0, 1.0);
            std::uniform_int_distribution<size_t> sym_dist(0, config.alphabet.size() - 1);
            
            for (const auto& from_state : states_) {
                // Randomly decide if this state gets a self-loop
                if (loop_prob(rng) < config.self_loop_probability) {
                    std::string sym = config.alphabet[sym_dist(rng)];
                    delta_[from_state.id][sym].insert(from_state.id);
                }
            }
        }

        // 5. Add bidirectional transitions randomly
        if (config.allow_bidirectional && num_states >= 2) {
            std::uniform_int_distribution<size_t> sym_dist(0, config.alphabet.size() - 1);
            std::uniform_real_distribution<double> bidir_prob(0.0, 1.0);
            
            // For each pair of states, decide randomly if they become bidirectional
            for (size_t i = 0; i < num_states; ++i) {
                for (size_t j = i + 1; j < num_states; ++j) {
                    if (bidir_prob(rng) < config.bidirectional_probability) {
                        std::string from_id = "q" + std::to_string(i);
                        std::string to_id = "q" + std::to_string(j);
                        
                        // Choose random symbols for both directions
                        std::string sym1 = config.alphabet[sym_dist(rng)];
                        std::string sym2 = config.alphabet[sym_dist(rng)];
                        
                        // Add transitions in both directions
                        delta_[from_id][sym1].insert(to_id);
                        delta_[to_id][sym2].insert(from_id);
                    }
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
// 4. Alphabet Generator (2-3 symbols, excluding 'e')
// -------------------------------------------------------------------
std::vector<std::string> generateRandomAlphabet(std::mt19937& rng) {
    const std::string letters = "abcdfghijklmnopqrstuvwxyz";
    std::uniform_int_distribution<size_t> size_dist(2, 3);
    size_t alphabet_size = size_dist(rng);
    
    std::vector<char> available(letters.begin(), letters.end());
    std::shuffle(available.begin(), available.end(), rng);
    
    std::vector<std::string> result;
    for (size_t i = 0; i < alphabet_size; ++i) {
        result.push_back(std::string(1, available[i]));
    }
    
    return result;
}

// -------------------------------------------------------------------
// 5. Cypher Exporter
// -------------------------------------------------------------------
class CypherExporter {
public:
    static std::string exportToCypher(const std::vector<ComplexAutomaton>& automata) {
        std::stringstream ss;

        ss << "// ===================================================================\n";
        ss << "// GENERATED CYPHER SCRIPT FOR AUTOMATA INGESTION IN NEO4J\n";
        ss << "// Number of automata: " << automata.size() << "\n";
        ss << "// ===================================================================\n\n";
        ss << "// Clear database\n";
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
                    std::string var_name = aut.getId() + "_" + st.id;
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

            ss << "// Automaton " << aut.getId() << " created with "
               << aut.getStates().size() << " states\n";
            
            std::string start_state;
            std::vector<std::string> final_states;
            for (const auto& st : aut.getStates()) {
                if (st.is_start) start_state = st.id;
                if (st.is_final) final_states.push_back(st.id);
            }
            
            ss << "// Start state: " << start_state << "\n";
            ss << "// Final states: ";
            for (size_t i = 0; i < final_states.size(); ++i) {
                if (i > 0) ss << ", ";
                ss << final_states[i];
            }
            
            // Count bidirectional pairs
            size_t bidir_pairs = 0;
            auto grouped_copy = aut.getGroupedTransitions();
            for (const auto& [from_id, to_map] : grouped_copy) {
                for (const auto& [to_id, symbols] : to_map) {
                    if (from_id < to_id) {
                        auto it = grouped_copy.find(to_id);
                        if (it != grouped_copy.end()) {
                            if (it->second.find(from_id) != it->second.end()) {
                                bidir_pairs++;
                            }
                        }
                    }
                }
            }
            
            size_t total_trans = 0;
            size_t self_loops = 0;
            for (const auto& [from_id, to_map] : aut.getGroupedTransitions()) {
                for (const auto& [to_id, symbols] : to_map) {
                    total_trans++;
                    if (from_id == to_id) self_loops++;
                }
            }
            ss << "\n// Total transitions: " << total_trans;
            ss << " (including " << self_loops << " self-loops)";
            ss << "\n// Bidirectional pairs: " << bidir_pairs << "\n\n";
        }

        ss << "// ===================================================================\n";
        ss << "// QUERIES FOR VISUALIZATION\n";
        ss << "// ===================================================================\n";
        ss << "// View all automata:\n";
        ss << "// MATCH (n) OPTIONAL MATCH (n)-[r]->(m) RETURN n, r, m;\n\n";
        ss << "// View specific automaton:\n";
        ss << "// MATCH (n {automaton_id: 'auto_01'}) OPTIONAL MATCH (n)-[r:TRANSITION]->(m) RETURN n, r, m;\n";

        return ss.str();
    }
};

// -------------------------------------------------------------------
// 6. Constants and Main Function
// -------------------------------------------------------------------

const int NUM_AUTOMATA = 5;
const std::string OUTPUT_FILE = "automata.cypher";

int main() {
    std::cout << "=== NFA to Neo4j Cypher Generator ===\n";
    std::cout << "Number of automata: " << NUM_AUTOMATA << "\n";
    std::cout << "Output file: " << OUTPUT_FILE << "\n\n";

    std::random_device rd;
    std::mt19937 rng(rd());

    std::vector<ComplexAutomaton> automata;
    automata.reserve(NUM_AUTOMATA);

    for (int i = 0; i < NUM_AUTOMATA; ++i) {
        std::stringstream ss;
        ss << "aut_" << std::setw(2) << std::setfill('0') << (i + 1);
        std::string automaton_id = ss.str();

        auto alphabet = generateRandomAlphabet(rng);

        AutomatonConfig config;
        config.min_states = 4;
        config.max_states = 7;
        config.alphabet = alphabet;
        config.transition_density = 0.25;
        config.allow_epsilon = true;
        config.allow_self_loops = true;
        config.self_loop_probability = 0.2;  // 30% chance for each state to have a self-loop
        config.allow_bidirectional = true;
        config.bidirectional_probability = 0.2;  // 30% chance for each pair

        ComplexAutomaton automaton(automaton_id);
        automaton.generateRandom(config, rng);
        automata.push_back(std::move(automaton));

        std::cout << "Generated " << automaton_id << " with alphabet {";
        for (size_t j = 0; j < alphabet.size(); ++j) {
            if (j > 0) std::cout << ", ";
            std::cout << alphabet[j];
        }
        std::cout << "}\n";
    }

    std::string cypher_script = CypherExporter::exportToCypher(automata);

    std::ofstream file(OUTPUT_FILE);
    if (file.is_open()) {
        file << cypher_script;
        file.close();
        std::cout << "\n✅ Cypher script saved to: " << OUTPUT_FILE << "\n";
    } else {
        std::cerr << "❌ Error: Could not open file " << OUTPUT_FILE << " for writing.\n";
        return 1;
    }

    std::cout << "\n=== DONE ===\n";
    return 0;
}
