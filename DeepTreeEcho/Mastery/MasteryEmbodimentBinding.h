// MasteryEmbodimentBinding.h
// Signal + Persona -> Pose. All rule evaluation lives here.
//
// The binding is a PURE FUNCTION of (FMasterySignal, FMasteryPersonaProfile) plus an optional
// temporal smoothing state. It touches no rig, no component, no world. That is what makes it
// testable without Unreal - see Tests/StandaloneMasteryBindingVerification.cpp.
//
// DESIGN PRINCIPLE - what actually reads as mastery:
// Expertise is legible far more in motion economy than in facial expression. The expert does
// not visibly hurry, does not overcorrect, and does not fidget; their gaze arrives where it
// needs to be before the event does. So competence drives MotionEconomy, GazeSteadiness,
// ReactionSharpness and (inversely) IdleFidget far more strongly than it drives smiling.
// A binding that expressed mastery mainly through the mouth would read as smugness, not skill.

#pragma once

#include "CoreMinimal.h"
#include "MasterySignal.h"
#include "MasteryEmbodimentPose.h"
#include "MasteryPersonaProfile.h"

/** Optional smoothing state so poses ease between frames instead of snapping. */
struct FMasteryBindingState
{
    FMasteryEmbodimentPose SmoothedPose = FMasteryEmbodimentPose::Neutral();
    bool bInitialized = false;
};

namespace MasteryEmbodimentBinding
{
    /**
     * Core rule evaluation. Deterministic, side-effect free.
     *
     * The "unknown is not excellence" invariant is enforced in two independent places, because
     * a single guard is one edit away from being removed:
     *   1. Every channel read supplies an explicit novice-leaning fallback for unmeasured data.
     *   2. ExpressionIntensity is capped by signal trust, so even if (1) were subverted, an
     *      untrusted signal cannot drive a strong pose.
     */
    FMasteryEmbodimentPose Evaluate(const FMasterySignal& Signal, const FMasteryPersonaProfile& Persona);

    /**
     * Evaluate then exponentially smooth toward the result. HalfLifeSeconds controls responsiveness;
     * momentary channels legitimately move fast, so this is deliberately short by default.
     */
    FMasteryEmbodimentPose EvaluateSmoothed(const FMasterySignal& Signal,
                                            const FMasteryPersonaProfile& Persona,
                                            FMasteryBindingState& State,
                                            float DeltaTime,
                                            float HalfLifeSeconds = 0.25f);
}
