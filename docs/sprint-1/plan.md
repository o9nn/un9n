# Sprint 1 — Embodiment Binding

> Sprint Goal: MetaHuman DNA rig data feeds the 4E body schema, and episodic/semantic
> memory recall participates in the 12-step cognitive cycle — both verified by tests.
> Branch: feature/sprint-1
> Estimated effort: ~2 dev-days

## Prioritized Task List

| # | Task | Owner | Est | Description |
|---|------|-------|-----|-------------|
| 1 | DNA → Body Schema binding | Milo | 4h | Map DNA joints/blendshape channels (via DNACalib reader) into `UEmbodiedCognitionComponent` proprioceptive body schema; preserve `neck_01`, `neck_02`, `FACIAL_C_FacialRoot` |
| 2 | Body schema tick wiring | Nova | 2h | Feed live rig pose from UnrealEcho avatar components into the embodied state each tick; expose BlueprintReadOnly snapshot |
| 3 | Episodic memory hooks | Sage | 3h | Wire episodic store writes at Reflecting steps (3,6,9,12) and recall at Perceiving steps (1,4,7,10) of the cycle |
| 4 | Semantic recall in relevance path | Sage | 2h | Similarity-based recall feeding the relevance realization step; cap recall latency per tick |
| 5 | Unit tests: binding + memory | Ivy | 3h | GTest suites in `DeepTreeEcho/Testing/UnitTests/` covering channel mapping, guarded joints, memory write/recall ordering |
| 6 | E2E: cycle with memory | Ivy | 2h | E2E scenario in `DeepTreeEcho/Testing/E2E/`: run N cycles, assert recalled context influences stream state |

## Work Schedule

### Phase 1: Embodiment (tasks 1–2)
- Build DNA channel → body schema mapping, then live tick wiring
- Checkpoint commit after phase

### Phase 2: Memory (tasks 3–4)
- Episodic write/recall hooks, semantic recall in relevance path
- Checkpoint commit after phase

### Phase 3: Verification & Integration (tasks 5–6)
- Unit + E2E suites green locally (`ctest -L unit`, `ctest -L e2e`)
- Bug fixes, final commit, PR

## Success Criteria

- [ ] DNA rig channels appear in body schema state (verified by unit test)
- [ ] Guarded joints (`neck_01`, `neck_02`, `FACIAL_C_FacialRoot`) never dropped/renamed
- [ ] Episodic writes occur only at Reflecting steps; recall only at Perceiving steps
- [ ] E2E scenario shows memory-influenced stream state across ≥3 full cycles
- [ ] `ctest -L unit` and `ctest -L e2e` pass
- [ ] CI green on `pr-validation.yml` + `ci-standalone-tests.yml`

## What's NOT in This Sprint

| Feature | Reason |
|---------|--------|
| OpenCog/AtomSpace integration | Planned for a later sprint; large surface area |
| Maya/FBX asset production | Asset phase runs separately (see NEXT_STEPS.md) |
| Live2D avatar parity | 3D MetaHuman path first; 2D follows |
| Diary → blog narrative loop | Depends on memory integration landing first |

## Agent Prompt

> Read PROJECT_BRIEF.md, then read docs/sprint-1/plan.md. Execute Sprint 1.
>
> First: git pull origin main && git checkout -b feature/sprint-1
>
> Close GitHub Issues in commits: "fix: description (Fixes #NN)"
> Update docs/sprint-1/progress.md after each phase.
> When done, push and create PR: git push origin feature/sprint-1
> Follow Sections 12-14 of PROJECT_BRIEF.md.
