# Goal: un9n avatar=complete — avatar pipeline builds green and is verified

## User Request

`/goal ( un9n [ avatar=complete ] )` — bring the un9n avatar system to a
verifiably complete state.

## Refined Goal

CLAUDE.md marks "DNA → Body Schema Binding" as In Progress while the code for it
already exists (`DeepTreeEcho/4ECognition/DNABodySchemaBinding.*`,
`DeepTreeEcho/Avatar/` — 54 files, plus `AvatarIntegrationE2E` suites). "Complete"
means: the standalone quality gate actually passes. Right now it does NOT — the
baseline build fails on MinGW g++ 16.1 with `error: 'M_PI' was not declared in this
scope` in three test files (strict `-std=c++17` does not define `M_PI` from <cmath>).
Fix the build, get all unit + E2E test suites (including the avatar suites) compiling
and passing, and update CLAUDE.md's status table so documentation matches reality.

## Acceptance Criteria

- [ ] Criterion 1: `cmake --build build` completes with exit code 0 (no compile
      errors) using the toolchain below.
- [ ] Criterion 2: `ctest --test-dir build -L unit` passes (0 failures).
- [ ] Criterion 3: `ctest --test-dir build -L e2e` passes (0 failures), including
      `AvatarIntegrationE2E` (AvatarIntegrationE2ETest, DNACalibrationE2ETest,
      AvatarPerformanceE2ETest, AvatarCognitiveIntegrationE2ETest).
- [ ] Criterion 4: The `M_PI` fix is portable (do not rely on `_USE_MATH_DEFINES`
      alone; MinGW/strict-C++17-safe — e.g. a shared constant or guarded define),
      applied consistently to all affected files.
- [ ] Criterion 5: CLAUDE.md "Current Status" table updated: "DNA → Body Schema
      Binding" moves to Complete (with tests passing as evidence) — only if
      criteria 1–3 hold.
- [ ] Criterion 6: No modifications to `Engine/` (Epic engine code) or
      `ReservoirEcho/external/` (vendored third-party).

## Scope Boundaries

**In scope:**
- `DeepTreeEcho/Testing/**` test sources, `DeepTreeEcho/**` avatar/cognition code
  if a real defect blocks tests, `CMakeLists.txt` if needed, CLAUDE.md status table.

**Out of scope:**
- Unreal Engine editor/runtime work (`Engine/`, `.uproject` builds), Maya/FBX asset
  production, Live2D parity, OpenCog integration, vendored third-party code.
- Weakening tests to make them pass (no deleting/skipping assertions).

## Applicable Project Conventions

**Quality gate command (Windows, this machine):**
```powershell
$env:PATH = "C:\msys64\mingw64\bin;$env:PATH"
cmake -B build -G Ninja -DCMAKE_CXX_COMPILER="C:/msys64/mingw64/bin/g++.exe" -DCMAKE_C_COMPILER="C:/msys64/mingw64/bin/gcc.exe" -DBUILD_TESTING=ON -DBUILD_E2E_TESTS=ON
cmake --build build
ctest --test-dir build -L unit --output-on-failure
ctest --test-dir build -L e2e --output-on-failure
```
(build/ is already configured; reconfigure only if you change CMakeLists.txt)

**Commit convention:**
- Conventional commits seen in history (`fix(scope): …`, `feat(scope): …`), ≤72 chars.
- Builder marker required: `type(scope): [B] description`
- Inspector marker required: `chore(scope): [I] description`
- Assisted-by trailer required (Builder: `Assisted-by: Claude:Sonnet-4.6`,
  Inspector: `Assisted-by: Claude:Haiku-4.5`)

**Guidelines:**
- CLAUDE.md (repo root) — architecture, guarded joints (`neck_01`, `neck_02`,
  `FACIAL_C_FacialRoot` must never be dropped/renamed), common tasks.
- PROJECT_BRIEF.md §9 — security rules (no secrets, vendored code stays intact).

**Rules:**
- Known baseline failures to fix (from baseline build 2026-08-06):
  - `DeepTreeEcho/Testing/E2E/CognitivePipelineE2E.cpp:501,504` — `M_PI` undeclared
  - `DeepTreeEcho/Testing/UnitTests/ReservoirIntegrationTests.cpp:707` — `M_PI` undeclared
  - `DeepTreeEcho/Testing/E2E/ReservoirCognitiveE2E.cpp:392` — `M_PI` undeclared
  - Grep for other `M_PI` uses in DeepTreeEcho/ and fix them all at once.
