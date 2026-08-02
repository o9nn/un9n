// MelodyPersona.h
// Melody, as data only.
//
// No class of her own, no Live2D asset reference, no .moc3, no texture path. She is a
// FMasteryPersonaProfile literal and nothing more. When cogpy/live2d-mel becomes reachable, her
// rig attaches at the BACKEND layer (MasteryAvatarBackend) - this file does not change, because
// nothing here knows or cares what renders her.
//
// Temperament brief: a master gamer girl. Genuinely high skill worn lightly. Her competence
// reads as ease rather than intensity - she is demonstrative rather than stony (high
// ConfidenceDisplayStyle), keeps her composure when the game outpaces her (high Composure), and
// her flow is visible to an audience (high FlowLegibility), which matters for a character
// intended to be watched while playing.
//
// Every number below is temperament, not skill. Melody at Competence 0.1 will look like a
// cheerful novice; the profile does not grant her competence she has not demonstrated.

#pragma once

#include "../MasteryPersonaProfile.h"

namespace MelodyPersona
{
    inline FMasteryPersonaProfile Profile()
    {
        FMasteryPersonaProfile P;
        P.PersonaName = TEXT("Melody");

        // Animated and readable - she is meant to be watched.
        P.ExpressivenessGain = 1.25f;
        P.BaselineWarmth = 0.22f;

        // Carries load lightly: low visible tension, but not inhuman.
        P.TensionBias = -0.15f;

        // Some idle energy - she is playful, not statuesque.
        P.FidgetBias = 0.10f;
        P.PlayfulnessBias = 0.55f;

        // Mastery shows as visible ease and enjoyment rather than blank stillness.
        P.ConfidenceDisplayStyle = 0.72f;

        // Holds together when outmatched; strain surfaces, but late.
        P.Composure = 0.70f;

        // Her absorption is legible to an audience.
        P.FlowLegibility = 0.80f;

        P.BaseBlinkRate = 16.0f;
        P.BaseBreathRate = 14.0f;
        P.BasePostureUprightness = 0.62f;

        // Even unmeasured, she may look alive - but not accomplished.
        P.UnknownStateIntensityCap = 0.30f;

        return P;
    }
}
