// MasterySignal.h
// The narrow mastery-signal contract.
//
// This is the ONLY thing the embodiment binding knows about competence. It deliberately mentions
// no skill, combo, opponent, reservoir, or Q-table type, so that any producer - present or future,
// in this repo or another - can satisfy it without the binding growing a dependency on it.
//
// EVERY channel carries an explicit validity bit. That is not defensive padding; it encodes the
// single invariant this whole subsystem exists to protect:
//
//     UNKNOWN IS NOT EXCELLENCE.
//
// A channel nobody has measured must never render as mastery. An avatar that carries herself like
// a grandmaster because her competence was never sampled is worse than one that looks like a
// novice - it is a confident lie about the system's own state. Consumers must branch on the
// validity bit, never on a sentinel value, because every plausible sentinel (0.0, -1.0) is also a
// legitimate reading of some channel.

#pragma once

#include "CoreMinimal.h"

/**
 * Embodiment-relevant scalars describing measured competence and momentary performance state.
 * All channels are normalized to [0,1] unless noted. Each has a paired bXxxValid flag.
 */
struct FMasterySignal
{
    // ---- Sustained competence (changes over sessions) --------------------------------------

    /** Overall demonstrated competence, 0 = novice, 1 = mastery. */
    float Competence = 0.0f;
    bool bCompetenceValid = false;

    /** Discrete skill tier normalized to [0,1] (e.g. tier 7 of 10 -> 0.7). */
    float Tier = 0.0f;
    bool bTierValid = false;

    /** Breadth of competence across domains, 0 = single narrow skill, 1 = broadly capable. */
    float DomainBreadth = 0.0f;
    bool bDomainBreadthValid = false;

    // ---- Execution quality (changes over minutes) ------------------------------------------

    /** Cleanliness of recent execution: success rate, low error rate. */
    float ExecutionQuality = 0.0f;
    bool bExecutionQualityValid = false;

    /** Frame-timing precision: how close to optimal windows actions land. */
    float TimingPrecision = 0.0f;
    bool bTimingPrecisionValid = false;

    /** Reflex readiness derived from latency - 1 = near-instantaneous response. */
    float ReflexReadiness = 0.0f;
    bool bReflexReadinessValid = false;

    /** Fluency of multi-step sequence execution (combo chaining). */
    float ComboFlow = 0.0f;
    bool bComboFlowValid = false;

    /** Accuracy of anticipating an opponent or the environment. */
    float PredictionAccuracy = 0.0f;
    bool bPredictionAccuracyValid = false;

    // ---- Momentary state (changes over seconds) --------------------------------------------

    /** Flow intensity - absorbed, effortless engagement. */
    float FlowIntensity = 0.0f;
    bool bFlowIntensityValid = false;

    /** Challenge/skill balance. 0.5 = matched; <0.5 = bored (too easy); >0.5 = anxious (too hard). */
    float ChallengeSkillBalance = 0.5f;
    bool bChallengeSkillBalanceValid = false;

    /** Frustration / tilt. */
    float Frustration = 0.0f;
    bool bFrustrationValid = false;

    /** Self-assessed confidence. */
    float Confidence = 0.0f;
    bool bConfidenceValid = false;

    /** Physiological arousal / activation. */
    float Arousal = 0.0f;
    bool bArousalValid = false;

    // ---- Provenance -------------------------------------------------------------------------

    /** How many discrete observations back this signal. Low counts warrant hedged embodiment. */
    int32 EvidenceCount = 0;

    /** Seconds since the most recent evidence arrived. Stale signals should decay toward neutral. */
    float TimeSinceLastEvidence = 0.0f;

    // ---- Accessors --------------------------------------------------------------------------
    //
    // Always read through these. Each takes the value to assume when the channel was never
    // measured, forcing the caller to state their fallback explicitly at the point of use
    // rather than inheriting a silent zero.

    float GetCompetence(float IfUnknown) const { return bCompetenceValid ? Competence : IfUnknown; }
    float GetTier(float IfUnknown) const { return bTierValid ? Tier : IfUnknown; }
    float GetDomainBreadth(float IfUnknown) const { return bDomainBreadthValid ? DomainBreadth : IfUnknown; }
    float GetExecutionQuality(float IfUnknown) const { return bExecutionQualityValid ? ExecutionQuality : IfUnknown; }
    float GetTimingPrecision(float IfUnknown) const { return bTimingPrecisionValid ? TimingPrecision : IfUnknown; }
    float GetReflexReadiness(float IfUnknown) const { return bReflexReadinessValid ? ReflexReadiness : IfUnknown; }
    float GetComboFlow(float IfUnknown) const { return bComboFlowValid ? ComboFlow : IfUnknown; }
    float GetPredictionAccuracy(float IfUnknown) const { return bPredictionAccuracyValid ? PredictionAccuracy : IfUnknown; }
    float GetFlowIntensity(float IfUnknown) const { return bFlowIntensityValid ? FlowIntensity : IfUnknown; }
    float GetChallengeSkillBalance(float IfUnknown) const { return bChallengeSkillBalanceValid ? ChallengeSkillBalance : IfUnknown; }
    float GetFrustration(float IfUnknown) const { return bFrustrationValid ? Frustration : IfUnknown; }
    float GetConfidence(float IfUnknown) const { return bConfidenceValid ? Confidence : IfUnknown; }
    float GetArousal(float IfUnknown) const { return bArousalValid ? Arousal : IfUnknown; }

    /** Number of channels actually measured (0-13). */
    int32 NumValidChannels() const
    {
        return (bCompetenceValid ? 1 : 0) + (bTierValid ? 1 : 0) + (bDomainBreadthValid ? 1 : 0) +
               (bExecutionQualityValid ? 1 : 0) + (bTimingPrecisionValid ? 1 : 0) +
               (bReflexReadinessValid ? 1 : 0) + (bComboFlowValid ? 1 : 0) +
               (bPredictionAccuracyValid ? 1 : 0) + (bFlowIntensityValid ? 1 : 0) +
               (bChallengeSkillBalanceValid ? 1 : 0) + (bFrustrationValid ? 1 : 0) +
               (bConfidenceValid ? 1 : 0) + (bArousalValid ? 1 : 0);
    }

    /** True when nothing at all has been measured. */
    bool IsWhollyUnknown() const { return NumValidChannels() == 0; }

    /**
     * Evidence-weighted confidence in the signal itself, [0,1]. Distinct from the Confidence
     * channel: this is how much the SYSTEM should trust the reading, not how confident the
     * performer feels. Drives how far embodiment is allowed to depart from neutral.
     */
    float GetSignalTrust() const
    {
        const int32 Valid = NumValidChannels();
        if (Valid == 0 || EvidenceCount <= 0)
        {
            return 0.0f;
        }

        // Coverage: fraction of channels measured.
        const float Coverage = static_cast<float>(Valid) / 13.0f;

        // Evidence: saturating, so a handful of samples is not treated as proof.
        const float Evidence = static_cast<float>(EvidenceCount) / (static_cast<float>(EvidenceCount) + 20.0f);

        // Staleness: full trust for 5s, decaying to zero by 60s.
        float Freshness = 1.0f;
        if (TimeSinceLastEvidence > 5.0f)
        {
            Freshness = 1.0f - ((TimeSinceLastEvidence - 5.0f) / 55.0f);
            Freshness = Freshness < 0.0f ? 0.0f : (Freshness > 1.0f ? 1.0f : Freshness);
        }

        return Coverage * Evidence * Freshness;
    }
};
