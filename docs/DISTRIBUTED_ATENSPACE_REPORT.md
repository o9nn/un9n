# Distributed ATenSpace Integration Report

**Scope:** togai (mobile/Android), flarecog/cogflare/orgflare (platform distributed), plan9cog/cogplan9, and the inferno-cluster lineage (infernos) — Dan's "deploying inferno clusters as a distributed ATenSpace" scaling vision, folded into the Deep Tree Echo integration.

**Date:** July 2026 · **Targets:** o9nn/un9n, orgitcog/u9n

---

## 1. Discovery Findings

| Repo | What it is | Distribution pattern extracted |
|------|------------|-------------------------------|
| **o9nn/ATenSpace** | PyTorch ATen fork with a first-class `aten/src/ATen/atomspace/` module — Atom/Node/Link hypergraph where every node carries a `torch::Tensor` embedding; `querySimilar` does cosine top-k semantic retrieval | The **tensor-embedded AtomSpace API** itself: dedup indices, incoming sets, thread-safe container, semantic query |
| **o9nn/infernos** | Inferno OS fork with an OpenCog kernel (`os/port/opencog.c`, ~700 LOC C) and a `/dev/opencog` device (`os/port/devopencog.c`) exposing cognition as a file namespace over Styx/9P | The **kernel-resident cognitive state** (system goals: system_survival, resource_optimization, distributed_coherence, cognitive_efficiency), the **file-verb protocol** (stats/atomspace/goals/reason/think/attention/patterns/distributed), and the `sync` verb for cluster coherence |
| **o9nn/togai** | 9mly Android AI app (Kotlin/Smali) with a **Cognitive Mesh API** — device nodes register with capability descriptors and exchange cognitive state | **Mesh node registration with roles** (sensor/effector on mobile, inference on GPU nodes, coordinator on platform) |
| **o9nn/flarecog** + **hyperholmes/orgflare** | Distributed AtomSpace on Cloudflare Workers — Durable Objects as atom shards, **dispatch-worker swarms** routing atoms by hash | **Consistent-hash sharding** of atoms to owner nodes; scatter-gather query across shards |
| **cogplan9 / plan9cog** | Plan 9 cognitive namespace work (per-process namespaces, 9P) | Confirms the 9P-file-tree-as-cognition direction already ported via cogdiod (AgentMessageBus, Gap D) |

The literal "distributed ATenSpace" is the composition: **ATenSpace's tensor hypergraph** × **infernos' kernel/namespace protocol** × **togai's mesh registry** × **flarecog's sharding**.

## 2. What Was Built — `DeepTreeEcho/Distributed/`

Three header-only C++17 components (zero external dependencies, matching the cogpy-hpp discipline):

### 2.1 `ATenSpaceLite.h` — the node-local hypergraph
Faithful port of `o9nn/ATenSpace` Atom.h/AtomSpace.h with `torch::Tensor` swapped for plain `std::vector<float>` embeddings so it lives inside the engine without libtorch. Preserved 1:1: the full Type enum (27 atom types incl. Hebbian links), node/link dedup indices, incoming sets with removal protection, and cosine top-k `querySimilar`. Re-binding to real ATen tensors later is a type-swap.

**Extension:** `TruthValue{strength, confidence, count}` (from the infernos kernel struct) with **count-weighted merge** — the algebra that makes distribution converge:

> merge(a, b).strength = (aₛ·a_c + bₛ·b_c) / (a_c + b_c)

### 2.2 `InfernoNamespace.h` — the /dev/opencog protocol
Port of `devopencog.c`: the 8-file synthetic tree (`/stats`, `/atomspace`, `/goals`, `/reason`, `/think`, `/attention`, `/patterns`, `/distributed`) with the exact write-verb dispatch (`create`, `add`, `cycle`, `threshold`, `focus`, `relax`, `sync`) and `CognitiveKernelState` carrying the four infernos system goals with their original urgency/importance values. Transport-agnostic — bindable over AgentMessageBus (the 9P-style bus already in Executive/) to serve remote nodes.

### 2.3 `ClusterAtomSpace.h` — the mesh coordinator
The synthesis layer:

- **Node registry** (togai): `registerNode(id, role)` with roles Coordinator / Inference / Sensor / Effector / Storage — mapping directly onto the CogHood/CogCity/mobile topology
- **Rendezvous (HRW) hashing** (flarecog dispatch-worker): `shardOf(atom)` deterministically assigns owner nodes and is **churn-stable** — removing a node remaps only that node's atoms (verified by test)
- **Delta gossip**: `learnAndPublish` emits sequenced `AtomDelta`s; replicas apply with per-origin high-water marks (idempotent) and TruthValue merge (conflicting observations converge by evidence weight)
- **Anti-entropy sync** (infernos `distributed_coherence`): the `echo sync > /distributed` verb triggers a full coherence pass; repeated syncs are stable (no evidence inflation)
- **Scatter-gather semantic query**: `globalQuery(embedding)` fans out cosine search across all shards and merges rankings
- **Pluggable transport** hook for inter-process clusters via AgentMessageBus or sockets

## 3. Validation

| Repo | Suite | Result | Commit |
|------|-------|--------|--------|
| o9nn/un9n | 275 tests (+22 new) | **275/275 green** | `e7d0a257c` |
| orgitcog/u9n | 1,122 tests (+22 new) | **1,122/1,122 green** | `d7410a0d6` |

The 22 new tests cover: node/link dedup, incoming-set removal protection, cosine ranking, TruthValue merge symmetry, 4-thread concurrent insertion, all namespace file verbs, goal satisfaction relaxation/saturation, mesh registration propagating `distributed_nodes`, sharding determinism + churn stability + spread, replication to all nodes, delta idempotence, evidence convergence under conflict, anti-entropy convergence + stability, namespace-driven cluster sync, scatter-gather ranking, and transport hooks.

## 4. How It Composes With the Existing Integration

```
             ┌─ Sensor node (togai mobile) ──────┐
             │  reservoir → salience → deltas    │
             ▼                                   │
  ClusterAtomSpace ── rendezvous shards ── gossip ── anti-entropy
             ▲                                   │
             │  /dev/opencog namespace verbs     │
             └─ Inference node (CogCity GPU) ────┘
                  LlamaCppBridge / Aphrodite sidecar
```

- **Gap A (reservoir)** patterns become embedded atoms: reservoir state vectors are exactly the `Embedding` payloads `querySimilar` searches over
- **Gap B (executive)** goals mirror the kernel's system-goal table; `Cog0TaskSubsystem` can drive `reasoningCycle`
- **Gap D (IPC)** AgentMessageBus is the natural transport for `AtomDelta` gossip and namespace serving across processes — the same 9P grammar end to end
- **Gap E (endocrine)** attention levels map onto the namespace `/attention` verb, so hormonal state modulates per-node cognitive focus
- **rr.kern (Relevance)** — the RelevanceArbiter can treat cluster-wide `globalQuery` results as its salience input, making relevance realization a *distributed* computation

## 5. Suggested Next Steps

1. **Socket transport**: bind `ClusterAtomSpace::setTransport` to DisTyxPipeService (C#, GTAngel) / TCP so mesh nodes span machines — CogHood coordinator + CogCity inference is the first real deployment
2. **togai binding**: expose the InfernoNamespace over HTTP on the Android side so the 9mly app registers as a Sensor/Effector node
3. **Link-level deltas**: the v1 gossip protocol replicates concept nodes; extend `AtomDelta` with outgoing-hash lists to replicate links (inheritance/Hebbian) for distributed PLN
4. **ECAN over the mesh**: attention values already travel in deltas — add a decay/spread cycle to make attention economy cluster-wide
