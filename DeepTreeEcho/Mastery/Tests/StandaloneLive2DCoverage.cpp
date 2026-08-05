// StandaloneLive2DCoverage.cpp
// Can the binding actually REACH what the artist authored?
//
// This is the question the other harnesses cannot answer. They verify the binding is
// self-consistent; this verifies it is *sufficient*. If an artist authors JOY_05 "Blissful" with
// ParamEyeLOpen at 0.15 and the binding can only ever produce 0.40, then the system physically
// cannot express bliss - and nothing in a self-consistency test would ever say so.
//
// Method: sweep the pose input space, record the min and max the backend emits per parameter,
// and compare against the range across 12 authored .exp3.json expressions
// (Character/Authored/). A parameter whose authored range is not covered is a capability gap.
//
// Authored ranges, extracted from those files:
//
//   ParamEyeLOpen / ParamEyeROpen    0.15 .. 1.00
//   ParamBrowLY   / ParamBrowRY     -0.10 .. 0.60
//   ParamBrowLAngle / ParamBrowRAngle -0.30 .. 0.30
//   ParamMouthForm                  -0.40 .. 1.00
//   ParamMouthOpenY                  0.00 .. 0.80
//   ParamAngleX                     -3.0  .. 3.0   (degrees)
//   ParamAngleY                     -2.0  .. 6.0
//   ParamAngleZ                     -5.0  .. 8.0
//
// SCOPE - what this harness does NOT prove. It sweeps the POSE space directly, so it establishes
// that the BACKEND can reach the authored values. It does not establish that the SYSTEM can,
// because the binding does not produce every pose in that space. StandaloneExpressionRegister
// found a concrete instance: this file reaches ParamEyeLOpen 0.15 by driving EyeNarrow to 1.0,
// but the binding caps EyeNarrow at 0.75 (0.45*Flow + 0.3*Skill*Arousal), so a mastery signal
// alone can only close the eyes to about 0.36. That is a deliberate scope boundary rather than a
// bug - JOY_05 "Blissful" is triggered by transcendence, not by competence - but the distinction
// between backend reachability and system reachability is real, and only the second one is what
// a viewer actually sees.
//
// NOTE ON MouthOpenY. The backend deliberately does NOT cover the authored 0.80 maximum, and
// that is correct rather than a gap: ParamMouthOpenY is the rig's LipSync channel, owned by a
// speech system. The authored 0.55-0.80 values come from SPEAK_01 and the laughing expressions,
// which are mouth SHAPES, not sustained states. The binding contributes only a small resting
// offset so it does not fight lip-sync for the channel. The test asserts that deliberate
// under-coverage rather than pretending it is full.
//
// Build & run:
//   g++ -std=c++17 -O2 -I ../Tests/StandaloneShim -o l2dcover StandaloneLive2DCoverage.cpp

#include <cstdio>
#include <map>
#include <string>
#include <vector>
#include <cmath>

#include "../MasteryEmbodimentPose.h"
#include "../Backends/MelodyLive2DBackend.h"

namespace
{

class FSweepRig : public IMelodyRigSink
{
public:
    std::map<std::string, std::pair<float,float>> Range;   // param -> (min, max)

    bool HasParameter(const FString&) const override { return true; }

    void SetParameter(const FString& Id, float V) override
    {
        auto It = Range.find(Id.Str);
        if (It == Range.end()) { Range[Id.Str] = {V, V}; }
        else
        {
            It->second.first  = std::min(It->second.first,  V);
            It->second.second = std::max(It->second.second, V);
        }
    }

    bool Covers(const char* Param, float Lo, float Hi, float Tol) const
    {
        auto It = Range.find(Param);
        if (It == Range.end()) return false;
        return It->second.first <= Lo + Tol && It->second.second >= Hi - Tol;
    }

    std::pair<float,float> Get(const char* Param) const
    {
        auto It = Range.find(Param);
        return It == Range.end() ? std::make_pair(0.0f, 0.0f) : It->second;
    }
};

int TestsFailed = 0;
void Check(bool bCond, const std::string& Label)
{
    std::printf("  [%s] %s\n", bCond ? "PASS" : "FAIL", Label.c_str());
    if (!bCond) ++TestsFailed;
}

/** Sweep the pose space exhaustively enough to bound the backend's output range. */
FSweepRig SweepPoseSpace()
{
    FSweepRig Rig;
    const float Steps[] = {0.0f, 0.25f, 0.5f, 0.75f, 1.0f};

    for (float Intensity : {0.0f, 0.5f, 1.0f})
    for (float BrowRaise : Steps)
    for (float BrowFurrow : Steps)
    for (float EyeNarrow : Steps)
    for (float EyeWiden : Steps)
    for (float SmileUp : Steps)
    for (float SmileDown : Steps)
    for (float Tension : Steps)
    for (float Tilt : {-1.0f, 0.0f, 1.0f})
    for (float Lean : {-1.0f, 0.0f, 1.0f})
    for (float Gaze : {0.0f, 1.0f})
    for (float Upright : {0.0f, 1.0f})
    {
        FMasteryEmbodimentPose P;
        P.ExpressionIntensity = Intensity;
        P.BrowRaise = BrowRaise;
        P.BrowFurrow = BrowFurrow;
        P.EyeNarrow = EyeNarrow;
        P.EyeWiden = EyeWiden;
        P.MouthCornerUp = SmileUp;
        P.MouthCornerDown = SmileDown;
        P.MouthTension = Tension;
        P.HeadTilt = Tilt;
        P.PostureLean = Lean;
        P.GazeSteadiness = Gaze;
        P.PostureUprightness = Upright;
        P.Asymmetry = 0.0f;
        MelodyLive2DBackend::ApplyPose(Rig, P);
    }
    return Rig;
}

} // namespace

int main()
{
    std::printf("=== Live2D coverage: can the binding reach the authored expressions? ===\n\n");

    const FSweepRig Rig = SweepPoseSpace();

    std::printf("%-22s %16s   %16s\n", "parameter", "backend range", "authored range");
    std::printf("%s\n", std::string(60, '-').c_str());

    struct FTarget { const char* Param; float Lo; float Hi; bool bMustCover; const char* Note; };
    const FTarget Targets[] = {
        { "ParamEyeLOpen",    0.15f,  1.00f, true,  "" },
        { "ParamEyeROpen",    0.15f,  1.00f, true,  "" },
        { "ParamBrowLY",     -0.10f,  0.60f, true,  "" },
        { "ParamBrowRY",     -0.10f,  0.60f, true,  "" },
        { "ParamBrowLAngle", -0.30f,  0.30f, true,  "" },
        { "ParamBrowRAngle", -0.30f,  0.30f, true,  "" },
        { "ParamMouthForm",  -0.40f,  1.00f, true,  "" },
        { "ParamAngleX",     -3.00f,  3.00f, true,  "" },
        { "ParamAngleY",     -2.00f,  6.00f, false, "gaze-driven only, one-sided by design" },
        { "ParamAngleZ",     -5.00f,  8.00f, true,  "" },
        { "ParamMouthOpenY",  0.00f,  0.80f, false, "LipSync channel - speech owns the top end" },
    };

    for (const FTarget& T : Targets)
    {
        const auto R = Rig.Get(T.Param);
        const bool bCovered = Rig.Covers(T.Param, T.Lo, T.Hi, 0.02f);
        std::printf("%-22s %7.2f..%-7.2f %7.2f..%-7.2f  %s%s\n",
                    T.Param, R.first, R.second, T.Lo, T.Hi,
                    bCovered ? "ok" : (T.bMustCover ? "GAP" : "partial"),
                    T.Note[0] ? "  " : "");
        if (T.Note[0]) std::printf("%-22s %s\n", "", T.Note);
    }

    std::printf("\n--- assertions ---\n");

    // The channels that MUST be fully reachable, because an authored expression depends on them.
    for (const FTarget& T : Targets)
    {
        if (!T.bMustCover) continue;
        Check(Rig.Covers(T.Param, T.Lo, T.Hi, 0.02f),
              std::string(T.Param) + " covers the authored range");
    }

    // JOY_05 "Blissful" is the specific expression that broke the earlier calibration - it needs
    // the eyes almost shut, and the previous formula bottomed out around 0.40.
    Check(Rig.Get("ParamEyeLOpen").first <= 0.20f,
          "eyes can close far enough for JOY_05 Blissful (0.15)");

    // SADNESS_01 needs a NEGATIVE brow angle while the brow itself is RAISED - the oblique sad
    // brow. A backend deriving angle purely from raise cannot produce this.
    {
        FSweepRig One;
        FMasteryEmbodimentPose Sad;
        Sad.ExpressionIntensity = 1.0f;
        Sad.BrowRaise = 0.3f;          // brow raised
        Sad.MouthCornerDown = 0.8f;    // and frowning
        MelodyLive2DBackend::ApplyPose(One, Sad);
        const float Angle = One.Get("ParamBrowLAngle").first;
        const float Y = One.Get("ParamBrowLY").first;
        std::printf("      melancholy probe: BrowY %+.2f with BrowAngle %+.2f\n", Y, Angle);
        Check(Y > 0.0f && Angle < 0.0f,
              "a raised-but-oblique sad brow is reachable (SADNESS_01 shape)");
    }

    // Head angles must stay in the authored envelope - the earlier version drove 30 degrees.
    {
        const auto Z = Rig.Get("ParamAngleZ");
        std::printf("      head tilt envelope: %.1f .. %.1f degrees (authored max 8)\n",
                    Z.first, Z.second);
        Check(Z.second <= 8.5f && Z.first >= -5.5f,
              "head tilt stays inside the authored envelope, not 30 degrees");
    }

    // And the deliberate under-coverage is asserted rather than silently tolerated.
    Check(!Rig.Covers("ParamMouthOpenY", 0.0f, 0.80f, 0.02f),
          "MouthOpenY is deliberately NOT fully driven - lip-sync owns it");

    std::printf("\n=== %s ===\n", TestsFailed == 0 ? "Live2DCoverage=success" : "FAILURES PRESENT");
    return TestsFailed == 0 ? 0 : 1;
}
