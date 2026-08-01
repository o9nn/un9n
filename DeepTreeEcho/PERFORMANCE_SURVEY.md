# DeepTreeEcho Performance Survey

Confirmed hot-path performance issues in the wider DeepTreeEcho tree, found during the
GameTraining optimization pass and adversarially verified. The two HIGH-severity findings have
since been fixed (finding 2 turned out to be a correctness bug, not merely a performance one);
the three MEDIUM findings remain open and are documented here with fix directions.

Each finding survived two independent refuters challenging whether the path is actually hot
and whether the cost is real and quantifiable.

**Status: findings 1 and 2 are FIXED and verified; 3–5 remain open.**

| # | Subsystem | Severity | Status |
|---|-----------|----------|--------|
| 1 | OnlineLearningSystem Q-table scans | HIGH | **Fixed** |
| 2 | DeepTreeEchoReservoir weight resampling | HIGH | **Fixed** (verified by `GameTraining/Tests/StandaloneReservoirVerification.cpp`) |
| 3 | ReservoirCognitiveIntegration Hebbian pass | MEDIUM | Open |
| 4 | DNABodySchemaBinding per-joint hashing | MEDIUM | Open |
| 5 | HypergraphMemorySystem decay walk | MEDIUM | Open |

## 1. OnlineLearningSystem: O(|QTable|) full-map scan per RL transition — HIGH — FIXED

`Learning/OnlineLearningSystem.cpp` — `GetMaxQValue` linearly scans the entire QTable with
FString prefix compares, and runs **twice per recorded transition**. With thousands of
state-action entries this is the dominant per-transition cost. `RecordExperience` also evicts
its buffer with `RemoveAt(0)` (~90KB memmove per transition once full).

**Applied fix:** added a `StateActionIndex` (`TMap<State, TArray<Action>>`) secondary index over
QTable, kept in sync by `UpdateQValue` and cleared alongside QTable in
`InitializeLearningSystem`/`ResetLearning`. `GetMaxQValue` and `GetBestAction` now examine only
the target state's actions. `UpdateQValue` collapsed from four hash lookups (`Contains` + two
`operator[]` + trailing `Add`) to one `FindOrAdd`, with the successor's max-Q computed before
taking the reference so a rehash cannot invalidate it. Not-found semantics preserved exactly
(0.0f for an unseen state; empty string for no best action).

*Still open in this file:* `RecordExperience` evicts its buffer with `RemoveAt(0)`.

## 2. DeepTreeEchoReservoir: connectivity regenerated per input — HIGH — FIXED

`Reservoir/DeepTreeEchoReservoir.cpp` — `ProcessInput` regenerated random reservoir
connectivity on every call: ~11,000 RNG invocations (O(Units²)) per input, ~600k per
movement-embedding query. This was **not only a performance problem**: resampling W every step
destroys the Echo State Property, so the reservoir could not encode temporal structure at all —
its output was filtered noise. Two further defects were found in the same loop: `InputSum` did
not depend on the unit index (every unit received an identical input drive, so there was no
input diversity), and `SpectralRadius` was applied as a post-hoc multiplier on an unnormalized
sum rather than as an actual spectral radius.

**Applied fix:** `FReservoirState` now carries a fixed sparse recurrent matrix in CSR form
(`RecurrentRowStart`/`RecurrentColIndex`/`RecurrentWeight`) plus a dense `Units × 16` input
matrix, generated once by `EnsureWeightsBuilt` from a persisted `WeightSeed` and normalized to
the configured spectral radius by power iteration. `ProcessInput` is now a fixed sparse mat-vec
with zero RNG draws. The weight arrays are deliberately non-`UPROPERTY` (derived data,
regenerable from the seed; reflected nested containers are illegal in UHT anyway).

**Verified** by `GameTraining/Tests/StandaloneReservoirVerification.cpp` (6/6): trajectory
reproducibility, Echo State Property contraction (initial-state separation 18.0 → 0.0002 over
60 steps), input sensitivity, spectral-radius accuracy, per-unit input diversity, and zero
hot-path RNG draws. The harness caught a real bug during development — a fixed 20-iteration
power iteration under-estimated the eigenvalue by ~2.8%, overshooting every requested radius by
the same factor; normalization now iterates to convergence.

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
