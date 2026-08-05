// MasteryBackendMetaHuman.h
// Drives a MetaHuman face from FMasteryEmbodimentPose.
//
// Sibling of MelodyLive2DBackend.h - same job, different rig vocabulary. Neither leaks upward:
// the binding rules that produce the pose have never heard of a control curve.
//
// ---------------------------------------------------------------------------------------------
// PROVENANCE OF THE CURVE NAMES
//
// Every curve name and decomposition weight below comes from the project's own
// `expression_catalog_facs_complete.md` (v4.0, "MH Standards Integration Map", section 6) - NOT
// from ARKit, and NOT from my inference. This matters because MetaHuman's native facial rig is
// its own control set; ARKit's 52 blendshapes are a separate interchange standard that has to be
// translated. Mixing the two vocabularies silently produces a face that does nothing.
//
// The catalog also supplies the decomposition weights, which are the part that would have been
// guessed wrong: an action unit is frequently NOT one curve.
//
//     brow_raise  = AU1 + AU2  -> browRaiseInner(v * 0.6) + browRaiseOuter(v * 0.4)
//     brow_furrow = AU4        -> browDown(v * 0.7)       + browLateral(v * 0.3)
//     eye_wide    = AU5        -> eyeUpperLidUp(v * 0.6)  + eyeWiden(v * 0.4)
//
// A backend that mapped brow_raise onto a single "browRaise" curve would be subtly wrong in a
// way that reads as "the face is stiff" rather than as an obvious bug.
// ---------------------------------------------------------------------------------------------

#pragma once

#include "CoreMinimal.h"
#include "../MasteryEmbodimentPose.h"

/**
 * Rig-facing sink, mirroring IMelodyRigSink. Implemented against a MetaHuman control rig, a
 * DNA-calibrated face component, or a test double.
 */
class IMetaHumanRigSink
{
public:
    virtual ~IMetaHumanRigSink() = default;

    /** True if the rig exposes this control curve. */
    virtual bool HasCurve(const FString& CurveName) const = 0;

    /** Set a control curve value, normally [0,1]. Only called after HasCurve() returns true. */
    virtual void SetCurve(const FString& CurveName, float Value) = 0;
};

namespace MasteryBackendMetaHuman
{
    /**
     * FACS intensity grade, per the catalog's scoring standard (section 3).
     * Exposed because it makes debugging legible: "brow_furrow at grade D" says more than 0.68.
     */
    enum class EFacsGrade : uint8 { Neutral, A_Trace, B_Slight, C_Marked, D_Severe, E_Maximum };

    inline EFacsGrade GradeOf(float V)
    {
        if (V <= 0.005f)  return EFacsGrade::Neutral;
        if (V <= 0.20f)   return EFacsGrade::A_Trace;
        if (V <= 0.40f)   return EFacsGrade::B_Slight;
        if (V <= 0.60f)   return EFacsGrade::C_Marked;
        if (V <= 0.80f)   return EFacsGrade::D_Severe;
        return EFacsGrade::E_Maximum;
    }

    inline const TCHAR* GradeName(EFacsGrade G)
    {
        switch (G)
        {
        case EFacsGrade::A_Trace:   return TEXT("A (trace)");
        case EFacsGrade::B_Slight:  return TEXT("B (slight)");
        case EFacsGrade::C_Marked:  return TEXT("C (marked)");
        case EFacsGrade::D_Severe:  return TEXT("D (severe)");
        case EFacsGrade::E_Maximum: return TEXT("E (maximum)");
        default:                    return TEXT("neutral");
        }
    }

    /** Write only if the rig exposes the curve, so a partial rig degrades one channel rather
     *  than erroring or writing to the wrong control. */
    inline void Apply(IMetaHumanRigSink& Rig, const FString& Curve, float Value)
    {
        if (Rig.HasCurve(Curve))
        {
            Rig.SetCurve(Curve, FMath::Clamp(Value, 0.0f, 1.0f));
        }
    }

    /**
     * Apply a left/right pair with an asymmetry bias.
     *
     * Laterality is a first-class concept in the catalog (section 4) precisely because genuine
     * affect is asymmetric; a perfectly mirrored face reads as synthetic. Bias is added to the
     * left and subtracted from the right, matching the catalog's "(asym)" modifier.
     */
    inline void ApplyLR(IMetaHumanRigSink& Rig, const TCHAR* BaseName, float Value, float AsymBias)
    {
        Apply(Rig, FString(BaseName) + TEXT("L"), Value + AsymBias);
        Apply(Rig, FString(BaseName) + TEXT("R"), Value - AsymBias);
    }

    /**
     * Push a pose onto a MetaHuman rig.
     *
     * Intensity gating: every curve is scaled by Pose.ExpressionIntensity, which upstream has
     * already capped by signal trust. That is how "unknown is not excellence" reaches the face -
     * an unmeasured character physically cannot pull a confident expression, because the
     * amplitude is not available to her.
     */
    inline void ApplyPose(IMetaHumanRigSink& Rig, const FMasteryEmbodimentPose& Pose)
    {
        const float I = FMath::Clamp(Pose.ExpressionIntensity, 0.0f, 1.0f);
        const float Asym = I * Pose.Asymmetry * 0.10f;

        // ---- AU1 + AU2 : inner + outer brow raise ------------------------------------------
        const float BrowRaise = I * Pose.BrowRaise;
        ApplyLR(Rig, TEXT("browRaiseInner"), BrowRaise * 0.6f, Asym);
        ApplyLR(Rig, TEXT("browRaiseOuter"), BrowRaise * 0.4f, Asym);

        // ---- AU4 : brow lowerer / furrow ----------------------------------------------------
        const float BrowFurrow = I * Pose.BrowFurrow;
        ApplyLR(Rig, TEXT("browDown"),    BrowFurrow * 0.7f, Asym * 0.5f);
        ApplyLR(Rig, TEXT("browLateral"), BrowFurrow * 0.3f, Asym * 0.5f);

        // ---- AU5 : upper lid raise ----------------------------------------------------------
        const float EyeWide = I * Pose.EyeWiden;
        ApplyLR(Rig, TEXT("eyeUpperLidUp"), EyeWide * 0.6f, 0.0f);
        ApplyLR(Rig, TEXT("eyeWiden"),      EyeWide * 0.4f, 0.0f);

        // ---- AU7 : lid tightener ------------------------------------------------------------
        ApplyLR(Rig, TEXT("eyeSquintInner"), I * Pose.EyeNarrow, 0.0f);

        // ---- AU12 : lip corner puller -------------------------------------------------------
        // Asymmetry lands hardest here - a faintly uneven smile is what reads as genuine.
        ApplyLR(Rig, TEXT("mouthCornerPull"), I * Pose.MouthCornerUp, Asym * 1.5f);

        // ---- AU6 : cheek raise ---------------------------------------------------------------
        // Not an independent pose channel. AU6 accompanying AU12 is what distinguishes a
        // Duchenne (felt) smile from a social one, so it is derived from the smile rather than
        // exposed separately - and scaled by ease, so a strained smile does not get the eyes.
        const float Duchenne = I * Pose.MouthCornerUp * (1.0f - Pose.MouthTension);
        ApplyLR(Rig, TEXT("eyeCheekRaise"), Duchenne * 0.7f, 0.0f);

        // ---- AU15 : lip corner depressor ----------------------------------------------------
        ApplyLR(Rig, TEXT("mouthCornerDepress"), I * Pose.MouthCornerDown, Asym * 0.5f);

        // ---- AU23 + AU24 : lip tightener + pressor ------------------------------------------
        const float MouthTension = I * Pose.MouthTension;
        ApplyLR(Rig, TEXT("mouthLipsTightenU"), MouthTension, 0.0f);
        ApplyLR(Rig, TEXT("mouthPressD"),       MouthTension * 0.8f, 0.0f);

        // ---- AU17 : chin raiser --------------------------------------------------------------
        // The catalog maps jaw clench most closely to AU17; there is no dedicated "jaw tension"
        // AU, and using jawOpen (AU26) for it would open the mouth, which is the opposite.
        ApplyLR(Rig, TEXT("jawChinRaiseD"), I * Pose.JawTension, 0.0f);

        // ---- AU43 : eye closure --------------------------------------------------------------
        // Blink RATE is a temporal property; the rig receives instantaneous closure. A blink
        // driver owns the timing and should composite over this - so only the sustained
        // narrowing that very low blink rates imply is written here.
        const float SustainedClosure = FMath::Clamp((10.0f - Pose.BlinkRate) / 20.0f, 0.0f, 0.25f);
        ApplyLR(Rig, TEXT("eyeBlink"), SustainedClosure, 0.0f);
    }

    /**
     * Head and eye orientation, kept separate from the face because it is driven by different
     * curves (and often by a look-at solver that should composite over, not fight, these).
     *
     * Corresponds to the catalog's M-codes (section 5): M55/M56 head tilt L/R, M57/M59 head
     * forward/back, M61/M62 eyes L/R, M68/M69 eyes up/down.
     *
     * Returned in degrees rather than written as curves, because head orientation on a MetaHuman
     * is normally a bone/control-rig transform rather than a named morph.
     */
    struct FHeadOrientation
    {
        float TiltDegrees = 0.0f;    // M55 / M56 - roll
        float PitchDegrees = 0.0f;   // M57 / M59 - forward / back
        float GazeYaw = 0.0f;        // M61 / M62 - eyes left / right, normalized [-1,1]
        float GazePitch = 0.0f;      // M68 / M69 - eyes up / down, normalized [-1,1]
    };

    inline FHeadOrientation ComputeHeadOrientation(const FMasteryEmbodimentPose& Pose)
    {
        FHeadOrientation O;
        const float I = FMath::Clamp(Pose.ExpressionIntensity, 0.0f, 1.0f);

        O.TiltDegrees = FMath::Clamp(I * Pose.HeadTilt * 25.0f, -25.0f, 25.0f);
        O.PitchDegrees = FMath::Clamp(Pose.PostureLean * 12.0f, -12.0f, 12.0f);

        // Low gaze steadiness means the eyes wander. Direction is not encoded in the pose - a
        // look-at target owns that - so only the MAGNITUDE of wander is supplied here.
        const float Wander = FMath::Clamp(1.0f - Pose.GazeSteadiness, 0.0f, 1.0f);
        O.GazeYaw = Wander * 0.5f;
        O.GazePitch = Wander * 0.25f;
        return O;
    }
}
