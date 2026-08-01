// MelodyLive2DBackend.h
// Maps a backend-neutral FMasteryEmbodimentPose onto Melody's Live2D Cubism rig.
//
// This is the ONLY file in the Mastery module that knows a rig-specific parameter name. That is
// the whole point of FMasteryEmbodimentPose: the binding rules upstream never learn what a
// "ParamEyeLOpen" is, so swapping Melody for a MetaHuman means writing a sibling of this file
// and changing nothing else.
//
// ------------------------------------------------------------------------------------------
// CONFIRMED vs ASSUMED PARAMETERS - read before extending.
//
// Melody's model binary (miara_pro_t03.moc3) is served from external storage and is NOT in the
// live2d-mel repository, so her full parameter list could not be enumerated. Only what
// melody.model3.json explicitly declares is confirmed:
//
//   CONFIRMED (from melody.model3.json Groups):
//     ParamEyeLOpen, ParamEyeROpen   - EyeBlink group
//     ParamMouthOpenY                - LipSync group
//   CONFIRMED (from FileReferences.Motions):
//     motion groups "Idle", "Tap", "Flick"; a physics3 asset is present
//
//   ASSUMED (standard Cubism naming convention, NOT verified against her rig):
//     ParamAngleX/Y/Z, ParamBodyAngleX/Y/Z, ParamBrowLY/RY, ParamEyeBallX/Y,
//     ParamMouthForm, ParamBreath
//
// Every assumed parameter is written through ApplyOptional(), which no-ops when the rig does not
// declare the ID. A missing parameter therefore degrades that one channel silently instead of
// erroring or, worse, writing to the wrong parameter. When the .moc3 becomes available, verify
// the assumed list and promote what exists.
// ------------------------------------------------------------------------------------------

#pragma once

#include "CoreMinimal.h"
#include "../MasteryEmbodimentPose.h"

/**
 * Rig-facing sink. Implemented against whatever Live2D integration is actually linked
 * (un9n's UnrealEcho/Live2DCubism is NOT currently a linkable module - see Mastery/README.md),
 * or against a test double. Keeping this abstract is what lets the mapping below be unit-tested
 * without a rig present.
 */
class IMelodyRigSink
{
public:
    virtual ~IMelodyRigSink() = default;

    /** True if the loaded model declares this parameter ID. */
    virtual bool HasParameter(const FString& ParamId) const = 0;

    /** Set a normalized parameter value. Only called after HasParameter() returns true. */
    virtual void SetParameter(const FString& ParamId, float Value) = 0;

    /** Request a motion group by name (e.g. "Idle", "Tap", "Flick"). */
    virtual void PlayMotion(const FString& MotionGroup) {}
};

namespace MelodyLive2DBackend
{
    // ---- Confirmed parameter IDs -------------------------------------------------------------
    inline const TCHAR* ParamEyeLOpen()  { return TEXT("ParamEyeLOpen"); }
    inline const TCHAR* ParamEyeROpen()  { return TEXT("ParamEyeROpen"); }
    inline const TCHAR* ParamMouthOpenY(){ return TEXT("ParamMouthOpenY"); }

    /** Write only if the rig declares the ID. Used for every ASSUMED parameter. */
    inline void ApplyOptional(IMelodyRigSink& Rig, const FString& ParamId, float Value)
    {
        if (Rig.HasParameter(ParamId))
        {
            Rig.SetParameter(ParamId, Value);
        }
    }

    /**
     * Apply a pose to Melody's rig.
     *
     * Intensity scaling: every channel is attenuated by Pose.ExpressionIntensity, which upstream
     * has already capped by signal trust. This is how "unknown is not excellence" reaches the
     * rig - an unmeasured Melody physically cannot pull a confident expression, because the
     * amplitude is not available to her.
     *
     * Asymmetry is applied by biasing the left/right eye pair, since genuine affect is asymmetric
     * and a perfectly mirrored face reads as synthetic.
     */
    inline void ApplyPose(IMelodyRigSink& Rig, const FMasteryEmbodimentPose& Pose)
    {
        const float I = FMath::Clamp(Pose.ExpressionIntensity, 0.0f, 1.0f);

        // ---- Eyes (CONFIRMED) ----------------------------------------------------------------
        // Openness falls with narrowing (focus) and rises with widening (startle). Cubism
        // convention: 1 = fully open, 0 = closed.
        const float Openness = FMath::Clamp(1.0f - I * (0.6f * Pose.EyeNarrow) + I * (0.3f * Pose.EyeWiden),
                                            0.0f, 1.0f);
        const float AsymOffset = I * Pose.Asymmetry * 0.12f;
        Rig.SetParameter(ParamEyeLOpen(), FMath::Clamp(Openness + AsymOffset, 0.0f, 1.0f));
        Rig.SetParameter(ParamEyeROpen(), FMath::Clamp(Openness - AsymOffset, 0.0f, 1.0f));

        // ---- Mouth (CONFIRMED) ---------------------------------------------------------------
        // ParamMouthOpenY is the rig's LipSync channel. Drive only the non-speech component here;
        // a speech system owns it when talking, so tension/ease contribute a small resting offset
        // rather than fighting lip-sync for the channel.
        const float MouthOpen = FMath::Clamp(I * (0.15f * Pose.MouthCornerUp - 0.10f * Pose.MouthTension),
                                             0.0f, 1.0f);
        Rig.SetParameter(ParamMouthOpenY(), MouthOpen);

        // ---- Face (ASSUMED - guarded) --------------------------------------------------------
        ApplyOptional(Rig, TEXT("ParamMouthForm"),
                      FMath::Clamp(I * (Pose.MouthCornerUp - Pose.MouthCornerDown), -1.0f, 1.0f));
        ApplyOptional(Rig, TEXT("ParamBrowLY"),
                      FMath::Clamp(I * (Pose.BrowRaise - Pose.BrowFurrow) + AsymOffset, -1.0f, 1.0f));
        ApplyOptional(Rig, TEXT("ParamBrowRY"),
                      FMath::Clamp(I * (Pose.BrowRaise - Pose.BrowFurrow) - AsymOffset, -1.0f, 1.0f));

        // ---- Gaze (ASSUMED - guarded) --------------------------------------------------------
        // Steady gaze means the eyeballs stop wandering. Drive toward centre as steadiness rises;
        // an actual look-at target, when one exists, should be composited over this.
        const float GazeWander = FMath::Clamp(1.0f - Pose.GazeSteadiness, 0.0f, 1.0f);
        ApplyOptional(Rig, TEXT("ParamEyeBallX"), I * GazeWander * 0.4f);
        ApplyOptional(Rig, TEXT("ParamEyeBallY"), I * GazeWander * 0.2f);

        // ---- Head and body (ASSUMED - guarded) -----------------------------------------------
        ApplyOptional(Rig, TEXT("ParamAngleZ"), FMath::Clamp(I * Pose.HeadTilt * 30.0f, -30.0f, 30.0f));
        ApplyOptional(Rig, TEXT("ParamAngleX"), FMath::Clamp(I * Pose.PostureLean * 15.0f, -30.0f, 30.0f));

        // Uprightness maps to body pitch: slumping reads as negative Y.
        ApplyOptional(Rig, TEXT("ParamBodyAngleY"),
                      FMath::Clamp((Pose.PostureUprightness - 0.5f) * 20.0f, -10.0f, 10.0f));
        ApplyOptional(Rig, TEXT("ParamBodyAngleX"),
                      FMath::Clamp(I * Pose.PostureLean * 10.0f, -10.0f, 10.0f));

        // ---- Breath (ASSUMED - guarded) ------------------------------------------------------
        // Cubism's ParamBreath is a 0..1 oscillator driven by the host; expose rate as amplitude
        // so faster breathing reads as more visible chest movement.
        ApplyOptional(Rig, TEXT("ParamBreath"),
                      FMath::Clamp(Pose.BreathRate / 30.0f, 0.0f, 1.0f));
    }

    /**
     * Choose a motion group for the current pose. Melody's rig confirms exactly three:
     * Idle, Tap, Flick.
     *
     * Deliberately conservative - returns Idle unless the pose is decisively energetic, because
     * spuriously firing motions over a subtle expression is far more jarring than not firing one.
     */
    inline FString SelectMotionGroup(const FMasteryEmbodimentPose& Pose)
    {
        if (Pose.ExpressionIntensity > 0.75f && Pose.ReactionSharpness > 0.8f)
        {
            return TEXT("Flick");   // sharp, decisive - a read landing
        }
        if (Pose.ExpressionIntensity > 0.6f && Pose.MouthCornerUp > 0.6f)
        {
            return TEXT("Tap");     // pleased acknowledgement
        }
        return TEXT("Idle");
    }
}
