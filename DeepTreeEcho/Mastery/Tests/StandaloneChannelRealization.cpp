// StandaloneChannelRealization.cpp
// Which pose channels actually reach a rig, and which are computed and then dropped?
//
// THE FINDING THAT PRODUCED THIS HARNESS
//
// MasteryEmbodimentBinding.h states the module's central design principle:
//
//     "Expertise is legible far more in motion economy than in facial expression. The expert does
//      not visibly hurry, does not overcorrect, and does not fidget; their gaze arrives where it
//      needs to be before the event does. So competence drives MotionEconomy, GazeSteadiness,
//      ReactionSharpness and (inversely) IdleFidget far more strongly than it drives smiling. A
//      binding that expressed mastery mainly through the mouth would read as smugness, not skill."
//
// Both shipped backends are face-only. MotionEconomy - named there as the primary carrier of
// legible expertise, and the single strongest "she is good at this" cue in the binding's own
// comments - is computed, smoothed, and referenced by eighteen assertions in the binding harness.
// It reaches no rig at all. Neither do IdleFidget, MicroMovementRate, ShoulderTension or
// SaccadeRate.
//
// So the module currently renders mastery ONLY through the face: exactly the failure its own
// header warns against.
//
// METHOD. Not grep. Grep tells you a symbol is absent from a file, which is weak evidence about
// behaviour - a channel could be read through an alias, folded into a derived term, or read and
// then multiplied by zero. Instead this drives each channel in isolation from a neutral pose and
// asks whether ANY backend output moves. That is the same technique that made the Live2D coverage
// and expression-register results trustworthy, and it catches the case grep cannot: a channel that
// is referenced but has no effect.
//
// The unrealized set is DECLARED here rather than merely tolerated, exactly as
// StandaloneLive2DCoverage declares the lip-sync channel and AuthoredExpressionSet declares
// JOY_05 out of scope. The test fails if a channel silently joins or leaves that set - so wiring
// a body backend will fail this test until the declaration is updated, which is the point.
//
// Build & run:
//   g++ -std=c++17 -O2 -I StandaloneShim -o chanreal StandaloneChannelRealization.cpp

#include <cstdio>
#include <string>
#include <vector>

#include "../MasteryEmbodimentPose.h"
#include "../Backends/MelodyLive2DBackend.h"
#include "../Backends/MasteryBackendMetaHuman.h"

namespace
{

int TestsFailed = 0;

void Check(bool bCond, const std::string& Label)
{
    std::printf("  [%s] %s\n", bCond ? "PASS" : "FAIL", Label.c_str());
    if (!bCond) ++TestsFailed;
}

/** Accepts every name and records name+value, so we can diff two applications exactly. */
struct FProbe : public IMelodyRigSink, public IMetaHumanRigSink
{
    std::vector<std::pair<std::string, float>> Out;

    bool HasParameter(const FString&) const override { return true; }
    void SetParameter(const FString& Id, float V) override { Out.push_back({Id.Str, V}); }
    bool HasCurve(const FString&) const override { return true; }
    void SetCurve(const FString& N, float V) override { Out.push_back({N.Str, V}); }
};

/** Apply a pose through BOTH backends and collect everything written. */
std::vector<std::pair<std::string, float>> RenderAll(const FMasteryEmbodimentPose& P)
{
    FProbe Probe;
    MelodyLive2DBackend::ApplyPose(Probe, P);
    MasteryBackendMetaHuman::ApplyPose(Probe, P);
    return Probe.Out;
}

bool Differs(const std::vector<std::pair<std::string, float>>& A,
             const std::vector<std::pair<std::string, float>>& B)
{
    if (A.size() != B.size()) return true;
    for (size_t i = 0; i < A.size(); ++i)
    {
        if (A[i].first != B[i].first) return true;
        const float d = A[i].second - B[i].second;
        if (d > 1e-6f || d < -1e-6f) return true;
    }
    return false;
}

/** One channel, how to perturb it, and whether it is expected to reach a rig. */
struct FChannel
{
    const char* Name;
    void (*Set)(FMasteryEmbodimentPose&);
    bool bExpectRealized;
    const char* Reason;   // why unrealized, when it is
};

} // namespace

int main()
{
    std::printf("=== Channel realization: which pose channels actually reach a rig? ===\n\n");

    // A neutral-but-expressive baseline. ExpressionIntensity must be non-zero or every channel
    // is scaled to nothing and the whole census would read as unrealized.
    FMasteryEmbodimentPose Base;
    Base.ExpressionIntensity = 1.0f;

    const FChannel Channels[] = {
        { "BrowRaise",           [](FMasteryEmbodimentPose& P){ P.BrowRaise = 1.0f; },          true,  "" },
        { "BrowFurrow",          [](FMasteryEmbodimentPose& P){ P.BrowFurrow = 1.0f; },         true,  "" },
        { "EyeWiden",            [](FMasteryEmbodimentPose& P){ P.EyeWiden = 1.0f; },           true,  "" },
        { "EyeNarrow",           [](FMasteryEmbodimentPose& P){ P.EyeNarrow = 1.0f; },          true,  "" },
        { "GazeSteadiness",      [](FMasteryEmbodimentPose& P){ P.GazeSteadiness = 1.0f; },     true,  "" },
        { "BlinkRate",           [](FMasteryEmbodimentPose& P){ P.BlinkRate = 2.0f; },          true,  "" },
        { "MouthCornerUp",       [](FMasteryEmbodimentPose& P){ P.MouthCornerUp = 1.0f; },      true,  "" },
        { "MouthCornerDown",     [](FMasteryEmbodimentPose& P){ P.MouthCornerDown = 1.0f; },    true,  "" },
        { "MouthTension",        [](FMasteryEmbodimentPose& P){ P.MouthTension = 1.0f; },       true,  "" },
        { "JawTension",          [](FMasteryEmbodimentPose& P){ P.JawTension = 1.0f; },         true,  "" },
        { "ChinRaise",           [](FMasteryEmbodimentPose& P){ P.ChinRaise = 1.0f; },          true,  "" },
        { "PostureUprightness",  [](FMasteryEmbodimentPose& P){ P.PostureUprightness = 1.0f; }, true,  "" },
        { "PostureLean",         [](FMasteryEmbodimentPose& P){ P.PostureLean = 1.0f; },        true,  "" },
        { "HeadTilt",            [](FMasteryEmbodimentPose& P){ P.HeadTilt = 1.0f; },           true,  "" },
        { "BreathRate",          [](FMasteryEmbodimentPose& P){ P.BreathRate = 30.0f; },        true,  "" },
        { "Asymmetry",           [](FMasteryEmbodimentPose& P){ P.Asymmetry = 1.0f; },          true,  "" },
        { "ExpressionIntensity", [](FMasteryEmbodimentPose& P){ P.ExpressionIntensity = 0.2f; },true,  "" },

        // ---- The gap ------------------------------------------------------------------------
        // Every one of these is a MOTION-QUALITY channel, and motion quality is what the binding
        // header names as the primary carrier of legible expertise. Both backends are face-only,
        // so none of them reaches a rig. This is a missing body/animation backend, not five
        // unrelated oversights.
        { "MotionEconomy",     [](FMasteryEmbodimentPose& P){ P.MotionEconomy = 1.0f; },     false,
          "THE headline expertise cue per the binding header; no body backend exists" },
        { "IdleFidget",        [](FMasteryEmbodimentPose& P){ P.IdleFidget = 1.0f; },        false,
          "named in the header as an inverse competence cue; needs an idle/fidget layer" },
        { "MicroMovementRate", [](FMasteryEmbodimentPose& P){ P.MicroMovementRate = 1.0f; }, false,
          "composed stillness vs restlessness; needs an additive noise layer" },
        { "ShoulderTension",   [](FMasteryEmbodimentPose& P){ P.ShoulderTension = 1.0f; },   false,
          "bracing; needs upper-body control, absent from both face rigs" },
        { "SaccadeRate",       [](FMasteryEmbodimentPose& P){ P.SaccadeRate = 1.0f; },       false,
          "eye darting is temporal; a look-at/saccade driver owns it, as with BlinkRate" },
        { "ReactionSharpness", [](FMasteryEmbodimentPose& P){ P.ReactionSharpness = 1.0f; }, false,
          "consumed by SelectMotionGroup, not by ApplyPose - motion selection, not a curve" },
    };

    const auto Baseline = RenderAll(Base);
    std::printf("baseline writes %d parameters across both backends\n\n", (int)Baseline.size());

    std::printf("%-22s %-10s %s\n", "channel", "reaches", "note");
    std::printf("%s\n", std::string(84, '-').c_str());

    int32 Mismatches = 0;
    int32 Realized = 0, Unrealized = 0;

    for (const FChannel& C : Channels)
    {
        FMasteryEmbodimentPose P = Base;
        C.Set(P);
        const bool bRealized = Differs(Baseline, RenderAll(P));

        if (bRealized) ++Realized; else ++Unrealized;

        const bool bMatch = (bRealized == C.bExpectRealized);
        if (!bMatch) ++Mismatches;

        std::printf("%-22s %-10s %s%s\n",
                    C.Name,
                    bRealized ? "a rig" : "NOTHING",
                    bMatch ? "" : ">>> DECLARATION MISMATCH <<< ",
                    C.Reason);
    }

    std::printf("\n%d of %d channels reach a rig; %d are computed and dropped.\n",
                Realized, (int32)(sizeof(Channels) / sizeof(Channels[0])), Unrealized);

    std::printf("\n--- assertions ---\n");

    Check(Mismatches == 0,
          "every channel's realization matches its declaration");

    // The specific claim, pinned so it cannot quietly change: the module's stated primary
    // expertise cue currently reaches no rig.
    {
        FMasteryEmbodimentPose Expert = Base;
        Expert.MotionEconomy = 1.0f;
        FMasteryEmbodimentPose Flailing = Base;
        Flailing.MotionEconomy = 0.0f;

        Check(!Differs(RenderAll(Expert), RenderAll(Flailing)),
              "a master and a novice are RIG-IDENTICAL on motion economy alone");
    }

    // And the converse, so the census cannot pass by everything being dead: the face channels
    // genuinely do distinguish states.
    {
        FMasteryEmbodimentPose Warm = Base;
        Warm.MouthCornerUp = 1.0f;
        Check(Differs(Baseline, RenderAll(Warm)),
              "the face channels are genuinely live - the harness can detect a difference");
    }

    // ExpressionIntensity gates everything, so at zero trust nothing should move at all. This is
    // "unknown is not excellence" observed at the rig rather than in the pose.
    {
        FMasteryEmbodimentPose Dark;
        Dark.ExpressionIntensity = 0.0f;
        FMasteryEmbodimentPose DarkButExpressive = Dark;
        DarkButExpressive.MouthCornerUp = 1.0f;
        DarkButExpressive.BrowRaise = 1.0f;
        DarkButExpressive.EyeNarrow = 1.0f;

        Check(!Differs(RenderAll(Dark), RenderAll(DarkButExpressive)),
              "at zero expression intensity the face cannot move, whatever the channels say");
    }

    std::printf("\n=== %s ===\n",
                TestsFailed == 0 ? "ChannelRealization=success" : "FAILURES PRESENT");
    return TestsFailed == 0 ? 0 : 1;
}
