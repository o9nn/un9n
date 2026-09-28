// StandaloneBodyBackend.cpp
// Does the body backend make a master and a novice LOOK different - and for the right reasons?
//
// StandaloneChannelRealization used to pin "a master and a novice are rig-identical on motion
// economy alone". That was true: both backends were face-only. This harness is the other half of
// closing that gap. It does not stop at "the output changed" - a slider wired to a bone would pass
// that. It measures the properties the binding header actually claims read as skill:
//
//   - the expert does not OVERCORRECT        -> overshoot after a reaction
//   - the expert does not WIND UP            -> onset latency
//   - the expert does not FIDGET             -> weight-shift count
//   - the expert is STILL                    -> sway RMS
//   - the expert's gaze does not HUNT        -> saccade count
//
// And the invariant, which here has a trap in it: zero motion is the EXPERT extreme, so gating by
// multiplying with intensity (as the face backends correctly do) would make an unmeasured
// character look like a grandmaster. Checked explicitly.
//
// All comparisons share a seed, so every stochastic draw is identical between the two characters
// and any difference is attributable to the channels alone.
//
// Build & run:
//   g++ -std=c++17 -O2 -I StandaloneShim -o bodybackend StandaloneBodyBackend.cpp

#include <cmath>
#include <cstdio>
#include <set>
#include <string>
#include <vector>

#include "../MasteryEmbodimentBinding.cpp"      // real rules, single translation unit
#include "../Personas/MelodyPersona.h"
#include "../Backends/MasteryBackendBody.h"
#include "../Backends/MelodyLive2DBackend.h"

namespace
{

int TestsFailed = 0;

void Check(bool bCond, const std::string& Label)
{
    std::printf("  [%s] %s\n", bCond ? "PASS" : "FAIL", Label.c_str());
    if (!bCond) ++TestsFailed;
}

constexpr float Dt = 1.0f / 60.0f;
constexpr uint32 Seed = 0xC0FFEEu;

FMasteryEmbodimentPose Pose(float Economy, float Sharp, float Micro, float Fidget, float Saccade,
                            float Gaze = 0.5f, float Intensity = 1.0f)
{
    FMasteryEmbodimentPose P;
    P.MotionEconomy = Economy;
    P.ReactionSharpness = Sharp;
    P.MicroMovementRate = Micro;
    P.IdleFidget = Fidget;
    P.SaccadeRate = Saccade;
    P.GazeSteadiness = Gaze;
    P.ExpressionIntensity = Intensity;
    P.PostureUprightness = 0.5f;   // zero slouch target, so reactions start from rest
    P.BreathRate = 0.0f;           // breath off where it would pollute a measurement
    return P;
}

/** Response to one reaction: peak overshoot past the held target, and time to 10% of it. */
struct FReaction { float OvershootFrac; float OnsetSec; };

FReaction MeasureReaction(const FMasteryEmbodimentPose& P)
{
    MasteryBackendBody::FDriver D(Seed);
    // Micro-movement and fidget would add to spine pitch? They do not (roll/pelvis only), but
    // settle first anyway so the measurement starts at rest.
    for (int i = 0; i < 60; ++i) D.Tick(P, Dt);
    const float Rest = D.Current().SpinePitch;

    D.React(1.0f, P);
    const float Target = 10.0f;               // FDriver: ReactionMag * 10 degrees
    float Peak = 0.0f, Onset = -1.0f;
    // Longest wind-up (0.28 s) plus the 0.45 s hold. An earlier 0.45 s window ended before a
    // hesitant character had even arrived, so it could not observe her overshoot at all.
    for (int i = 0; i < 44; ++i)
    {
        D.Tick(P, Dt);
        const float X = D.Current().SpinePitch - Rest;
        Peak = std::max(Peak, X);
        if (Onset < 0.0f && X > 0.1f * Target) Onset = (i + 1) * Dt;
    }
    return { std::max(0.0f, Peak - Target) / Target, Onset < 0.0f ? 99.0f : Onset };
}

/** Run for Seconds; count fidget shifts (pelvis target flips) and saccades (eye jumps), and sway RMS. */
struct FIdleStats { int Fidgets; int Saccades; float SwayRms; float MeanEyeReach; };

FIdleStats MeasureIdle(const FMasteryEmbodimentPose& P, float Seconds)
{
    MasteryBackendBody::FDriver D(Seed);
    FIdleStats S{0, 0, 0.0f, 0.0f};
    float PrevEye = 0.0f, PrevVel = 0.0f, PrevPelvis = 0.0f;
    double SumSq = 0.0, Reach = 0.0;
    const int N = (int)(Seconds / Dt);
    for (int i = 0; i < N; ++i)
    {
        const FBodyMotionFrame& F = D.Tick(P, Dt);
        if (F.EyeYaw != PrevEye) { ++S.Saccades; Reach += std::fabs(F.EyeYaw); PrevEye = F.EyeYaw; }
        // A new fidget reverses the pelvis's direction of travel toward the other side.
        const float Vel = F.PelvisYaw - PrevPelvis;
        if (i > 1 && ((Vel > 1e-4f && PrevVel < -1e-4f) || (Vel < -1e-4f && PrevVel > 1e-4f))) ++S.Fidgets;
        if (std::fabs(Vel) > 1e-4f) PrevVel = Vel;
        PrevPelvis = F.PelvisYaw;
        SumSq += F.NeckYaw * F.NeckYaw + F.SpineRoll * F.SpineRoll;
    }
    S.SwayRms = (float)std::sqrt(SumSq / N);
    S.MeanEyeReach = S.Saccades ? (float)(Reach / S.Saccades) : 0.0f;
    return S;
}

/** Records every bone write. */
struct FSkeletonProbe : public IBodySkeletonSink
{
    std::set<std::string> Allowed;   // empty = accept everything
    std::vector<std::pair<std::string, float>> Out;
    bool HasBone(const FString& B) const override { return Allowed.empty() || Allowed.count(B.Str); }
    void SetBoneRotationOffset(const FString& B, float P, float Y, float R) override
    {
        Out.push_back({B.Str, P + 7.0f * Y + 13.0f * R});   // order-sensitive fingerprint
    }
};

std::vector<float> Trajectory(const FMasteryEmbodimentPose& P, float Seconds, bool bReact)
{
    MasteryBackendBody::FDriver D(Seed);
    std::vector<float> T;
    for (int i = 0; i < (int)(Seconds / Dt); ++i)
    {
        if (bReact && i == 30) D.React(1.0f, P);
        FSkeletonProbe Probe;
        MasteryBackendBody::ApplyToSkeleton(Probe, D.Tick(P, Dt));
        for (auto& W : Probe.Out) T.push_back(W.second);
    }
    return T;
}

float MaxAbsDiff(const std::vector<float>& A, const std::vector<float>& B)
{
    if (A.size() != B.size()) return 1e9f;
    float M = 0.0f;
    for (size_t i = 0; i < A.size(); ++i) M = std::max(M, std::fabs(A[i] - B[i]));
    return M;
}

FMasterySignal Measured(float Skill, float Frustration)
{
    FMasterySignal S;
    S.Competence = Skill;            S.bCompetenceValid = true;
    S.Tier = Skill;                  S.bTierValid = true;
    S.DomainBreadth = Skill;         S.bDomainBreadthValid = true;
    S.ExecutionQuality = Skill;      S.bExecutionQualityValid = true;
    S.TimingPrecision = Skill;       S.bTimingPrecisionValid = true;
    S.ReflexReadiness = Skill;       S.bReflexReadinessValid = true;
    S.ComboFlow = Skill;             S.bComboFlowValid = true;
    S.PredictionAccuracy = Skill;    S.bPredictionAccuracyValid = true;
    S.FlowIntensity = Skill;         S.bFlowIntensityValid = true;
    S.ChallengeSkillBalance = 0.5f;  S.bChallengeSkillBalanceValid = true;
    S.Frustration = Frustration;     S.bFrustrationValid = true;
    S.Confidence = Skill;            S.bConfidenceValid = true;
    S.Arousal = 0.5f;                S.bArousalValid = true;
    S.EvidenceCount = 400;
    S.TimeSinceLastEvidence = 0.5f;
    return S;
}

} // namespace

int main()
{
    std::printf("=== Body backend: does motion quality read as skill? ===\n\n");

    // ---- 1. Overcorrection ------------------------------------------------------------------
    std::printf("--- 1. overcorrection (MotionEconomy -> damping) ---\n");
    const FReaction RExpert = MeasureReaction(Pose(1.0f, 0.5f, 0.0f, 0.0f, 0.0f));
    const FReaction RNovice = MeasureReaction(Pose(0.0f, 0.5f, 0.0f, 0.0f, 0.0f));
    std::printf("  overshoot: economy 1 = %.1f%%   economy 0 = %.1f%%\n",
                RExpert.OvershootFrac * 100.0f, RNovice.OvershootFrac * 100.0f);
    Check(RExpert.OvershootFrac < 0.01f, "the expert arrives and stops - under 1% overshoot");
    Check(RNovice.OvershootFrac > 0.25f, "the novice overcorrects - over 25% overshoot");

    // ---- 2. Wind-up ---------------------------------------------------------------------------
    std::printf("\n--- 2. wind-up (ReactionSharpness -> onset latency) ---\n");
    const FReaction RSharp = MeasureReaction(Pose(1.0f, 1.0f, 0.0f, 0.0f, 0.0f));
    const FReaction RSlow  = MeasureReaction(Pose(1.0f, 0.0f, 0.0f, 0.0f, 0.0f));
    std::printf("  time to 10%%: sharp = %.0f ms   hesitant = %.0f ms\n",
                RSharp.OnsetSec * 1000.0f, RSlow.OnsetSec * 1000.0f);
    Check(RSharp.OnsetSec < 0.12f, "a sharp reaction is under way within 120 ms");
    Check(RSlow.OnsetSec > RSharp.OnsetSec + 0.2f, "a hesitant one winds up at least 200 ms longer");

    // ---- 3-5. Idle behaviour ------------------------------------------------------------------
    std::printf("\n--- 3-5. idle over 120 s (fidget, sway, saccades) ---\n");
    const FIdleStats Still = MeasureIdle(Pose(1.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f), 120.0f);
    const FIdleStats Restless = MeasureIdle(Pose(1.0f, 0.5f, 1.0f, 1.0f, 1.0f, 0.0f), 120.0f);
    std::printf("  %-9s fidgets=%3d  saccades=%3d  swayRMS=%.2f deg  eyeReach=%.2f\n",
                "still", Still.Fidgets, Still.Saccades, Still.SwayRms, Still.MeanEyeReach);
    std::printf("  %-9s fidgets=%3d  saccades=%3d  swayRMS=%.2f deg  eyeReach=%.2f\n",
                "restless", Restless.Fidgets, Restless.Saccades, Restless.SwayRms, Restless.MeanEyeReach);
    Check(Still.Fidgets == 0, "IdleFidget 0 produces no weight shifts at all");
    Check(Restless.Fidgets >= 8, "IdleFidget 1 produces a visible stream of weight shifts");
    Check(Restless.SwayRms > 10.0f * Still.SwayRms, "MicroMovementRate moves sway by over 10x");
    Check(Restless.Saccades > 5 * Still.Saccades, "SaccadeRate moves saccade count by over 5x");
    Check(Restless.MeanEyeReach > 3.0f * Still.MeanEyeReach,
          "a wandering gaze jumps further - saccade reach follows GazeSteadiness");

    // ---- 6. Shoulders -------------------------------------------------------------------------
    std::printf("\n--- 6. shoulder bracing ---\n");
    {
        FMasteryEmbodimentPose Braced = Pose(1.0f, 0.5f, 0.0f, 0.0f, 0.0f);
        Braced.ShoulderTension = 1.0f;
        MasteryBackendBody::FDriver D(Seed);
        for (int i = 0; i < 180; ++i) D.Tick(Braced, Dt);
        std::printf("  clavicle raise after 3 s at tension 1: %.2f deg\n", D.Current().ClavicleRaise);
        Check(std::fabs(D.Current().ClavicleRaise - 8.0f) < 0.05f, "ShoulderTension 1 settles at 8 degrees");
    }

    // ---- 7. Gating: unknown is not excellence, with the trap --------------------------------
    std::printf("\n--- 7. trust gating ---\n");
    {
        const FMasteryEmbodimentPose Expert0 = Pose(1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
        const FMasteryEmbodimentPose Novice0 = Pose(0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f);
        const FMasteryEmbodimentPose ExpertLive = Pose(1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f);
        const float D = MaxAbsDiff(Trajectory(Expert0, 20.0f, true), Trajectory(Novice0, 20.0f, true));
        std::printf("  expert vs novice at zero intensity: max diff %.2e\n", D);
        Check(D < 1e-5f, "at zero intensity the channels cannot distinguish expert from novice");

        const FIdleStats Unknown = MeasureIdle(Expert0, 120.0f);
        const FIdleStats Master = MeasureIdle(ExpertLive, 120.0f);
        const FReaction RUnknown = MeasureReaction(Expert0);
        std::printf("  unknown: fidgets=%d swayRMS=%.2f overshoot=%.1f%%  |  measured master: fidgets=%d swayRMS=%.2f\n",
                    Unknown.Fidgets, Unknown.SwayRms, RUnknown.OvershootFrac * 100.0f,
                    Master.Fidgets, Master.SwayRms);
        // The trap. If gating multiplied toward zero, the unknown character would be stiller
        // than the measured master.
        Check(Unknown.SwayRms > 5.0f * Master.SwayRms && Unknown.Fidgets > Master.Fidgets,
              "an UNMEASURED character is visibly less composed than a measured master");
        Check(RUnknown.OvershootFrac > 0.02f,
              "an unmeasured character does not get the expert's clean, overshoot-free arrival");
    }

    // ---- 8. Determinism -------------------------------------------------------------------------
    std::printf("\n--- 8. determinism ---\n");
    {
        const FMasteryEmbodimentPose P = Pose(0.3f, 0.6f, 0.7f, 0.8f, 0.6f, 0.4f);
        Check(MaxAbsDiff(Trajectory(P, 30.0f, true), Trajectory(P, 30.0f, true)) == 0.0f,
              "same seed and pose sequence give a bit-identical trajectory");
    }

    // ---- 9. Skeleton realization ----------------------------------------------------------------
    std::printf("\n--- 9. skeleton realization ---\n");
    {
        const std::set<std::string> Expected = {
            "pelvis", "spine_01", "spine_02", "spine_03", "spine_04", "spine_05",
            "clavicle_l", "clavicle_r", "neck_01", "neck_02", "FACIAL_L_Eye", "FACIAL_R_Eye" };
        FSkeletonProbe All;
        MasteryBackendBody::ApplyToSkeleton(All, FBodyMotionFrame());
        std::set<std::string> Written;
        for (auto& W : All.Out) Written.insert(W.first);
        Check(Written == Expected, "writes exactly the 12 known UE5/MetaHuman joints and nothing else");

        FSkeletonProbe Partial;
        Partial.Allowed = { "spine_03", "neck_01" };
        MasteryBackendBody::ApplyToSkeleton(Partial, FBodyMotionFrame());
        Check(Partial.Out.size() == 2, "a partial skeleton degrades to the bones it has, without error");
    }

    // ---- 10. Live2D realization -----------------------------------------------------------------
    std::printf("\n--- 10. Live2D realization (Melody) ---\n");
    {
        struct FL2D : public IMelodyRigSink
        {
            std::vector<std::pair<std::string, float>> Out;
            bool HasParameter(const FString&) const override { return true; }
            void SetParameter(const FString& Id, float V) override { Out.push_back({Id.Str, V}); }
        };

        const FMasteryEmbodimentPose Face = Pose(0.0f, 0.5f, 1.0f, 1.0f, 1.0f, 0.0f);
        FL2D Without, WithZero;
        MelodyLive2DBackend::ApplyPose(Without, Face);
        MasteryBackendBody::FLive2DBodyOffsets Zero;
        MelodyLive2DBackend::ApplyPose(WithZero, Face, &Zero);
        // WithZero additionally writes ParamBodyAngleY; everything shared must be identical.
        bool bSame = true;
        for (auto& A : Without.Out)
            for (auto& B : WithZero.Out)
                if (A.first == B.first && std::fabs(A.second - B.second) > 1e-6f) bSame = false;
        Check(bSame, "a zero body layer leaves every face-written parameter exactly as it was");

        // Run the restless novice for a minute through the full stack and check authored ranges.
        MasteryBackendBody::FDriver D(Seed);
        float MaxBody = 0.0f, MaxEye = 0.0f;
        bool bMoved = false;
        std::vector<std::pair<std::string, float>> First;
        for (int i = 0; i < 3600; ++i)
        {
            if (i % 300 == 0) D.React((i / 300) % 2 ? 1.0f : -1.0f, Face);
            const auto Off = MasteryBackendBody::ToLive2D(D.Tick(Face, Dt));
            FL2D Rig;
            MelodyLive2DBackend::ApplyPose(Rig, Face, &Off);
            for (auto& W : Rig.Out)
            {
                if (W.first.rfind("ParamBodyAngle", 0) == 0) MaxBody = std::max(MaxBody, std::fabs(W.second));
                if (W.first.rfind("ParamEyeBall", 0) == 0)  MaxEye = std::max(MaxEye, std::fabs(W.second));
            }
            if (i == 0) First = Rig.Out; else if (Rig.Out != First) bMoved = true;
        }
        std::printf("  over 60 s: max |ParamBodyAngle*| = %.2f   max |ParamEyeBall*| = %.2f\n", MaxBody, MaxEye);
        Check(bMoved, "the body layer animates Melody's rig");
        Check(MaxBody <= 3.0f + 1e-5f, "body angles stay inside the authored +/-3 degree range");
        Check(MaxEye <= 1.0f + 1e-5f, "eyeball offsets stay inside [-1,1]");
    }

    // ---- 11. End to end: signal -> binding -> body ----------------------------------------------
    std::printf("\n--- 11. end to end through the real binding (Melody) ---\n");
    {
        const FMasteryPersonaProfile Melody = MelodyPersona::Profile();
        const FMasteryEmbodimentPose PM = MasteryEmbodimentBinding::Evaluate(Measured(0.95f, 0.05f), Melody);
        const FMasteryEmbodimentPose PN = MasteryEmbodimentBinding::Evaluate(Measured(0.10f, 0.70f), Melody);
        const FReaction RM = MeasureReaction(PM), RN = MeasureReaction(PN);
        const FIdleStats IM = MeasureIdle(PM, 120.0f), IN = MeasureIdle(PN, 120.0f);
        std::printf("  intensity: master=%.2f novice=%.2f\n", PM.ExpressionIntensity, PN.ExpressionIntensity);
        std::printf("  master: econ=%.2f overshoot=%4.1f%% onset=%3.0fms fidgets=%2d sway=%.2f saccades=%3d\n",
                    PM.MotionEconomy, RM.OvershootFrac * 100.0f, RM.OnsetSec * 1000.0f, IM.Fidgets, IM.SwayRms, IM.Saccades);
        std::printf("  novice: econ=%.2f overshoot=%4.1f%% onset=%3.0fms fidgets=%2d sway=%.2f saccades=%3d\n",
                    PN.MotionEconomy, RN.OvershootFrac * 100.0f, RN.OnsetSec * 1000.0f, IN.Fidgets, IN.SwayRms, IN.Saccades);
        Check(RM.OvershootFrac < 0.01f && RN.OvershootFrac > 0.05f,
              "the measured master arrives clean; the measured novice visibly overcorrects");
        Check(RM.OnsetSec < RN.OnsetSec, "the measured master winds up less");
        Check(IM.Fidgets < IN.Fidgets && IM.SwayRms < IN.SwayRms, "the measured master is stiller");
        Check(IM.Saccades < IN.Saccades, "the measured master's gaze hunts less");
    }

    std::printf("\n=== %s ===\n", TestsFailed == 0 ? "BodyBackend=success" : "FAILURES PRESENT");
    return TestsFailed == 0 ? 0 : 1;
}
