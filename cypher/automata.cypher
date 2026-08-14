// ===================================================================
// GENERATED CYPHER SCRIPT FOR AUTOMATA INGESTION IN NEO4J
// Number of automata: 5
// ===================================================================

// Clear database
MATCH (n) DETACH DELETE n;

// --- Automaton: auto_01 ---
CREATE (auto_01_start_indicator:START_NODE {name: 'start', automaton_id: 'auto_01'});
CREATE (auto_01_q0:START_STATE {name: 'q0', automaton_id: 'auto_01'});
CREATE (auto_01_q1:STATE {name: 'q1', automaton_id: 'auto_01'});
CREATE (auto_01_q2:FINAL_STATE {name: 'q2', automaton_id: 'auto_01'});
CREATE (auto_01_q3:STATE {name: 'q3', automaton_id: 'auto_01'});
CREATE (auto_01_q4:STATE {name: 'q4', automaton_id: 'auto_01'});
MATCH (from:START_NODE {automaton_id: 'auto_01'}), (to {name: 'q0', automaton_id: 'auto_01'})
CREATE (from)-[:TRANSITION {symbols: 'Start', automaton_id: 'auto_01'}]->(to);
MATCH (from {name: 'q0', automaton_id: 'auto_01'}), (to {name: 'q1', automaton_id: 'auto_01'})
CREATE (from)-[:TRANSITION {symbols: 'a', automaton_id: 'auto_01'}]->(to);
MATCH (from {name: 'q1', automaton_id: 'auto_01'}), (to {name: 'q2', automaton_id: 'auto_01'})
CREATE (from)-[:TRANSITION {symbols: 'a', automaton_id: 'auto_01'}]->(to);
MATCH (from {name: 'q2', automaton_id: 'auto_01'}), (to {name: 'q3', automaton_id: 'auto_01'})
CREATE (from)-[:TRANSITION {symbols: 'w', automaton_id: 'auto_01'}]->(to);
MATCH (from {name: 'q3', automaton_id: 'auto_01'}), (to {name: 'q0', automaton_id: 'auto_01'})
CREATE (from)-[:TRANSITION {symbols: 'a', automaton_id: 'auto_01'}]->(to);
MATCH (from {name: 'q3', automaton_id: 'auto_01'}), (to {name: 'q4', automaton_id: 'auto_01'})
CREATE (from)-[:TRANSITION {symbols: 'x', automaton_id: 'auto_01'}]->(to);
// Automaton auto_01 created with 5 states
// Start state: q0
// Final states: q2

// --- Automaton: auto_02 ---
CREATE (auto_02_start_indicator:START_NODE {name: 'start', automaton_id: 'auto_02'});
CREATE (auto_02_q0:START_STATE {name: 'q0', automaton_id: 'auto_02'});
CREATE (auto_02_q1:STATE {name: 'q1', automaton_id: 'auto_02'});
CREATE (auto_02_q2:FINAL_STATE {name: 'q2', automaton_id: 'auto_02'});
CREATE (auto_02_q3:STATE {name: 'q3', automaton_id: 'auto_02'});
CREATE (auto_02_q4:STATE {name: 'q4', automaton_id: 'auto_02'});
MATCH (from:START_NODE {automaton_id: 'auto_02'}), (to {name: 'q0', automaton_id: 'auto_02'})
CREATE (from)-[:TRANSITION {symbols: 'Start', automaton_id: 'auto_02'}]->(to);
MATCH (from {name: 'q0', automaton_id: 'auto_02'}), (to {name: 'q1', automaton_id: 'auto_02'})
CREATE (from)-[:TRANSITION {symbols: 't', automaton_id: 'auto_02'}]->(to);
MATCH (from {name: 'q1', automaton_id: 'auto_02'}), (to {name: 'q2', automaton_id: 'auto_02'})
CREATE (from)-[:TRANSITION {symbols: 'w', automaton_id: 'auto_02'}]->(to);
MATCH (from {name: 'q1', automaton_id: 'auto_02'}), (to {name: 'q4', automaton_id: 'auto_02'})
CREATE (from)-[:TRANSITION {symbols: 't,w', automaton_id: 'auto_02'}]->(to);
MATCH (from {name: 'q2', automaton_id: 'auto_02'}), (to {name: 'q3', automaton_id: 'auto_02'})
CREATE (from)-[:TRANSITION {symbols: 'w', automaton_id: 'auto_02'}]->(to);
MATCH (from {name: 'q3', automaton_id: 'auto_02'}), (to {name: 'q4', automaton_id: 'auto_02'})
CREATE (from)-[:TRANSITION {symbols: 'w', automaton_id: 'auto_02'}]->(to);
// Automaton auto_02 created with 5 states
// Start state: q0
// Final states: q2

// --- Automaton: auto_03 ---
CREATE (auto_03_start_indicator:START_NODE {name: 'start', automaton_id: 'auto_03'});
CREATE (auto_03_q0:START_STATE {name: 'q0', automaton_id: 'auto_03'});
CREATE (auto_03_q1:STATE {name: 'q1', automaton_id: 'auto_03'});
CREATE (auto_03_q2:STATE {name: 'q2', automaton_id: 'auto_03'});
CREATE (auto_03_q3:FINAL_STATE {name: 'q3', automaton_id: 'auto_03'});
MATCH (from:START_NODE {automaton_id: 'auto_03'}), (to {name: 'q0', automaton_id: 'auto_03'})
CREATE (from)-[:TRANSITION {symbols: 'Start', automaton_id: 'auto_03'}]->(to);
MATCH (from {name: 'q0', automaton_id: 'auto_03'}), (to {name: 'q0', automaton_id: 'auto_03'})
CREATE (from)-[:TRANSITION {symbols: 'k', automaton_id: 'auto_03'}]->(to);
MATCH (from {name: 'q0', automaton_id: 'auto_03'}), (to {name: 'q1', automaton_id: 'auto_03'})
CREATE (from)-[:TRANSITION {symbols: 'y,ε', automaton_id: 'auto_03'}]->(to);
MATCH (from {name: 'q1', automaton_id: 'auto_03'}), (to {name: 'q2', automaton_id: 'auto_03'})
CREATE (from)-[:TRANSITION {symbols: 'y', automaton_id: 'auto_03'}]->(to);
MATCH (from {name: 'q1', automaton_id: 'auto_03'}), (to {name: 'q3', automaton_id: 'auto_03'})
CREATE (from)-[:TRANSITION {symbols: 'ε', automaton_id: 'auto_03'}]->(to);
MATCH (from {name: 'q2', automaton_id: 'auto_03'}), (to {name: 'q3', automaton_id: 'auto_03'})
CREATE (from)-[:TRANSITION {symbols: 'y', automaton_id: 'auto_03'}]->(to);
// Automaton auto_03 created with 4 states
// Start state: q0
// Final states: q3

// --- Automaton: auto_04 ---
CREATE (auto_04_start_indicator:START_NODE {name: 'start', automaton_id: 'auto_04'});
CREATE (auto_04_q0:START_STATE {name: 'q0', automaton_id: 'auto_04'});
CREATE (auto_04_q1:STATE {name: 'q1', automaton_id: 'auto_04'});
CREATE (auto_04_q2:STATE {name: 'q2', automaton_id: 'auto_04'});
CREATE (auto_04_q3:FINAL_STATE {name: 'q3', automaton_id: 'auto_04'});
CREATE (auto_04_q4:STATE {name: 'q4', automaton_id: 'auto_04'});
MATCH (from:START_NODE {automaton_id: 'auto_04'}), (to {name: 'q0', automaton_id: 'auto_04'})
CREATE (from)-[:TRANSITION {symbols: 'Start', automaton_id: 'auto_04'}]->(to);
MATCH (from {name: 'q0', automaton_id: 'auto_04'}), (to {name: 'q1', automaton_id: 'auto_04'})
CREATE (from)-[:TRANSITION {symbols: 't', automaton_id: 'auto_04'}]->(to);
MATCH (from {name: 'q1', automaton_id: 'auto_04'}), (to {name: 'q2', automaton_id: 'auto_04'})
CREATE (from)-[:TRANSITION {symbols: 't,ε', automaton_id: 'auto_04'}]->(to);
MATCH (from {name: 'q2', automaton_id: 'auto_04'}), (to {name: 'q3', automaton_id: 'auto_04'})
CREATE (from)-[:TRANSITION {symbols: 'n,t,ε', automaton_id: 'auto_04'}]->(to);
MATCH (from {name: 'q3', automaton_id: 'auto_04'}), (to {name: 'q4', automaton_id: 'auto_04'})
CREATE (from)-[:TRANSITION {symbols: 'n', automaton_id: 'auto_04'}]->(to);
MATCH (from {name: 'q4', automaton_id: 'auto_04'}), (to {name: 'q4', automaton_id: 'auto_04'})
CREATE (from)-[:TRANSITION {symbols: 't', automaton_id: 'auto_04'}]->(to);
// Automaton auto_04 created with 5 states
// Start state: q0
// Final states: q3

// --- Automaton: auto_05 ---
CREATE (auto_05_start_indicator:START_NODE {name: 'start', automaton_id: 'auto_05'});
CREATE (auto_05_q0:START_STATE {name: 'q0', automaton_id: 'auto_05'});
CREATE (auto_05_q1:FINAL_STATE {name: 'q1', automaton_id: 'auto_05'});
CREATE (auto_05_q2:STATE {name: 'q2', automaton_id: 'auto_05'});
CREATE (auto_05_q3:STATE {name: 'q3', automaton_id: 'auto_05'});
CREATE (auto_05_q4:STATE {name: 'q4', automaton_id: 'auto_05'});
CREATE (auto_05_q5:STATE {name: 'q5', automaton_id: 'auto_05'});
MATCH (from:START_NODE {automaton_id: 'auto_05'}), (to {name: 'q0', automaton_id: 'auto_05'})
CREATE (from)-[:TRANSITION {symbols: 'Start', automaton_id: 'auto_05'}]->(to);
MATCH (from {name: 'q0', automaton_id: 'auto_05'}), (to {name: 'q1', automaton_id: 'auto_05'})
CREATE (from)-[:TRANSITION {symbols: 'u', automaton_id: 'auto_05'}]->(to);
MATCH (from {name: 'q1', automaton_id: 'auto_05'}), (to {name: 'q2', automaton_id: 'auto_05'})
CREATE (from)-[:TRANSITION {symbols: 'm', automaton_id: 'auto_05'}]->(to);
MATCH (from {name: 'q2', automaton_id: 'auto_05'}), (to {name: 'q3', automaton_id: 'auto_05'})
CREATE (from)-[:TRANSITION {symbols: 'u', automaton_id: 'auto_05'}]->(to);
MATCH (from {name: 'q2', automaton_id: 'auto_05'}), (to {name: 'q5', automaton_id: 'auto_05'})
CREATE (from)-[:TRANSITION {symbols: 'm', automaton_id: 'auto_05'}]->(to);
MATCH (from {name: 'q3', automaton_id: 'auto_05'}), (to {name: 'q1', automaton_id: 'auto_05'})
CREATE (from)-[:TRANSITION {symbols: 'y', automaton_id: 'auto_05'}]->(to);
MATCH (from {name: 'q3', automaton_id: 'auto_05'}), (to {name: 'q2', automaton_id: 'auto_05'})
CREATE (from)-[:TRANSITION {symbols: 'ε', automaton_id: 'auto_05'}]->(to);
MATCH (from {name: 'q3', automaton_id: 'auto_05'}), (to {name: 'q4', automaton_id: 'auto_05'})
CREATE (from)-[:TRANSITION {symbols: 'y', automaton_id: 'auto_05'}]->(to);
MATCH (from {name: 'q4', automaton_id: 'auto_05'}), (to {name: 'q5', automaton_id: 'auto_05'})
CREATE (from)-[:TRANSITION {symbols: 'u', automaton_id: 'auto_05'}]->(to);
MATCH (from {name: 'q5', automaton_id: 'auto_05'}), (to {name: 'q2', automaton_id: 'auto_05'})
CREATE (from)-[:TRANSITION {symbols: 'u', automaton_id: 'auto_05'}]->(to);
// Automaton auto_05 created with 6 states
// Start state: q0
// Final states: q1

// ===================================================================
// QUERIES FOR VISUALIZATION
// ===================================================================
// View all automata:
// MATCH (n) OPTIONAL MATCH (n)-[r]->(m) RETURN n, r, m;

// View specific automaton:
// MATCH (n {automaton_id: 'auto_01'}) OPTIONAL MATCH (n)-[r:TRANSITION]->(m) RETURN n, r, m;
