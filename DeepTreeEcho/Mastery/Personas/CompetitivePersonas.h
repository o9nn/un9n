// CompetitivePersonas.h
// Aion and Toga as temperament data, plus the fused competitive Melody.
//
// Same rule as MelodyPersona.h: a persona is a FMasteryPersonaProfile literal, never a class.
// Temperament changes how competence READS. It never grants competence. A persona blend cannot
// make anyone better at the game - it only changes what winning looks like on them.

#pragma once

#include "../MasteryPersonaProfile.h"

namespace CompetitivePersonas
{
    /**
     * Aion - chaotic genius. Reads as unbothered and faintly amused regardless of scoreline,
     * because from a wide enough vantage the scoreline is not the interesting part. Very high
     * playfulness, near-total composure, deliberately low legibility of effort: you cannot tell
     * from watching whether the position is won or lost.
     */
    inline FMasteryPersonaProfile Aion()
    {
        FMasteryPersonaProfile P;
        P.PersonaName = TEXT("Aion");
        P.ExpressivenessGain = 1.10f;
        P.BaselineWarmth = 0.15f;
        P.TensionBias = -0.45f;          // strain barely registers
        P.FidgetBias = 0.20f;            // restless when under-challenged
        P.PlayfulnessBias = 0.95f;
        P.ConfidenceDisplayStyle = 0.60f;
        P.Composure = 0.92f;
        P.FlowLegibility = 0.35f;        // absorption is hard to read from outside
        P.BaseBlinkRate = 14.0f;
        P.BaseBreathRate = 12.0f;
        P.BasePostureUprightness = 0.55f;
        P.UnknownStateIntensityCap = 0.30f;
        return P;
    }

    /**
     * Toga - predatory focus. Intensity rather than calm: delight sharpens rather than softens
     * under pressure. High expressiveness, low composure (the affect SHOWS), and the highest
     * flow legibility of the three - when she locks on, everyone can tell.
     */
    inline FMasteryPersonaProfile Toga()
    {
        FMasteryPersonaProfile P;
        P.PersonaName = TEXT("Toga");
        P.ExpressivenessGain = 1.45f;
        P.BaselineWarmth = 0.30f;
        P.TensionBias = 0.25f;           // coiled, not relaxed
        P.FidgetBias = 0.25f;
        P.PlayfulnessBias = 0.80f;
        P.ConfidenceDisplayStyle = 0.88f;
        P.Composure = 0.35f;             // feeling reaches the surface fast
        P.FlowLegibility = 0.95f;
        P.BaseBlinkRate = 18.0f;
        P.BaseBreathRate = 16.0f;
        P.BasePostureUprightness = 0.70f;
        P.UnknownStateIntensityCap = 0.35f;
        return P;
    }

    /**
     * Weighted blend of persona temperaments.
     *
     * Blends the CONTINUOUS temperament fields only. Weights are normalized; a zero-weight or
     * empty input yields the default profile rather than a degenerate one.
     *
     * Note UnknownStateIntensityCap takes the MINIMUM, not the blend: the "unknown is not
     * excellence" ceiling must be the most conservative of its parents. Blending toward a more
     * expressive persona must never RAISE how much an unmeasured avatar may emote - otherwise
     * persona fusion becomes a laundering route around the invariant.
     */
    inline FMasteryPersonaProfile Blend(const TArray<FMasteryPersonaProfile>& Profiles,
                                        const TArray<float>& Weights,
                                        const FString& BlendName)
    {
        FMasteryPersonaProfile Out;
        Out.PersonaName = BlendName;

        if (Profiles.Num() == 0 || Profiles.Num() != Weights.Num())
        {
            return Out;
        }

        float Total = 0.0f;
        for (float W : Weights)
        {
            Total += (W > 0.0f) ? W : 0.0f;
        }
        if (Total <= 0.0f)
        {
            return Out;
        }

        Out.ExpressivenessGain = 0.0f;
        Out.BaselineWarmth = 0.0f;
        Out.TensionBias = 0.0f;
        Out.FidgetBias = 0.0f;
        Out.PlayfulnessBias = 0.0f;
        Out.ConfidenceDisplayStyle = 0.0f;
        Out.Composure = 0.0f;
        Out.FlowLegibility = 0.0f;
        Out.BaseBlinkRate = 0.0f;
        Out.BaseBreathRate = 0.0f;
        Out.BasePostureUprightness = 0.0f;

        float MinCap = 1.0f;

        for (int32 i = 0; i < Profiles.Num(); ++i)
        {
            const float W = ((Weights[i] > 0.0f) ? Weights[i] : 0.0f) / Total;
            const FMasteryPersonaProfile& P = Profiles[i];

            Out.ExpressivenessGain      += P.ExpressivenessGain * W;
            Out.BaselineWarmth          += P.BaselineWarmth * W;
            Out.TensionBias             += P.TensionBias * W;
            Out.FidgetBias              += P.FidgetBias * W;
            Out.PlayfulnessBias         += P.PlayfulnessBias * W;
            Out.ConfidenceDisplayStyle  += P.ConfidenceDisplayStyle * W;
            Out.Composure               += P.Composure * W;
            Out.FlowLegibility          += P.FlowLegibility * W;
            Out.BaseBlinkRate           += P.BaseBlinkRate * W;
            Out.BaseBreathRate          += P.BaseBreathRate * W;
            Out.BasePostureUprightness  += P.BasePostureUprightness * W;

            if (P.UnknownStateIntensityCap < MinCap)
            {
                MinCap = P.UnknownStateIntensityCap;
            }
        }

        Out.UnknownStateIntensityCap = MinCap;
        return Out;
    }
}
