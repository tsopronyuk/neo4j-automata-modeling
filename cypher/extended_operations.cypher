// ===================================================================
// EXTENDED AUTOMATA OPERATIONS & ALGEBRA SUITE (SUBSECTION 6.3)
// Database Engine: Neo4j (Cypher Query Language)
// Companion File: extended_automata_suite.cypher
// -------------------------------------------------------------------
// FULL AUTOMATA INVENTORY IN THIS SUITE:
// 1. 'aut_word'    - NFA topology specifically used for word path tracing (e.g., w = ['a', 'b', 'b']).
// 2. 'aut_main'    - Primary NFA topology (M1) used for general queries & product synthesis.
// 3. 'aut_04'      - Synthetic 7-state NFA benchmark topology for shortest word search.
// 4. 'aut_nfa_eps' - NFA topology containing epsilon transitions (M2).
// 5. 'aut_prod'    - Synthesized Cartesian Product Automaton (aut_main x aut_nfa_eps).
// ===================================================================

// ===================================================================
// PART 1: DYNAMIC WORD ACCEPTANCE VALIDATION (SUBSECTION 6.3.1)
// Target Word: w = ['a', 'b', 'b']
// Target Automaton: 'aut_word' (Dedicated Word Traversal & Acceptance Topology)
// ===================================================================

// -------------------------------------------------------------------
// QUERY 1A: Visual Path Extraction for Non-Repeating Sequences
// Target Automaton: 'aut_word'
// Input Word: w = ['a', 'b', 'b']
// Objective: Extract and return the complete visual subgraph path 
//            (including the entry START_NODE) for rendering in Neo4j Browser.
// -------------------------------------------------------------------
WITH ['a', 'b', 'b'] AS inputWord
MATCH (entry:START_NODE {automaton_id: 'aut_word'})-[r_start:TRANSITION]->(start:START_FINAL_STATE {automaton_id: 'aut_word'})
MATCH p = (start)-[:TRANSITION*]->(target:FINAL_STATE|START_FINAL_STATE {automaton_id: 'aut_word'})
WHERE length(p) = size(inputWord)
  AND ALL(i IN range(0, size(inputWord)-1) 
      WHERE inputWord[i] IN split((relationships(p)[i]).symbols, ','))
RETURN entry, r_start, p;


// -------------------------------------------------------------------
// QUERY 1B: Generalized Dynamic Algorithm for Arbitrary Cyclic Words
// Target Automaton: 'aut_word'
// Input Word: w = ['a', 'a', 'a', 'b', 'b', 'b', 'b']
// Objective: Evaluate language acceptance dynamically over state sets
//            without hardcoded state IDs or single-path trail limits.
// -------------------------------------------------------------------
WITH ['a', 'a', 'a', 'b', 'b', 'b', 'b'] AS inputWord
MATCH (start:START_STATE|START_FINAL_STATE {automaton_id: 'aut_word'})
WITH inputWord, [start] AS activeStates
UNWIND inputWord AS sym
MATCH (s {automaton_id: 'aut_word'})-[r:TRANSITION {automaton_id: 'aut_word'}]->(target {automaton_id: 'aut_word'})
WHERE s IN activeStates AND sym IN split(r.symbols, ',')
WITH collect(DISTINCT target) AS activeStates
WHERE size(activeStates) > 0
WITH activeStates
RETURN ANY(node IN activeStates WHERE node:FINAL_STATE OR node:START_FINAL_STATE) AS IsAccepted;


// ===================================================================
// PART 2: GLOBAL SHORTEST ACCEPTED WORD EXTRACTION (SUBSECTION 6.3.2)
// Target Automaton: 'aut_04' (7-state synthetic NFA placement figure)
// ===================================================================

// -------------------------------------------------------------------
// QUERY 2: Global Shortest Accepted Word Search Across All Final States
// Target Automaton: 'aut_04'
// Objective: Extract ONLY global shortest paths in L(aut_04) \ {epsilon}
//            including the initial indicator node (START_NODE).
// -------------------------------------------------------------------
MATCH (entry:START_NODE {automaton_id: 'aut_04'})-[r_start:TRANSITION]->(s:START_STATE|START_FINAL_STATE {automaton_id: 'aut_04'}),
      (f:FINAL_STATE|START_FINAL_STATE {automaton_id: 'aut_04'})
MATCH p = (s)-[:TRANSITION*0..10]->(f)
WHERE length(p) >= 1
WITH length(p) AS len, collect(p) AS paths, entry, r_start
ORDER BY len ASC
LIMIT 1
UNWIND paths AS p
RETURN entry, r_start, p;


// ===================================================================
// PART 3: SYNCHRONOUS PRODUCT AUTOMATON SYNTHESIS M1 x M2 (SUBSECTION 6.3.3)
// Source Automata: 'aut_main' (M1), 'aut_nfa_eps' (M2)
// Target Result Automaton: 'aut_prod' (Synthesized Product Automaton)
// Objective: Construct synchronized Cartesian product topology M1 x M2,
//            inheriting exact role labels (START, FINAL, START_FINAL)
//            to guarantee proper visual legend styling in Neo4j Browser.
// ===================================================================
// ===================================================================
// EXTENDED AUTOMATA OPERATIONS & ALGEBRA SUITE (SUBSECTION 6.3)
// Database Engine: Neo4j (Cypher Query Language)
// Companion File: extended_automata_suite.cypher
// -------------------------------------------------------------------
// FULL AUTOMATA INVENTORY IN THIS SUITE:
// 1. 'aut_word'    - NFA topology specifically used for word path tracing (e.g., w = ['a', 'b', 'b']).
// 2. 'aut_main'    - Primary NFA topology (M1) used for general queries & product synthesis.
// 3. 'aut_04'      - Synthetic 7-state NFA benchmark topology for shortest word search.
// 4. 'aut_nfa_eps' - NFA topology containing epsilon transitions (M2).
// 5. 'aut_prod'    - Synthesized Cartesian Product Automaton (aut_main x aut_nfa_eps).
// ===================================================================

// ===================================================================
// PART 1: DYNAMIC WORD ACCEPTANCE VALIDATION (SUBSECTION 6.3.1)
// Target Word: w = ['a', 'b', 'b']
// Target Automaton: 'aut_word' (Dedicated Word Traversal & Acceptance Topology)
// ===================================================================

// -------------------------------------------------------------------
// QUERY 1A: Visual Path Extraction for Non-Repeating Sequences
// Target Automaton: 'aut_word'
// Input Word: w = ['a', 'b', 'b']
// Objective: Extract and return the complete visual subgraph path 
//            (including the entry START_NODE) for rendering in Neo4j Browser.
// -------------------------------------------------------------------
WITH ['a', 'b', 'b'] AS inputWord
MATCH (entry:START_NODE {automaton_id: 'aut_word'})-[r_start:TRANSITION]->(start:START_FINAL_STATE {automaton_id: 'aut_word'})
MATCH p = (start)-[:TRANSITION*]->(target:FINAL_STATE|START_FINAL_STATE {automaton_id: 'aut_word'})
WHERE length(p) = size(inputWord)
  AND ALL(i IN range(0, size(inputWord)-1) 
      WHERE inputWord[i] IN split((relationships(p)[i]).symbols, ','))
RETURN entry, r_start, p;


// -------------------------------------------------------------------
// QUERY 1B: Generalized Dynamic Algorithm for Arbitrary Cyclic Words
// Target Automaton: 'aut_word'
// Input Word: w = ['a', 'a', 'a', 'b', 'b', 'b', 'b']
// Objective: Evaluate language acceptance dynamically over state sets
//            without hardcoded state IDs or single-path trail limits.
// -------------------------------------------------------------------
WITH ['a', 'a', 'a', 'b', 'b', 'b', 'b'] AS inputWord
MATCH (start:START_STATE|START_FINAL_STATE {automaton_id: 'aut_word'})
WITH inputWord, [start] AS activeStates
UNWIND inputWord AS sym
MATCH (s {automaton_id: 'aut_word'})-[r:TRANSITION {automaton_id: 'aut_word'}]->(target {automaton_id: 'aut_word'})
WHERE s IN activeStates AND sym IN split(r.symbols, ',')
WITH collect(DISTINCT target) AS activeStates
WHERE size(activeStates) > 0
WITH activeStates
RETURN ANY(node IN activeStates WHERE node:FINAL_STATE OR node:START_FINAL_STATE) AS IsAccepted;


// ===================================================================
// PART 2: GLOBAL SHORTEST ACCEPTED WORD EXTRACTION (SUBSECTION 6.3.2)
// Target Automaton: 'aut_04' (7-state synthetic NFA placement figure)
// ===================================================================

// -------------------------------------------------------------------
// QUERY 2: Global Shortest Accepted Word Search Across All Final States
// Target Automaton: 'aut_04'
// Objective: Extract ONLY global shortest paths in L(aut_04) \ {epsilon}
//            including the initial indicator node (START_NODE).
// -------------------------------------------------------------------
MATCH (entry:START_NODE {automaton_id: 'aut_04'})-[r_start:TRANSITION]->(s:START_STATE|START_FINAL_STATE {automaton_id: 'aut_04'}),
      (f:FINAL_STATE|START_FINAL_STATE {automaton_id: 'aut_04'})
MATCH p = (s)-[:TRANSITION*1..10]->(f)
WHERE length(p) >= 1
WITH length(p) AS len, collect(p) AS paths, entry, r_start
ORDER BY len ASC
LIMIT 1
UNWIND paths AS p
RETURN entry, r_start, p;


// ===================================================================
// PART 3: SYNCHRONOUS PRODUCT AUTOMATON SYNTHESIS M1 x M2 (SUBSECTION 6.3.3)
// Source Automata: 'aut_main' (M1), 'aut_nfa_eps' (M2)
// Target Result Automaton: 'aut_prod' (Synthesized Product Automaton)
// Objective: Construct synchronized Cartesian product topology M1 x M2,
//            inheriting exact role labels (START, FINAL, START_FINAL)
//            to guarantee proper visual legend styling in Neo4j Browser.
// ===================================================================

// -------------------------------------------------------------------
// STEP 3.1: Cleanup Previous Product Topology ('aut_prod')
// -------------------------------------------------------------------
MATCH (n {automaton_id: 'aut_prod'}) DETACH DELETE n;


// -------------------------------------------------------------------
// STEP 3.2: Synthesize Product State Nodes & Inherit Role Labels
// Source Automata: 'aut_main', 'aut_nfa_eps'
// Target Result Automaton: 'aut_prod'
// -------------------------------------------------------------------
MATCH (n1 {automaton_id: 'aut_main'}), (n2 {automaton_id: 'aut_nfa_eps'})
WHERE NOT n1:START_NODE AND NOT n2:START_NODE

// Robustly detect start states (via labels OR incoming START_NODE edges)
OPTIONAL MATCH (:START_NODE {automaton_id: 'aut_main'})-[r1_start]->(n1)
OPTIONAL MATCH (:START_NODE {automaton_id: 'aut_nfa_eps'})-[r2_start]->(n2)

WITH n1, n2,
     (n1:START_STATE OR n1:START_FINAL_STATE OR r1_start IS NOT NULL) AS n1_is_start,
     (n2:START_STATE OR n2:START_FINAL_STATE OR r2_start IS NOT NULL) AS n2_is_start,
     (n1:FINAL_STATE OR n1:START_FINAL_STATE) AS n1_is_final,
     (n2:FINAL_STATE OR n2:START_FINAL_STATE) AS n2_is_final

WITH n1, n2,
     (n1_is_start AND n2_is_start) AS is_start,
     (n1_is_final AND n2_is_final) AS is_final

CREATE (p {
  name: n1.name + '_' + n2.name,
  automaton_id: 'aut_prod'
})

// Assign exactly one primary label to enforce Neo4j Browser legend colors
FOREACH (_ IN CASE WHEN is_start AND is_final THEN [1] ELSE [] END | SET p:START_FINAL_STATE)
FOREACH (_ IN CASE WHEN is_start AND NOT is_final THEN [1] ELSE [] END | SET p:START_STATE)
FOREACH (_ IN CASE WHEN NOT is_start AND is_final THEN [1] ELSE [] END | SET p:FINAL_STATE)
FOREACH (_ IN CASE WHEN NOT is_start AND NOT is_final THEN [1] ELSE [] END | SET p:STATE);


// -------------------------------------------------------------------
// STEP 3.3: Synthesize Start Indicator Node for 'aut_prod'
// Target Result Automaton: 'aut_prod'
// -------------------------------------------------------------------
CREATE (start_ind:START_NODE {name: 'start', automaton_id: 'aut_prod'});

MATCH (start_ind:START_NODE {automaton_id: 'aut_prod'}), 
      (start_st {automaton_id: 'aut_prod'})
WHERE start_st:START_STATE OR start_st:START_FINAL_STATE
CREATE (start_ind)-[:TRANSITION {symbols: 'Start', automaton_id: 'aut_prod'}]->(start_st);


// -------------------------------------------------------------------
// STEP 3.4: Synthesize Synchronized Transitions
// Source Automata: 'aut_main', 'aut_nfa_eps'
// Target Result Automaton: 'aut_prod'
// -------------------------------------------------------------------
MATCH (n1 {automaton_id: 'aut_main'})-[r1:TRANSITION]->(m1 {automaton_id: 'aut_main'}),
      (n2 {automaton_id: 'aut_nfa_eps'})-[r2:TRANSITION]->(m2 {automaton_id: 'aut_nfa_eps'})
WITH n1, m1, n2, m2,
     [sym IN split(r1.symbols, ',') WHERE sym IN split(r2.symbols, ',')] AS commonSymbols
WHERE size(commonSymbols) > 0
MATCH (p1 {name: n1.name + '_' + n2.name, automaton_id: 'aut_prod'}),
      (p2 {name: m1.name + '_' + m2.name, automaton_id: 'aut_prod'})
MERGE (p1)-[r:TRANSITION {automaton_id: 'aut_prod'}]->(p2)
SET r.symbols = reduce(s = "", x IN commonSymbols | CASE WHEN s = "" THEN x ELSE s + "," + x END);


// -------------------------------------------------------------------
// STEP 3.5: Verify Non-Emptiness of Language Intersection L(M1 ∩ M2)
// Target Result Automaton: 'aut_prod'
// Objective: Prove whether L(M1 ∩ M2) ≠ Ø by checking reachability 
//            from initial state to any final state in 'aut_prod'.
// -------------------------------------------------------------------
OPTIONAL MATCH path = (init:START_STATE|START_FINAL_STATE {automaton_id: 'aut_prod'})-[:TRANSITION*0..15]->(target:FINAL_STATE|START_FINAL_STATE {automaton_id: 'aut_prod'})
RETURN count(path) > 0 AS IsIntersectionNonEmpty;