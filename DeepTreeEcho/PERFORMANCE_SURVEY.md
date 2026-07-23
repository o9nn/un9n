# DeepTreeEcho Performance Survey

Confirmed hot-path performance issues in the wider DeepTreeEcho tree, found during the
GameTraining optimization pass (adversarially verified, report-only — these subsystems are
outside the GameTraining branch's scope and should be fixed on their own branches).

Each finding survived two independent refuters challenging whether the path is actually hot
and whether the cost is real and quantifiable.

## 1. OnlineLearningSystem: O(|QTable|) full-map scan per RL transition — HIGH

`Learning/OnlineLearningSystem.cpp` — `GetMaxQValue` linearly scans the entire QTable with
FString prefix compares, and runs **twice per recorded transition**. With thousands of
state-action entries this is the dominant per-transition cost. `RecordExperience` also evicts
its buffer with `RemoveAt(0)` (~90KB memmove per transition once full).

**Fix direction:** restructure QTable as `TMap<State, TMap<Action, Entry>>` so max-Q scans
only one state's actions; ring-buffer the experience eviction; cache the `Find` result in
`UpdateQValue` instead of `Contains` + double `operator[]`.

## 2. DeepTreeEchoReservoir: connectivity regenerated per input — HIGH

`Reservoir/DeepTreeEchoReservoir.cpp` — `ProcessInput` regenerates random reservoir
connectivity on every call: ~11,000 RNG invocations (O(Units²)) per input, ~600k per
movement-embedding query. Beyond the cost, regenerating weights per call breaks the "echo
state" property the reservoir exists to provide.

**Fix direction:** generate and store the sparse recurrent weight matrix once in
`CreateReservoir` (CSR layout); `ProcessInput` becomes a fixed sparse mat-vec (~1,000
multiply-adds at 10% density) into a preallocated scratch buffer.

## 3. ReservoirCognitiveIntegration: unconditional Hebbian pass every tick — MEDIUM

`Reservoir/ReservoirCognitiveIntegration.cpp` — `ApplyHebbianLearning` runs 4 × ReservoirSize²
(~40,000) multiply-add-clamp iterations at 60 Hz whether or not any state changed.

**Fix direction:** gate behind a dirty flag set by `ProcessSensoryInput`, and/or decimate to
every Nth tick with the learning rate scaled by N. Cache the AttentionAllocation rebuild in
`UpdateCognitiveState` behind a SalienceMap-changed check.

## 4. DNABodySchemaBinding: per-joint TMap hashing at 30 Hz — MEDIUM

`4ECognition/DNABodySchemaBinding.cpp` — the proprioceptive sync performs 5 FString-hash TMap
operations per joint plus an unreserved 14-floats-per-joint array rebuild, for hundreds of
joints on a MetaHuman rig, 30 times per second.

**Fix direction:** store `PrevPosition`/`PrevOrientation` inside `FProprioceptiveState` and
iterate a flat index-based binding array (zero hashing); `Reserve(NumJoints * 14)` or reuse a
member buffer in `GetProprioceptiveVector`; make MovementHistory a ring of preallocated frames.

## 5. HypergraphMemorySystem: full-map decay walk under lock every tick — MEDIUM

`Memory/HypergraphMemorySystem.cpp` — `DecayActivations` walks the entire unbounded node map
under a critical section every tick, including nodes already at zero activation.
`EpisodicMemorySystem::DecayMemoryStrengths` has the same shape over AllTraces.

**Fix direction:** maintain a `TSet<int64> ActiveNodeIDs` updated by
`ActivateNode`/`SpreadActivation` and decay only that set, removing IDs that reach zero — or
switch to lazy decay computed from `LastAccessTime` on read.
