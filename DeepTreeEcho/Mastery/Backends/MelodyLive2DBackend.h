// MelodyLive2DBackend.h
// Maps a backend-neutral FMasteryEmbodimentPose onto Melody's Live2D Cubism rig.
//
// This is the ONLY file in the Mastery module that knows a rig-specific parameter name. That is
// the whole point of FMasteryEmbodimentPose: the binding rules upstream never learn what a
// "ParamEyeLOpen" is, so swapping Melody for a MetaHuman means writing a sibling of this file
// and changing nothing else.
//
// ------------------------------------------------------------------------------------------
// PARAMETER SET AND RANGES - read before extending.
//
// Melody's model binary (miara_pro_t03.moc3) is served from external storage, so the rig could
// not be enumerated directly. The parameter set below is instead established from
// melody.model3.json Groups plus 13 authored .exp3.json expression files
// (JOY_01/02/03/05, WONDER_02/03, SADNESS_01, SPEAK_01, PHOTO_Awe/UpwardGaze/ExuberantLaugh,
// NEUTRAL_Reset). The expression files settle both the parameter list AND, critically, the
// value RANGES an artist actually uses:
//
//   ParamEyeLOpen / ParamEyeROpen   0.15 .. 1.00   (1.0 = open; 0.15 = blissful closed)
//   ParamBrowLY   / ParamBrowRY    -0.10 .. 0.60   SIGNED
//   ParamBrowLAngle / RAngle       -0.30 .. 0.30   SIGNED - the oblique-brow axis
//   ParamMouthForm                 -0.40 .. 1.00   SIGNED - frown .. smile
//   ParamMouthOpenY                 0.00 .. 0.80
//   ParamAngleX                    -3 .. +3 degrees
//   ParamAngleY                    -2 .. +6 degrees
//   ParamAngleZ                    -5 .. +8 degrees
//   ParamBodyAngleX / Z            ~ -3 .. +3 degrees
//
// THE HEAD ANGLES ARE THE IMPORTANT CORRECTION. An earlier version of this file drove
// ParamAngleZ at +/-30 degrees and ParamAngleX at +/-15, inferred from "typical Cubism range".
// The authored expressions never exceed 8. Driving 30 where the artist uses 5 does not read as
// a bug - it reads as the character cranking her head over, which is worse, because it looks
// like a deliberate (bad) art choice rather than a miscalibration.
//
// Blend mode: every authored expression uses "Add", i.e. expressions are offsets layered over a
// base pose. This backend writes ABSOLUTE values instead, because it drives a continuous state
// rather than triggering discrete expressions. Both are valid; they must not be mixed on the
// same parameter in the same frame, so a host compositing authored .exp3 expressions on top of
// this must own the arbitration.
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
        // Authored range is 0.15 .. 1.00 - JOY_05 "Blissful" closes to 0.15, so narrowing must
        // be able to reach nearly shut. An earlier version bottomed out around 0.40 and could
        // not express a blissful or laughing squint at all.
        const float Openness = FMath::Clamp(1.0f - I * (0.85f * Pose.EyeNarrow) + I * (0.15f * Pose.EyeWiden),
                                            0.15f, 1.0f);
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

        // ---- Face -----------------------------------------------------------------------------
        // MouthForm is the signed smile/frown axis: authored range -0.40 (melancholy) .. 1.00
        // (laughing).
        ApplyOptional(Rig, TEXT("ParamMouthForm"),
                      FMath::Clamp(I * (Pose.MouthCornerUp - Pose.MouthCornerDown), -0.4f, 1.0f));

        // BrowY authored range is -0.10 .. 0.60, so raise is scaled to 0.6 rather than 1.0.
        const float BrowY = FMath::Clamp(I * (Pose.BrowRaise * 0.6f - Pose.BrowFurrow * 0.1f), -0.1f, 0.6f);
        ApplyOptional(Rig, TEXT("ParamBrowLY"), BrowY + AsymOffset);
        ApplyOptional(Rig, TEXT("ParamBrowRY"), BrowY - AsymOffset);

        // BrowAngle: the oblique-brow axis, and it cannot be derived from BrowY alone. In the
        // authored set it tracks brow raise EXCEPT for melancholy, which pairs a RAISED brow
        // (+0.15) with a NEGATIVE angle (-0.30) - the classic sad oblique brow, inner ends up
        // and outer ends down. So sadness has to pull it negative independently of raise, which
        // is what the MouthCornerDown term does here.
        const float BrowAngle = FMath::Clamp(
            I * (Pose.BrowRaise * 0.4f - Pose.MouthCornerDown * 0.5f - Pose.BrowFurrow * 0.2f),
            -0.3f, 0.3f);
        ApplyOptional(Rig, TEXT("ParamBrowLAngle"), BrowAngle);
        ApplyOptional(Rig, TEXT("ParamBrowRAngle"), BrowAngle);

        // ---- Gaze (ASSUMED - guarded) --------------------------------------------------------
        // Steady gaze means the eyeballs stop wandering. Drive toward centre as steadiness rises;
        // an actual look-at target, when one exists, should be composited over this.
        const float GazeWander = FMath::Clamp(1.0f - Pose.GazeSteadiness, 0.0f, 1.0f);
        ApplyOptional(Rig, TEXT("ParamEyeBallX"), I * GazeWander * 0.4f);
        ApplyOptional(Rig, TEXT("ParamEyeBallY"), I * GazeWander * 0.2f);

        // ---- Head and body ---------------------------------------------------------------------
        // Degrees, calibrated to the authored range (see header). These are SMALL - the whole
        // authored set stays within about 8 degrees.
        // Scaled to 8 so WONDER_02_CuriousGaze's authored +8 is reachable; a 6x scale clamped
        // at 8 could never actually get there.
        //
        // HeadTilt is now SIGNED, so both authored directions are reachable: positive is the
        // inquisitive tilt (WONDER_02 at +8), negative the mirthful one (JOY_02 at -4,
        // PHOTO_ExuberantLaugh at -5). The asymmetric clamp is the rig's, not a mistake - the
        // artist uses more range one way than the other.
        ApplyOptional(Rig, TEXT("ParamAngleZ"), FMath::Clamp(I * Pose.HeadTilt * 8.0f, -5.0f, 8.0f));
        ApplyOptional(Rig, TEXT("ParamAngleX"), FMath::Clamp(I * Pose.PostureLean * 3.0f, -3.0f, 3.0f));
        ApplyOptional(Rig, TEXT("ParamAngleY"),
                      FMath::Clamp(I * (1.0f - Pose.GazeSteadiness) * 4.0f, -2.0f, 6.0f));

        // Body angles are smaller still in the authored set (about +/-3).
        ApplyOptional(Rig, TEXT("ParamBodyAngleZ"),
                      FMath::Clamp((Pose.PostureUprightness - 0.5f) * 4.0f, -3.0f, 3.0f));
        ApplyOptional(Rig, TEXT("ParamBodyAngleX"),
                      FMath::Clamp(I * Pose.PostureLean * 2.0f, -2.0f, 2.0f));

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
