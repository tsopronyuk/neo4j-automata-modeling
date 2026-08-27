// ===================================================================
// DEMONSTRATION QUERY SUITE FOR LPG AUTOMATA (7 ANALYTICAL DEMONSTRATIONS)
// Database Engine: Neo4j (Cypher Query Language)
// Supports both regular and combined start/final node labels:
//   - Start states:  :START_STATE | :START_FINAL_STATE
//   - Final states:  :FINAL_STATE | :START_FINAL_STATE
// 
// Mapping: 
//   - Demonstrations 1-5: Primary Automaton ('aut_main')
//   - Demonstration 6:   NFA Epsilon Automaton ('aut_nfa_eps')
//   - Demonstration 7:   Empty Language Automaton ('aut_empty_lang')
// ===================================================================

// ===================================================================
// PART 1: DEMONSTRATIONS FOR PRIMARY AUTOMATON TOPOLOGY ('aut_main')
// ===================================================================

// -------------------------------------------------------------------
// DEMONSTRATION 1: Valid Path Traversal
// Target: 'aut_main'
// Objective: Retrieve all accepting paths from initial to final state.
// -------------------------------------------------------------------
MATCH (start:START_STATE|START_FINAL_STATE {automaton_id: 'aut_main'}),
      (final:FINAL_STATE|START_FINAL_STATE {automaton_id: 'aut_main'}),
      p = (start)-[:TRANSITION*0..15]->(final)
RETURN [node IN nodes(p) | node.name] AS Path,
       [rel IN relationships(p) | rel.symbols] AS TransitionSymbols;

// -------------------------------------------------------------------
// DEMONSTRATION 2: Unreachable States Detection
// Target: 'aut_main'
// Objective: Identify isolated nodes that cannot be reached from any start state.
// -------------------------------------------------------------------
MATCH (all_states {automaton_id: 'aut_main'})
WHERE NOT all_states:START_NODE
  AND NOT EXISTS {
    MATCH (:START_STATE|START_FINAL_STATE {automaton_id: 'aut_main'})-[:TRANSITION*0..15]->(all_states)
  }
RETURN all_states.name AS UnreachableState;


// -------------------------------------------------------------------
// DEMONSTRATION 3: Trap / Sink State Identification
// Target: 'aut_main'
// Objective: Locate states from which no path exists to ANY final state.
// -------------------------------------------------------------------
MATCH (trap {automaton_id: 'aut_main'})
WHERE NOT trap:START_NODE
  AND NOT EXISTS {
    MATCH (trap)-[:TRANSITION*0..15]->(:FINAL_STATE|START_FINAL_STATE {automaton_id: 'aut_main'})
  }
RETURN trap.name AS TrapState;


// -------------------------------------------------------------------
// DEMONSTRATION 4: Immediate Empty-String Acceptance
// Target: 'aut_main'
// Objective: Verify if the automaton accepts epsilon immediately at start.
// -------------------------------------------------------------------
MATCH (s:START_FINAL_STATE {automaton_id: 'aut_main'})
RETURN s.name AS StartAndFinalState, 
       true AS AcceptsEmptyString;


// -------------------------------------------------------------------
// DEMONSTRATION 5: Cycle and Strongly Connected Component (SCC) Detection
// Target: 'aut_main'
// Objective: Detect loops and cyclic execution paths within the graph.
// -------------------------------------------------------------------
MATCH p = (n {automaton_id: 'aut_main'})-[:TRANSITION*1..15]->(n)
WHERE NOT n:START_NODE
RETURN DISTINCT n.name AS CycleNode, 
                [node IN nodes(p) | node.name] AS CyclePath;


// ===================================================================
// PART 2: DEMONSTRATIONS FOR SPECIALIZED TOPOLOGIES
// ===================================================================

// -------------------------------------------------------------------
// DEMONSTRATION 6: Epsilon-Closure Computation
// Target: 'aut_nfa_eps' (NFA with Epsilon Transitions)
// Objective: Compute all states reachable via epsilon transitions.
// -------------------------------------------------------------------
MATCH (start:START_STATE|START_FINAL_STATE {name: 'n0', automaton_id: 'aut_nfa_eps'})
MATCH p = (start)-[rel:TRANSITION*0..10]->(target)
WHERE ALL(r IN rel WHERE 'eps' IN split(r.symbols, ','))
RETURN start.name AS InitialState, 
       collect(DISTINCT target.name) AS EpsilonClosure;


// -------------------------------------------------------------------
// DEMONSTRATION 7: Empty Language Verification
// Target Topology: 'aut_empty_lang' (Disconnected Final State)
// Objective: Prove L(M) = Ø without generating Cartesian products
// -------------------------------------------------------------------
OPTIONAL MATCH p = (start:START_STATE|START_FINAL_STATE {automaton_id: 'aut_empty_lang'})-[:TRANSITION*0..15]->(final:FINAL_STATE|START_FINAL_STATE {automaton_id: 'aut_empty_lang'})
RETURN count(p) = 0 AS IsLanguageEmpty;
