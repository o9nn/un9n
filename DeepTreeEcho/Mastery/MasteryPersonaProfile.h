// MasteryPersonaProfile.h
// A persona is DATA, never a subclass.
//
// This is the mechanism that keeps personas from becoming code. Two performers at identical
// measured competence should not look identical: one goes quiet and still under pressure,
// another gets louder and more animated. That difference is temperament, not skill - so it
// belongs in a data profile that modulates the universal binding rules, not in a bespoke
// class per character.
//
// Melody is an instance of this struct. So is anyone else. No persona ever gets its own type.

#pragma once

#include "CoreMinimal.h"

struct FMasteryPersonaProfile
{
    /** Display name. Identification only - no behavior keys off it. */
    FString PersonaName = TEXT("Default");

    // ---- Expressive temperament -------------------------------------------------------------

    /** Global gain on facial expression. <1 = reserved/deadpan, >1 = animated. */
    float ExpressivenessGain = 1.0f;

    /** Resting warmth - baseline smile carried even at neutral. */
    float BaselineWarmth = 0.0f;

    /** Bias toward visible tension under load. Low = unflappable; high = visibly strains. */
    float TensionBias = 0.0f;

    /** Bias toward restless movement at rest. Low = still; high = fidgety. */
    float FidgetBias = 0.0f;

    /** Bias toward playful head tilt / asymmetry. */
    float PlayfulnessBias = 0.0f;

    // ---- How competence is displayed --------------------------------------------------------

    /**
     * Confidence display style, [0,1].
     *   0 = understated: mastery shows as stillness, economy, near-blankness.
     *   1 = demonstrative: mastery shows as visible ease, smiling, expansive posture.
     * This is the main axis along which two equally-skilled personas read differently.
     */
    float ConfidenceDisplayStyle = 0.5f;

    /**
     * Composure under pressure, [0,1]. How much high challenge-skill imbalance is allowed to
     * surface as visible strain. 1 = ice; 0 = wears everything on her face.
     */
    float Composure = 0.5f;

    /**
     * Flow legibility, [0,1]. How visibly absorption reads to an observer. High = obvious
     * "in the zone" stillness; low = flow looks much like ordinary play.
     */
    float FlowLegibility = 0.5f;

    // ---- Physiological baselines ------------------------------------------------------------

    /** Resting blinks per minute. */
    float BaseBlinkRate = 15.0f;

    /** Resting breaths per minute. */
    float BaseBreathRate = 14.0f;

    /** Resting posture uprightness. */
    float BasePostureUprightness = 0.5f;

    // ---- Safety -----------------------------------------------------------------------------

    /**
     * Ceiling on ExpressionIntensity when signal trust is zero, [0,1]. Bounds how expressive an
     * unmeasured persona may be. Keeping this well below 1 is what stops an unmeasured avatar
     * from performing a competence nobody has observed.
     */
    float UnknownStateIntensityCap = 0.25f;

    /** Neutral, unopinionated profile. */
    static FMasteryPersonaProfile Default()
    {
        return FMasteryPersonaProfile();
    }
};
