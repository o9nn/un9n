// MasteryBackendBody.h
// Realizes the MOTION-QUALITY channels of FMasteryEmbodimentPose - the ones both face backends
// computed and then dropped.
//
// WHY THIS IS A DIFFERENT KIND OF BACKEND
//
// The binding header's central claim is that expertise is legible in economy of motion: the expert
// does not hurry, does not overcorrect, does not fidget. None of those are poses. "Overcorrects" is
// a property of a trajectory, "fidgets" is a property of a event rate, "gaze darts" is a property
// of an interval. A backend that wrote MotionEconomy straight onto a curve would produce a slider,
// not a skill cue - so this file is a small DRIVER that owns time, and the channels become
// dynamics parameters:
//
//     MotionEconomy      -> damping ratio of every postural spring. 1 = critically damped: arrives
//                           and stops. 0 = underdamped: overshoots and rings - overcorrection,
//                           literally.
//     ReactionSharpness  -> onset latency (wind-up) and spring stiffness. Sharp = no wind-up, fast.
//     MicroMovementRate  -> amplitude of continuous low-frequency sway on spine and neck.
//     IdleFidget         -> Poisson rate of discrete weight shifts.
//     SaccadeRate        -> Poisson rate of ballistic eye jumps, scaled by gaze wander.
//     ShoulderTension    -> clavicle elevation (the one static channel of the five).
//
// GATING IS NOT SCALING - READ BEFORE CHANGING
//
// The face backends gate by MULTIPLYING by ExpressionIntensity, because for a face zero is
// neutral. For motion quality zero is NOT neutral: zero fidget, zero sway, perfect damping is the
// grandmaster's extreme. Multiplying by intensity would make an untrusted, unmeasured character
// stand with perfect expert stillness - "unknown is not excellence" violated at the rig, by the
// very gate meant to enforce it. So here intensity LERPS each channel toward its neutral default
// (FMasteryEmbodimentPose's own initializers): an unmeasured character moves like an ordinary
// person, neither master nor novice. StandaloneBodyBackend asserts this directly.
//
// DETERMINISM. All stochastic events come from a seeded xorshift, never from a global RNG, so a
// given seed and pose sequence always produce the same trajectory. That is what lets the harness
// compare a master and a novice on identical random draws and attribute every difference to the
// channels rather than to luck.
//
// REALIZATIONS. Two, from one rig-neutral FBodyMotionFrame:
//   - UE5 / MetaHuman body skeleton: pelvis, spine_01..05, clavicle_l/r, neck_01/02, and the
//     FACIAL_L_Eye / FACIAL_R_Eye joints. Bone names are the ones DNABodySchemaBinding.cpp maps and
//     the shipped DNA files contain - not inferred.
//   - Melody's Live2D rig: body and head angles, eyeball offsets, within the authored ranges
//     documented in MelodyLive2DBackend.h (body ~ +/-3 degrees). Those parameters are also written
//     by the face backend, so this layer is applied AFTER it as an additive offset - see
//     MelodyLive2DBackend::ApplyPose's Body argument.

#pragma once

#include "CoreMinimal.h"
#include "../MasteryEmbodimentPose.h"

/** Rig-neutral output of one driver step. Degrees unless noted. */
struct FBodyMotionFrame
{
    float SpinePitch = 0.0f;     // + = forward. Posture + lean + reaction + breath.
    float SpineRoll = 0.0f;      // side sway (micro-movement)
    float PelvisYaw = 0.0f;      // weight shift (fidget)
    float PelvisRoll = 0.0f;     // weight shift (fidget)
    float ClavicleRaise = 0.0f;  // shoulder elevation
    float NeckYaw = 0.0f;        // micro-movement
    float NeckPitch = 0.0f;      // micro-movement
    float EyeYaw = 0.0f;         // saccade offset, normalized [-1,1]
    float EyePitch = 0.0f;       // saccade offset, normalized [-1,1]

    // ---- Diagnostics: not realized by any rig ----------------------------------------------
    // The postural spring's position and the target it is chasing, without breath. Exposed so
    // a harness can measure overcorrection during live play, where cues arrive back to back and
    // "peak minus target" is no longer well defined.
    float PostureSpring = 0.0f;
    float PostureTarget = 0.0f;
};

/** Rig-facing sink for a skeletal body. Rotations are additive offsets over the animation pose. */
class IBodySkeletonSink
{
public:
    virtual ~IBodySkeletonSink() = default;
    virtual bool HasBone(const FString& Bone) const = 0;
    virtual void SetBoneRotationOffset(const FString& Bone, float Pitch, float Yaw, float Roll) = 0;
};

namespace MasteryBackendBody
{
    /** Motion-quality channels after trust gating. See header: lerp to neutral, never scale. */
    struct FGatedChannels
    {
        float Economy, Sharpness, Micro, Fidget, Saccade, Shoulder, GazeWander, Upright, Lean,
              BreathRate;
    };

    inline FGatedChannels Gate(const FMasteryEmbodimentPose& P)
    {
        const FMasteryEmbodimentPose N = FMasteryEmbodimentPose::Neutral();
        const float I = FMath::Clamp(P.ExpressionIntensity, 0.0f, 1.0f);
        auto G = [I](float Neutral, float V) { return FMath::Lerp(Neutral, V, I); };

        FGatedChannels C;
        C.Economy    = FMath::Clamp(G(N.MotionEconomy,     P.MotionEconomy),     0.0f, 1.0f);
        C.Sharpness  = FMath::Clamp(G(N.ReactionSharpness, P.ReactionSharpness), 0.0f, 1.0f);
        C.Micro      = FMath::Clamp(G(N.MicroMovementRate, P.MicroMovementRate), 0.0f, 1.0f);
        C.Fidget     = FMath::Clamp(G(N.IdleFidget,        P.IdleFidget),        0.0f, 1.0f);
        C.Saccade    = FMath::Clamp(G(N.SaccadeRate,       P.SaccadeRate),       0.0f, 1.0f);
        C.Shoulder   = FMath::Clamp(G(N.ShoulderTension,   P.ShoulderTension),   0.0f, 1.0f);
        C.GazeWander = FMath::Clamp(1.0f - G(N.GazeSteadiness, P.GazeSteadiness), 0.0f, 1.0f);
        C.Upright    = FMath::Clamp(G(N.PostureUprightness, P.PostureUprightness), 0.0f, 1.0f);
        C.Lean       = FMath::Clamp(G(N.PostureLean,       P.PostureLean),      -1.0f, 1.0f);
        C.BreathRate = FMath::Max(G(N.BreathRate, P.BreathRate), 0.0f);
        return C;
    }

    /** Damping ratio from economy. Below 1 overshoots; the novice floor rings visibly. */
    inline float DampingRatio(float Economy)    { return FMath::Lerp(0.30f, 1.0f, Economy); }
    /** Natural frequency (rad/s) from sharpness. */
    inline float NaturalFrequency(float Sharp)  { return FMath::Lerp(7.0f, 18.0f, Sharp); }
    /** Seconds between the cue and the body starting to move - the wind-up. */
    inline float OnsetLatency(float Sharp)      { return FMath::Lerp(0.28f, 0.04f, Sharp); }

    /** A second-order spring toward a target, semi-implicit Euler, substepped for stability. */
    struct FSpring
    {
        float X = 0.0f, V = 0.0f;
        void Step(float Target, float Omega, float Zeta, float Dt)
        {
            const int32 Sub = FMath::Max(1, FMath::FloorToInt(Dt / 0.004f) + 1);
            const float H = Dt / (float)Sub;
            for (int32 i = 0; i < Sub; ++i)
            {
                V += (Omega * Omega * (Target - X) - 2.0f * Zeta * Omega * V) * H;
                X += V * H;
            }
        }
    };

    class FDriver
    {
    public:
        explicit FDriver(uint32 Seed = 0x9E3779B9u) { Reset(Seed); }

        void Reset(uint32 Seed)
        {
            Rng = Seed ? Seed : 1u;
            Time = 0.0f; BreathPhase = 0.0f;
            Lean = FSpring(); Pelvis = FSpring(); Clavicle = FSpring();
            NumPending = 0; ReactionMag = 0.0f; ReactionHold = 0.0f;
            Fired = 0; Dropped = 0;
            FidgetSide = 1.0f; FidgetTarget = 0.0f;
            EyeYaw = EyePitch = 0.0f;
            for (float& Ph : Phase) Ph = Uniform() * 6.2831853f;
            Out = FBodyMotionFrame();
        }

        /**
         * Something happened that the body should answer - a read landing, a threat, a hit.
         * Magnitude in [-1,1]: positive presses in, negative pulls back. The reaction is where
         * economy and sharpness become visible at all; a character who is never asked to move
         * cannot show that she moves well.
         *
         * Each cue gets its OWN wind-up timer, like separate stimuli each perceived after their
         * own delay. An earlier version kept one timer and restarted it on every cue, so in a
         * duel - where an attack starting and the hit landing arrive six frames apart - a slow
         * character's wind-ups kept being reset, and cues arriving faster than her latency were
         * never answered at all. The single-cue tests could not see that; StandaloneDuelBody did.
         */
        void React(float Magnitude, const FMasteryEmbodimentPose& Pose)
        {
            const FGatedChannels C = Gate(Pose);
            if (NumPending == MaxPending)
            {
                // Full: the oldest cue is the one the body has had longest to answer and has not.
                for (int32 i = 1; i < NumPending; ++i) { Pending[i - 1] = Pending[i]; }
                --NumPending;
                ++Dropped;
            }
            Pending[NumPending].Timer = OnsetLatency(C.Sharpness);
            Pending[NumPending].Mag = FMath::Clamp(Magnitude, -1.0f, 1.0f);
            ++NumPending;
        }

        /** Cues whose wind-up has elapsed and which the body has begun to answer. */
        int32 ReactionsFired() const { return Fired; }
        /** Cues discarded because more than MaxPending were winding up at once. */
        int32 ReactionsDropped() const { return Dropped; }

        const FBodyMotionFrame& Tick(const FMasteryEmbodimentPose& Pose, float Dt)
        {
            const FGatedChannels C = Gate(Pose);
            const float Zeta = DampingRatio(C.Economy);
            const float Omega = NaturalFrequency(C.Sharpness);
            Time += Dt;

            // ---- Reaction: wind-up, then a held offset the spring has to reach and settle on --
            // Timers are equal for equal poses, so pending cues fire in the order they arrived. A
            // later cue that fires replaces the held offset: the body answers what it most
            // recently perceived.
            int32 Kept = 0;
            for (int32 i = 0; i < NumPending; ++i)
            {
                Pending[i].Timer -= Dt;
                if (Pending[i].Timer < 0.0f)
                {
                    ReactionMag = Pending[i].Mag;
                    ReactionHold = 0.45f;
                    ++Fired;
                }
                else
                {
                    Pending[Kept++] = Pending[i];
                }
            }
            NumPending = Kept;
            float ReactionOffset = 0.0f;
            if (ReactionHold > 0.0f) { ReactionHold -= Dt; ReactionOffset = ReactionMag * 10.0f; }

            // ---- Posture: slouch, lean, reaction, all through ONE spring so economy shapes all
            const float PostureTarget = (0.5f - C.Upright) * 12.0f + C.Lean * 8.0f + ReactionOffset;
            Lean.Step(PostureTarget, Omega, Zeta, Dt);

            // ---- Breath ----------------------------------------------------------------------
            BreathPhase += Dt * C.BreathRate / 60.0f;
            const float Breath = FMath::Sin(BreathPhase * 6.2831853f) * 0.6f;

            // ---- Fidget: Poisson weight shifts, eased by the same (economy-damped) spring -----
            const float FidgetPerSec = C.Fidget * 8.0f / 60.0f;   // up to 8 shifts a minute
            if (Uniform() < FidgetPerSec * Dt)
            {
                FidgetSide = -FidgetSide;
                FidgetTarget = FidgetSide * (1.5f + 2.5f * Uniform());
            }
            Pelvis.Step(FidgetTarget, Omega * 0.6f, Zeta, Dt);

            // ---- Shoulders: static bracing, still arriving through the spring ----------------
            Clavicle.Step(C.Shoulder * 8.0f, Omega, Zeta, Dt);

            // ---- Micro-movement: incommensurate sines, so it never visibly loops ---------------
            const float Amp = FMath::Lerp(0.05f, 1.6f, C.Micro);
            const float SwayA = FMath::Sin(Time * 0.73f * 6.2831853f + Phase[0]);
            const float SwayB = FMath::Sin(Time * 0.41f * 6.2831853f + Phase[1]);
            const float SwayC = FMath::Sin(Time * 1.13f * 6.2831853f + Phase[2]);

            // ---- Saccades: ballistic, so they jump rather than spring -----------------------
            const float SaccPerSec = FMath::Lerp(0.3f, 4.0f, C.Saccade);
            if (Uniform() < SaccPerSec * Dt)
            {
                const float Reach = 0.05f + 0.45f * C.GazeWander;
                EyeYaw = (Uniform() * 2.0f - 1.0f) * Reach;
                EyePitch = (Uniform() * 2.0f - 1.0f) * Reach * 0.5f;
            }

            Out.SpinePitch = Lean.X + Breath;
            Out.SpineRoll = Amp * 0.6f * SwayA;
            Out.PelvisYaw = Pelvis.X;
            Out.PelvisRoll = Pelvis.X * 0.4f;
            Out.ClavicleRaise = Clavicle.X;
            Out.NeckYaw = Amp * SwayB;
            Out.NeckPitch = Amp * 0.5f * SwayC;
            Out.EyeYaw = EyeYaw;
            Out.EyePitch = EyePitch;
            Out.PostureSpring = Lean.X;
            Out.PostureTarget = PostureTarget;
            return Out;
        }

        const FBodyMotionFrame& Current() const { return Out; }

    private:
        float Uniform()
        {
            Rng ^= Rng << 13; Rng ^= Rng >> 17; Rng ^= Rng << 5;
            return (float)(Rng & 0xFFFFFFu) / (float)0x1000000u;
        }

        uint32 Rng = 1u;
        float Time = 0.0f, BreathPhase = 0.0f;
        FSpring Lean, Pelvis, Clavicle;
        struct FPendingCue { float Timer = 0.0f; float Mag = 0.0f; };
        static constexpr int32 MaxPending = 8;
        FPendingCue Pending[MaxPending];
        int32 NumPending = 0, Fired = 0, Dropped = 0;
        float ReactionMag = 0.0f, ReactionHold = 0.0f;
        float FidgetSide = 1.0f, FidgetTarget = 0.0f;
        float EyeYaw = 0.0f, EyePitch = 0.0f;
        float Phase[3] = {0.0f, 0.0f, 0.0f};
        FBodyMotionFrame Out;
    };

    // ---- Realization: UE5 / MetaHuman skeleton ------------------------------------------------

    inline void ApplyBone(IBodySkeletonSink& Rig, const TCHAR* Bone, float P, float Y, float R)
    {
        if (Rig.HasBone(Bone)) { Rig.SetBoneRotationOffset(Bone, P, Y, R); }
    }

    /** Degrees of eye rotation at a normalized saccade of 1. */
    constexpr float EyeRangeDegrees = 20.0f;

    inline void ApplyToSkeleton(IBodySkeletonSink& Rig, const FBodyMotionFrame& F)
    {
        // Spine flexion is distributed up the chain, weighted toward the chest, so the body
        // bends as a column rather than hinging at one vertebra.
        static const TCHAR* Spine[5] = { TEXT("spine_01"), TEXT("spine_02"), TEXT("spine_03"),
                                         TEXT("spine_04"), TEXT("spine_05") };
        static const float W[5] = { 0.10f, 0.15f, 0.20f, 0.25f, 0.30f };
        for (int32 i = 0; i < 5; ++i)
        {
            ApplyBone(Rig, Spine[i], F.SpinePitch * W[i], 0.0f, F.SpineRoll * W[i]);
        }
        ApplyBone(Rig, TEXT("pelvis"), 0.0f, F.PelvisYaw, F.PelvisRoll);
        // Mirrored roll: both shoulders rise.
        ApplyBone(Rig, TEXT("clavicle_l"), 0.0f, 0.0f,  F.ClavicleRaise);
        ApplyBone(Rig, TEXT("clavicle_r"), 0.0f, 0.0f, -F.ClavicleRaise);
        ApplyBone(Rig, TEXT("neck_01"), F.NeckPitch * 0.5f, F.NeckYaw * 0.5f, 0.0f);
        ApplyBone(Rig, TEXT("neck_02"), F.NeckPitch * 0.5f, F.NeckYaw * 0.5f, 0.0f);
        ApplyBone(Rig, TEXT("FACIAL_L_Eye"), F.EyePitch * EyeRangeDegrees, F.EyeYaw * EyeRangeDegrees, 0.0f);
        ApplyBone(Rig, TEXT("FACIAL_R_Eye"), F.EyePitch * EyeRangeDegrees, F.EyeYaw * EyeRangeDegrees, 0.0f);
    }

    // ---- Realization: Live2D offsets ----------------------------------------------------------
    //
    // Returned rather than written, because every one of these parameters is also driven by the
    // face backend; writing both would make the last writer win. MelodyLive2DBackend adds them.

    struct FLive2DBodyOffsets
    {
        float BodyAngleX = 0.0f, BodyAngleY = 0.0f, BodyAngleZ = 0.0f;
        float AngleX = 0.0f, AngleY = 0.0f;
        float EyeBallX = 0.0f, EyeBallY = 0.0f;
    };

    inline FLive2DBodyOffsets ToLive2D(const FBodyMotionFrame& F)
    {
        // Authored body range is about +/-3 degrees, a quarter of a skeleton's comfortable range;
        // compress rather than clip so the dynamics (overshoot, ringing) survive the mapping.
        FLive2DBodyOffsets O;
        O.BodyAngleX = FMath::Clamp(F.PelvisYaw * 0.6f, -3.0f, 3.0f);
        O.BodyAngleY = FMath::Clamp(F.SpinePitch * 0.25f + F.ClavicleRaise * 0.15f, -3.0f, 3.0f);
        O.BodyAngleZ = FMath::Clamp((F.SpineRoll + F.PelvisRoll) * 0.6f, -3.0f, 3.0f);
        O.AngleX = FMath::Clamp(F.NeckYaw, -3.0f, 3.0f);
        O.AngleY = FMath::Clamp(F.NeckPitch, -2.0f, 2.0f);
        O.EyeBallX = FMath::Clamp(F.EyeYaw, -1.0f, 1.0f);
        O.EyeBallY = FMath::Clamp(F.EyePitch, -1.0f, 1.0f);
        return O;
    }
}
