// ===================================================================
// AUTOMATA DATASET INITIALIZATION SCRIPT (3 TOPOLOGIES)
// Engine: Neo4j (Cypher Query Language)
// ===================================================================

// -------------------------------------------------------------------
// STEP 1: CLEANUP
// Delete all existing nodes and transitions related to our benchmark
// -------------------------------------------------------------------
MATCH (n)
WHERE n.automaton_id IS NOT NULL
DETACH DELETE n;


// -------------------------------------------------------------------
// STEP 2: AUTOMATON 1 - Main Complex Topology ('aut_main')
// Single continuous CREATE statement to ensure full graph connectivity
// -------------------------------------------------------------------
CREATE 
  (s_main:START_NODE {name: 'start', automaton_id: 'aut_main'}),
  (q0:START_FINAL_STATE {name: 'q0', automaton_id: 'aut_main'}),
  (q1:STATE {name: 'q1', automaton_id: 'aut_main'}),
  (q2:FINAL_STATE {name: 'q2', automaton_id: 'aut_main'}),
  (q_trap:STATE {name: 'q_trap', automaton_id: 'aut_main'}),
  (u_iso1:STATE {name: 'u_iso1', automaton_id: 'aut_main'}),
  (u_iso2:STATE {name: 'u_iso2', automaton_id: 'aut_main'}),

  (s_main)-[:TRANSITION {symbols: 'Start', automaton_id: 'aut_main'}]->(q0),
  (q0)-[:TRANSITION {symbols: 'a', automaton_id: 'aut_main'}]->(q1),
  (q1)-[:TRANSITION {symbols: 'a', automaton_id: 'aut_main'}]->(q0),
  (q1)-[:TRANSITION {symbols: 'b', automaton_id: 'aut_main'}]->(q2),
  (q2)-[:TRANSITION {symbols: 'b', automaton_id: 'aut_main'}]->(q2),
  (q0)-[:TRANSITION {symbols: 'err', automaton_id: 'aut_main'}]->(q_trap),
  (q_trap)-[:TRANSITION {symbols: 'a,b,err', automaton_id: 'aut_main'}]->(q_trap),
  (u_iso1)-[:TRANSITION {symbols: 'x', automaton_id: 'aut_main'}]->(u_iso2),
  (u_iso2)-[:TRANSITION {symbols: 'y', automaton_id: 'aut_main'}]->(u_iso1);


// -------------------------------------------------------------------
// STEP 3: AUTOMATON 2 - NFA with Epsilon Transitions ('aut_nfa_eps')
// Single continuous CREATE statement for exact Epsilon-Closure structure
// -------------------------------------------------------------------
CREATE 
  (s_eps:START_NODE {name: 'start', automaton_id: 'aut_nfa_eps'}),
  (n0:START_STATE {name: 'n0', automaton_id: 'aut_nfa_eps'}),
  (n1:STATE {name: 'n1', automaton_id: 'aut_nfa_eps'}),
  (n2:STATE {name: 'n2', automaton_id: 'aut_nfa_eps'}),
  (n3:FINAL_STATE {name: 'n3', automaton_id: 'aut_nfa_eps'}),

  (s_eps)-[:TRANSITION {symbols: 'Start', automaton_id: 'aut_nfa_eps'}]->(n0),
  (n0)-[:TRANSITION {symbols: 'eps', automaton_id: 'aut_nfa_eps'}]->(n1),
  (n0)-[:TRANSITION {symbols: 'eps', automaton_id: 'aut_nfa_eps'}]->(n2),
  (n1)-[:TRANSITION {symbols: 'a', automaton_id: 'aut_nfa_eps'}]->(n3),
  (n2)-[:TRANSITION {symbols: 'b', automaton_id: 'aut_nfa_eps'}]->(n3);


// -------------------------------------------------------------------
// STEP 4: AUTOMATON 3 - Empty Language Boundary Case ('aut_empty_lang')
// Disconnected final state to test L(M) = Ø
// -------------------------------------------------------------------
CREATE 
  (s_emp:START_NODE {name: 'start_emp', automaton_id: 'aut_empty_lang'}),
  (e0:START_STATE {name: 'e0', automaton_id: 'aut_empty_lang'}),
  (e1:STATE {name: 'e1', automaton_id: 'aut_empty_lang'}),
  (e_fin:FINAL_STATE {name: 'e_fin', automaton_id: 'aut_empty_lang'}),

  (s_emp)-[:TRANSITION {symbols: 'Start', automaton_id: 'aut_empty_lang'}]->(e0),
  (e0)-[:TRANSITION {symbols: 'a', automaton_id: 'aut_empty_lang'}]->(e1),
  (e1)-[:TRANSITION {symbols: 'b', automaton_id: 'aut_empty_lang'}]->(e0);