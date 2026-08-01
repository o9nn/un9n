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
| `Tests/StandaloneMasteryBindingVerification.cpp` | 21 assertions, runs without UE. |

Build and run the tests (same convention as `GameTraining/Tests/`):

```
g++ -std=c++17 -O2 -o masteryverify Tests/StandaloneMasteryBindingVerification.cpp && ./masteryverify
```

## Melody

Melody is a `FMasteryPersonaProfile` **literal** — no class of her own, no `.moc3`, no texture
path, no asset reference. Her rig attaches at the *backend* layer if and when
`cogpy/live2d-mel` becomes reachable; `MelodyPersona.h` does not change, because nothing in it
knows what renders her.

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

**Not built:** the provider adapters (`UGameSkillTrainingSystem` → signal,
`UGamingMasterySystem` → signal, `UReinforcementLearningBridge`'s `FCognitiveModulation` → affect
channels), the multi-provider aggregator, and the concrete backends (MetaHuman / telemetry /
Live2D). The pure core was built and proven first deliberately — the adapters are mechanical once
the contract is fixed, and the contract is the part worth getting right.

Two design coefficients were corrected by the test rather than by review:

- `Ease` originally summed to 1.3 and saturated, pinning a demonstrative persona at a
  near-maximum grin and making the mouth a *bigger* skill-tell than motion economy — inverting
  the design principle. Coefficients now sum to 1.0.
- Motion economy is not perfectly persona-invariant: composure reduces strain, and less strain
  protects execution. That effect is real and kept; the test isolates the skill-derived component
  at zero strain rather than denying it.
