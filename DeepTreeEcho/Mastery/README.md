# Mastery Embodiment Binding

Makes measured competence **visible in a body**. Skill systems know how good the agent is; avatar
systems know how to move a face. Nothing joined them. This does.

The core claim: **expertise is legible far more in motion economy than in facial expression.** An
expert does not visibly hurry, does not overcorrect, does not fidget, and their gaze arrives where
it needs to be before the event does. A binding that expressed mastery mainly through the mouth
would read as smugness, not skill — so competence drives `MotionEconomy`, `GazeSteadiness`,
`ReactionSharpness` and (inversely) `IdleFidget` much harder than it drives smiling.

## The invariant this exists to protect

> **UNKNOWN IS NOT EXCELLENCE.**

An avatar that carries herself like a grandmaster because her competence was never sampled is
worse than one that looks like a novice — it is a confident lie about the system's own state.

Enforced by **two independent guards**, deliberately redundant because a single guard is one edit
away from being removed:

1. **Guard 1 (per-channel).** Every channel read goes through `Get<Channel>(IfUnknown)`, forcing
   the caller to state a fallback at the point of use. All fallbacks lean novice.
2. **Guard 2 (global).** `ExpressionIntensity` is capped by `FMasterySignal::GetSignalTrust()`,
   which multiplies channel coverage × saturating evidence count × freshness. Even if Guard 1 were
   subverted, an untrusted signal cannot drive a strong pose.

Verified adversarially: a signal claiming `EvidenceCount = 10000` with **zero** measured channels
yields trust `0.0` and stays pinned at the persona's unknown-state cap.

## Architecture

```
   producers  ->  FMasterySignal  ->  [ + FMasteryPersonaProfile ]  ->  FMasteryEmbodimentPose  ->  backend
  (any system)   13 channels,          persona = pure data,            FACS AUs + body/motion      (MetaHuman,
                 each with a           never a subclass                scalars, rig-neutral         Live2D, …)
                 validity bit
```

`MasteryEmbodimentBinding::Evaluate` is a **pure function** — no rig, no component, no world.
That is what makes it testable without Unreal.

| File | Role |
|---|---|
| `MasterySignal.h` | The narrow contract. Zero deps beyond `CoreMinimal`. Mentions no skill/combo/RL type, so any producer can satisfy it. |
| `MasteryEmbodimentPose.h` | Backend-neutral output. No Live2D parameter IDs, no MetaHuman curve names — a backend cannot leak its vocabulary upward because there is nowhere to put it. |
| `MasteryPersonaProfile.h` | Persona as data. Temperament modulates presentation; it never grants competence. |
| `MasteryEmbodimentBinding.{h,cpp}` | All rule evaluation, plus frame-rate-independent smoothing. |
| `Personas/MelodyPersona.h` | Melody, as a struct literal. No class, no asset path. |
| `Personas/CompetitivePersonas.h` | Aion and Toga temperaments, plus a weighted `Blend()`. |
| `Competitive/CounterAdaptivePolicy.h` | Safe exploitation — see the competitive section below. |
| `Backends/MelodyLive2DBackend.h` | The only file that knows a rig-specific parameter name. |
| `Tests/StandaloneMasteryBindingVerification.cpp` | 21 assertions, runs without UE. |
| `Tests/StandaloneCompetitiveVerification.cpp` | 10 assertions, 8-opponent tournament. |

Build and run the tests (same convention as `GameTraining/Tests/`):

```
g++ -std=c++17 -O2 -o masteryverify Tests/StandaloneMasteryBindingVerification.cpp && ./masteryverify
```

## Melody

Melody is a `FMasteryPersonaProfile` **literal** — no class of her own, no `.moc3`, no texture
path, no asset reference. Her rig attaches at the *backend* layer — which is exactly what
happened: `live2d-mel` arrived as a direct upload, and `Backends/MelodyLive2DBackend.h` was added
without changing a single line of `MelodyPersona.h`, the binding rules, or the pose struct. The
backend-agnostic boundary paid for itself.

Her profile is temperament only: demonstrative (`ConfidenceDisplayStyle 0.72`), composed
(`0.70`), legibly absorbed when in flow (`0.80`), playful (`0.55`). **Melody at competence 0.1
looks like a cheerful novice.** The profile cannot grant skill she has not demonstrated — test 3
asserts exactly this.

## Verified facts this design depends on

Established by reading the actual files, not assumed. Re-check before relying on them.

- **`un9n` HAS `Source/DeepTreeEcho/GamingMasterySystem.{h,cpp}`** with the real flow model
  (`FFlowStateMetrics`, `IsInFlowState()`, `FOnFlowStateEntered`). It has exactly **one**
  `UGamingMasterySystem`, so no duplicate-UCLASS collision.
- **`u9n` has a DIFFERENT `UGamingMasterySystem`** at `DeepTreeEcho/GameTraining/` (reflex tiers,
  combos, opponent modelling, timing windows — but no flow model), *plus* a second one at
  `Source/DeepTreeEcho/`. Those two declare the same `UCLASS` and the same `USTRUCT FOpponentModel`,
  so **they cannot both compile into one module**. Porting u9n's version into `un9n` is not merely
  undesirable, it is not compilable. Hence the narrow-interface approach.
- **`un9n` HAS a Live2D stack** at `UnrealEcho/Live2DCubism/` (`Live2DCubismAvatarComponent`,
  `ExpressionSynthesizer`, `CubismSDKIntegration`, `PhysicsDeformer`) — but `UnrealEcho/` contains
  **only one `.Build.cs`** (`TensorLogic/`). Live2D is therefore **not a linkable module**. Do not
  depend on it. This is the strong reason for backend-agnosticism, beyond mere preference.

## Status

**Built and verified:** the signal contract, the pose currency, the persona profile, the binding
rules, the Melody persona, and the standalone test (21/21).

**Also built since:** the competitive layer (below), Aion/Toga personas with blending, and
Melody's Live2D backend.

**Still not built:** the provider adapters (`UGameSkillTrainingSystem` → signal,
`UGamingMasterySystem` → signal, `UReinforcementLearningBridge`'s `FCognitiveModulation` → affect
channels), the multi-provider aggregator, and a MetaHuman backend. The signal contract is still
fed by synthetic data in tests rather than by live systems — that is the next increment, and it is
mechanical now that the contract is fixed.

Two design coefficients were corrected by the test rather than by review:

- `Ease` originally summed to 1.3 and saturated, pinning a demonstrative persona at a
  near-maximum grin and making the mouth a *bigger* skill-tell than motion economy — inverting
  the design principle. Coefficients now sum to 1.0.
- Motion economy is not perfectly persona-invariant: composure reduces strain, and less strain
  protects execution. That effect is real and kept; the test isolates the skill-derived component
  at zero strain rather than denying it.

---

## Competitive layer — "undefeated"

`Competitive/CounterAdaptivePolicy.h` + `Tests/StandaloneCompetitiveVerification.cpp`.

**Undefeated is defined as: no opponent achieves a winning record.** Not "wins every match" —
that claim is false and unachievable. Against a Nash-equilibrium opponent nobody gains expected
value, ever; that is arithmetic, not skill. A benchmark demanding "always wins" could only be
passed by rigging it.

The policy interpolates equilibrium ↔ soft best-response by an exploitation weight λ that must be
*earned*, held down by two independent brakes (mirroring the embodiment binding's two-guard
pattern):

- **Brake 1 (evidence)** — λ scales with the opponent's distance from equilibrium × saturating
  sample count, taken over whichever of the **marginal** or **sequential** model currently
  predicts better.
- **Brake 2 (results)** — a performance monitor. Sustained negative results while deviating is
  the signature of being counter-read, so λ is cut multiplicatively and recovers slowly (much
  slower than it retreats, so an opponent cannot bait it back out with a few soft rounds).

### Anti-rigging

Five of the eight opponents exist specifically to defeat an exploiter: `FrequencyCounter`
(best-responds to *our* history), `RegretMatcher` (no-regret learner), `NashPlayer`
(mathematically unbeatable), `Trapper` (plays exploitably to bait deviation, then counters the
bias it induced), and `Oracle` (**sees our exact mixed strategy each round**). Greedy exploitation
loses to all five; pure equilibrium beats nobody. Passing requires the tradeoff to genuinely work.

### Results (3000 rounds × 5 seeds)

| Opponent | EV/round | Record |
|---|---|---|
| Nash (equilibrium) | +0.004 | draw — optimal |
| Biased (70% rock) | +0.189 | win |
| Cycler (R,P,S…) | +0.703 | win |
| FrequencyCounter | +0.190 | win |
| RegretMatcher | +0.020 | win |
| WinStayLoseShift | +0.053 | win |
| Trapper | +0.152 | win |
| Oracle (sees our strategy) | +0.023 | draw — brake retreats to 0.70 |

### Two honest notes on the numbers

- **A pure best-response scores ~0.55 against the biased opponent; we score ~0.19.** That gap is
  the price of the safety property, not a defect — a pure BR is itself a pure strategy and loses
  outright to the five counter-exploiters. The assertion bar is set at "meaningfully exploits"
  rather than at the unconstrained optimum, because tuning until 0.55 would mean overfitting this
  benchmark and forfeiting the record it exists to verify. `MaxExploitation` was swept
  (0.75/0.85/0.95); all stayed undefeated, and 0.85 was kept as the middle with margin.
- **The sequential model was added because the harness caught its absence.** A deterministic
  cycler is perfectly predictable yet has an exactly *uniform* marginal distribution, so the
  original frequency-only model scored **0.004** against it — seeing equilibrium where there was
  total predictability. Conditioning on the opponent's previous action took it to **0.703**.

Two further test errors were mine, not the code's: the brake assertion originally fired against
`FrequencyCounter`, which she had *started beating* (a brake is a losing-position mechanism, so
`Oracle` was added to test it properly); and the first threshold pair was set from intuition
rather than from the game's theoretical maxima.

## Personas

`Personas/CompetitivePersonas.h` adds Aion (chaotic, near-total composure, deliberately illegible
effort) and Toga (predatory focus, low composure, highest flow legibility), plus a weighted
`Blend()`.

`Blend` takes the **minimum** `UnknownStateIntensityCap` rather than the weighted average — the
"unknown is not excellence" ceiling must be the most conservative of its parents, or persona
fusion becomes a laundering route around the invariant.

## Melody's rig

`Backends/MelodyLive2DBackend.h` — the only file in the module that knows a rig-specific
parameter name.

Her model binary (`miara_pro_t03.moc3`) is served from external storage and is not in the
`live2d-mel` repository, so her parameter list could not be fully enumerated. **Confirmed** from
`melody.model3.json`: `ParamEyeLOpen`, `ParamEyeROpen` (EyeBlink), `ParamMouthOpenY` (LipSync),
and motion groups `Idle` / `Tap` / `Flick`. Everything else is standard-Cubism naming and is
marked **assumed**, written through `ApplyOptional()` so a missing ID degrades one channel
silently instead of erroring or writing to the wrong parameter. Verify and promote the assumed
list when the `.moc3` is available.
