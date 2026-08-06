# PROJECT_BRIEF.md — un9n (Deep Tree Echo / UnrealEngineCog)

> Last updated: 2026-08-06 | Sprint 1 | Status: In Progress

## 1. Project Overview

**un9n** is a unified cognitive architecture for embodied AI avatars. It fuses the Deep Tree
Echo cognitive framework (12-step cycle, 3 concurrent consciousness streams), ReservoirCpp
echo state networks, 4E embodied cognition, and MetaHuman DNA avatar rigs — all layered onto
an Unreal Engine 5.3 fork (**UnrealEngineCog**). The goal: AI characters with persistent
memory, adaptive personality, and real-time embodied expression.

## 2. Concept / Product Description

The system runs a 12-step cognitive loop with three streams phased 120° apart
(Perceiving / Acting / Reflecting), synchronized at triadic points {1,5,9} {2,6,10}
{3,7,11} {4,8,12}. Nested execution contexts follow OEIS A000081 rooted-tree enumeration.

Key user-facing capabilities:
- **Embodied avatars** — MetaHuman DNA rigs driven by cognitive state (expressions, poses)
- **4E cognition** — body schema, affordances, sensorimotor contingencies, extended memory
- **Narrative memory** — diary → insight → blog loop for self-aware characters
- **Reservoir computing** — hierarchical ESNs for temporal pattern recognition
- **Personality evolution** — trait systems that adapt through interaction

Two build surfaces:
1. **Standalone cognitive core** — CMake + GTest, no UE required (fast iteration)
2. **Full engine** — UnrealEngineCog.uproject, UE 5.3 fork with cognitive components

## 3. Tech Stack

- **Core:** C++17, CMake ≥3.14, Eigen 3.4.0 (bundled fallbacks: 3.3.7), Threads
- **Engine:** Unreal Engine 5.3 fork (UCLASS/UPROPERTY reflection, Actor components)
- **Avatar:** MetaHuman DNA Calibration (Python 3.7/3.9, Maya 2022–2024), Live2D Cubism
- **Testing:** GoogleTest v1.14 (FetchContent fallback), CTest labels `unit` / `e2e`
- **CI/CD:** GitHub Actions (18 workflows — PR validation, standalone tests, nightly, releases)

## 4. Architecture

```
┌─────────────────────────────────────────────────────┐
│           UnrealEcho (UE 5.3 components)            │
│  Avatar · Animation · Audio · Consciousness ·       │
│  Neurochemical · Personality · Rendering            │
└──────────────────┬──────────────────────────────────┘
                   │ Actor components / Tick
┌──────────────────▼──────────────────────────────────┐
│        DeepTreeEcho (cognitive core, C++17)         │
│  Core (12-step cycle) · 4ECognition · Avatar        │
│  ActiveInference · Entelechy · System5 · Wisdom     │
└────────┬─────────────────────────────┬──────────────┘
         │                             │
┌────────▼─────────────┐  ┌────────────▼──────────────┐
│  ReservoirEcho       │  │  MetaHuman-DNA-Calibration│
│  Hierarchical ESNs   │  │  DNACalib · dna_viewer    │
│  (Eigen-backed)      │  │  rig/expression pipeline  │
└──────────────────────┘  └───────────────────────────┘
```

## 5. Key Files Map

| Area | Path | Contents |
|------|------|----------|
| Standalone build | `CMakeLists.txt` | Core + tests build (Eigen discovery, GTest) |
| Cognitive core | `DeepTreeEcho/` | Core, Reservoir, 4ECognition, Avatar, System5… |
| Unit tests | `DeepTreeEcho/Testing/UnitTests/` | GTest suites (label `unit`) |
| E2E tests | `DeepTreeEcho/Testing/E2E/` | One executable per scenario (label `e2e`) |
| ESN library | `ReservoirEcho/reservoircpp_cpp/` | Reservoir computing implementation |
| UE components | `UnrealEcho/` | 106 files of engine-side cognitive components |
| Avatar tooling | `MetaHuman-DNA-Calibration/` | DNA files, calibration lib, Maya viewer |
| UE project | `UnrealEngineCog.uproject` | Engine fork entry point (UE 5.3) |
| CI | `.github/workflows/` | 18 pipelines (see §11) |
| Personas | `.github/agents/` | 101 agent definitions (incl. `aion.md`) |
| Sprint docs | `docs/sprint-N/` | plan.md, progress.md, done.md per sprint |

## 6. Team Roles

| Agent | Name | Role |
|-------|------|------|
| Producer | Remy | Sprint plans, coordination, merging, issue triage |
| Cognitive Core | Sage | `DeepTreeEcho/` + `ReservoirEcho/` C++ — cycle, memory, ESNs |
| UE Integration | Nova | `UnrealEcho/` components, Actor/Tick wiring, Blueprints API |
| Avatar/Expression | Milo | MetaHuman DNA binding, materials, animation, visual polish |
| Cognition Design | Kira | 4E specs, cognitive cycle design, persona coherence |
| QA | Ivy | GTest/CTest suites, CI green-keeping, sign-offs |
| DevOps | Dash | GitHub Actions, release builds, Docker, Git LFS |
| Chaos Reviewer | Aion | Red-team reviews, paradox hunting (`.github/agents/aion.md`) |

## 7. Sprint Status

| Sprint | Name | Status | Scope |
|--------|------|--------|-------|
| 0 | Architecture & Core | ✅ Done | 12-step cycle, 3 streams, 4E cognition, ESNs, avatar evolution, personality/neurochemical systems, CI |
| 1 | Embodiment Binding | 🔨 In Progress | DNA → Body Schema binding, memory integration into the cycle |

## 8. Current State (rewrite every sprint)

**What works:**
- 12-step cognitive cycle with 3 concurrent streams + triadic synchronization
- 4E embodied cognition components (Embodied/Embedded/Enacted/Extended)
- Hierarchical reservoir computing (ReservoirCpp integrated, Eigen-backed)
- Avatar evolution, personality traits, neurochemical simulation
- MetaHuman DNA tooling (load/modify/calibrate DNA, Maya rig generation)
- Standalone CMake build with GTest unit + E2E suites; 18 CI workflows

**What doesn't work yet:**
- DNA → Body Schema binding (in progress — rig channels not yet feeding proprioception)
- Memory integration (episodic/semantic recall not yet wired into cycle steps)
- OpenCog integration (planned, not started)

**What's next:**
- Sprint 1: complete the embodiment binding + memory integration (see `docs/sprint-1/plan.md`)

## 9. Security Rules

1. Secrets live in environment variables / GitHub Actions secrets only — never in code or git.
2. No credentials or tokens in DNA files, .uproject, or config; CI uses repo secrets.
3. MetaHuman DNA assets and Unreal Engine code are governed by the Epic EULA — do not
   redistribute engine or DNA assets outside license terms.
4. Large binary assets go through Git LFS, never raw commits.
5. Third-party code stays in its vendored location (`ReservoirEcho/external/`,
   `Engine/Source/ThirdParty/`) with licenses intact.

## 10. How to Run Locally

**Standalone cognitive core (no UE needed):**
```bash
cmake -B build -DBUILD_TESTING=ON -DBUILD_E2E_TESTS=ON
cmake --build build --config Release
ctest --test-dir build -L unit          # unit suites
ctest --test-dir build -L e2e           # end-to-end scenarios
```

**Full engine (Windows, UE 5.3):**
```bash
# From repo root
Setup.bat
GenerateProjectFiles.bat
# Open UE5.sln, build UnrealEngineCog target, launch UnrealEngineCog.uproject
```

**MetaHuman DNA tooling:** Python 3.7/3.9 + Maya 2022–2024; see
`MetaHuman-DNA-Calibration/examples/`.

## 11. How to Deploy

CI/CD is GitHub Actions (`.github/workflows/`):
- **PR gate:** `pr-validation.yml`, `ci-standalone-tests.yml`, `ci-code-quality.yml`
- **Module CI:** `ci-deep-tree-echo.yml`, `ci-reservoir-echo.yml`, `ci-avatar-module.yml`,
  `ci-metahuman-dna.yml`, `ci-uht-fullengine.yml`, `ci-integration.yml`
- **Nightly:** `nightly.yml`, `nightly-cognitive.yml`, `nightly-builds.yml`,
  `ci-cognitive-benchmarks.yml`
- **Release:** `release.yml`, `release-builds.yml` (CPack: ZIP on Windows, TGZ/DEB elsewhere)
- **Security:** `ci-security-scan.yml`; **Docker:** `ci-docker-build.yml`

Releases are cut by tagging; CPack packages the standalone core (`un9n-<version>`).

## 12. Cross-Chat Handoff Protocol

Every sprint chat must do these before finishing:

1. Write `docs/sprint-N/done.md` — what was built, what's not done, what needs manual
   setup, files changed/created
2. Update PROJECT_BRIEF.md: Section 7 (mark sprint done) + Section 8 (rewrite current state)
3. Commit all changes with descriptive message: `sprint-N: <summary>`

This is how context survives across chats. If skipped, the next chat starts blind and may
overwrite or duplicate work. The repo is the shared memory — keep it accurate.

## 13. Bug & Fix Tracking

Bugs are tracked as GitHub Issues on `o9nn/un9n`. Single source of truth for all teams.

**For QA (Ivy):** File bugs as GitHub Issues with labels (`bug`,
`severity:blocker/major/minor`). Include: component (DeepTreeEcho / ReservoirEcho /
UnrealEcho / MetaHuman-DNA), steps to reproduce, expected vs actual. When no blockers
found: write `docs/qa/sprint-N-signoff.md` with test count, pass rate, explicit
"no blockers" statement.

**For Dev Team (Sage/Nova/Milo):** Check GitHub Issues before starting work. Fix blockers
and majors before polish. Use GitHub closing keywords in commits:
`fix: description (Fixes #42)`. For reference-only, use `Refs #42`.

**For DevOps (Dash):** File infrastructure issues with label `infra`.

**For feature ideas:** add to `docs/ideas-backlog.md`.

## 14. Multi-Repo Setup

Each team works in their own separate clone of the repo. No shared checkouts. Everyone
works on their own branch, pushes to origin, creates PRs.

**Teams:**
- Producer on `main` (coordination hub)
- Dev Team on `feature/sprint-N`
- QA on `feature/qa-N`
- DevOps on `feature/devops-N` (only when needed)

**Setup:**
```bash
git clone https://github.com/o9nn/un9n.git un9n-<team>
cd un9n-<team>
git checkout -b <branch-name>
cmake -B build -DBUILD_TESTING=ON   # standalone core sanity check
```

**Branch strategy:** Feature branches → PR → regular merge to main. Never push directly
to main. Never squash. Never rebase feature branches (causes commit loss).
