// StandaloneMetaHumanBackend.cpp
// Does the MetaHuman backend actually match the project's FACS catalog?
//
// The curve names and decomposition weights in MasteryBackendMetaHuman.h come from
// expression_catalog_facs_complete.md (v4.0, section 6). This asserts that the code agrees with
// the document, so a later edit to either one that breaks the correspondence is caught.
//
//   [1] DECOMPOSITION - multi-curve action units split with the catalog's exact weights.
//   [2] INVARIANT     - "unknown is not excellence" reaches the rig: a low-trust pose cannot
//                       drive a strong face, no matter what the pose channels say.
//   [3] LATERALITY    - asymmetry produces genuinely unequal L/R, since a mirrored face reads
//                       as synthetic.
//   [4] PARTIAL RIG   - a rig missing curves degrades those channels silently rather than
//                       erroring or writing elsewhere.
//   [5] RANGE         - every written value stays in [0,1] for fuzzed input.
//   [6] DUCHENNE      - cheek raise accompanies a relaxed smile and withdraws under tension,
//                       which is what separates a felt smile from a strained one.
//
// Build & run:
//   g++ -std=c++17 -O2 -I ../Tests/StandaloneShim -o mhverify StandaloneMetaHumanBackend.cpp

#include <cstdio>
#include <map>
#include <random>
#include <string>
#include <vector>
#include <cmath>

#include "../MasteryEmbodimentPose.h"
#include "../Backends/MasteryBackendMetaHuman.h"

namespace
{

/** Records every curve write so assertions can inspect what the backend actually did. */
class FRecordingRig : public IMetaHumanRigSink
{
public:
    /** Curves this rig claims to expose. Empty set == expose everything. */
    std::vector<std::string> Available;
    std::map<std::string, float> Written;

    bool HasCurve(const FString& CurveName) const override
    {
        if (Available.empty()) return true;
        for (const std::string& A : Available)
        {
            if (A == CurveName.Str) return true;
        }
        return false;
    }

    void SetCurve(const FString& CurveName, float Value) override
    {
        Written[CurveName.Str] = Value;
    }

    float Get(const char* Name) const
    {
        auto It = Written.find(Name);
        return It == Written.end() ? -1.0f : It->second;
    }

    bool WasWritten(const char* Name) const { return Written.count(Name) > 0; }
};

int TestsFailed = 0;
void Check(bool bCond, const std::string& Label)
{
    std::printf("  [%s] %s\n", bCond ? "PASS" : "FAIL", Label.c_str());
    if (!bCond) ++TestsFailed;
}

bool Near(float A, float B, float Tol = 0.0015f) { return std::fabs(A - B) <= Tol; }

/** A fully-expressed pose with maximum trust, so intensity does not mask the mapping. */
FMasteryEmbodimentPose FullPose()
{
    FMasteryEmbodimentPose P;
    P.BrowRaise = 1.0f;
    P.BrowFurrow = 1.0f;
    P.EyeWiden = 1.0f;
    P.EyeNarrow = 1.0f;
    P.MouthCornerUp = 1.0f;
    P.MouthCornerDown = 1.0f;
    P.MouthTension = 0.0f;   // zero so the Duchenne term is not suppressed
    P.ChinRaise = 1.0f;
    P.JawTension = 0.0f;     // zero so AU17 isolates the deliberate chin-raise channel
    P.Asymmetry = 0.0f;      // zero so L and R are directly comparable
    P.BlinkRate = 15.0f;
    P.ExpressionIntensity = 1.0f;
    return P;
}

} // namespace

int main()
{
    std::printf("=== MetaHuman backend vs expression_catalog_facs_complete.md ===\n\n");

    // ------------------------------------------------------------------ [1] decomposition
    std::printf("[1] multi-curve AU decomposition matches the catalog's weights\n");
    {
        FRecordingRig Rig;
        MasteryBackendMetaHuman::ApplyPose(Rig, FullPose());

        std::printf("      AU1+AU2 brow_raise  -> Inner %.3f (want 0.600), Outer %.3f (want 0.400)\n",
                    Rig.Get("browRaiseInnerL"), Rig.Get("browRaiseOuterL"));
        Check(Near(Rig.Get("browRaiseInnerL"), 0.60f) && Near(Rig.Get("browRaiseOuterL"), 0.40f),
              "AU1+AU2 splits 0.6 inner / 0.4 outer");

        std::printf("      AU4 brow_furrow     -> Down  %.3f (want 0.700), Lateral %.3f (want 0.300)\n",
                    Rig.Get("browDownL"), Rig.Get("browLateralL"));
        Check(Near(Rig.Get("browDownL"), 0.70f) && Near(Rig.Get("browLateralL"), 0.30f),
              "AU4 splits 0.7 down / 0.3 lateral");

        std::printf("      AU5 eye_wide        -> UpperLid %.3f (want 0.600), Widen %.3f (want 0.400)\n",
                    Rig.Get("eyeUpperLidUpL"), Rig.Get("eyeWidenL"));
        Check(Near(Rig.Get("eyeUpperLidUpL"), 0.60f) && Near(Rig.Get("eyeWidenL"), 0.40f),
              "AU5 splits 0.6 upper-lid / 0.4 widen");

        // Single-curve AUs go through at full value.
        Check(Near(Rig.Get("eyeSquintInnerL"), 1.0f), "AU7 -> eyeSquintInner at full");
        Check(Near(Rig.Get("mouthCornerPullL"), 1.0f), "AU12 -> mouthCornerPull at full");
        Check(Near(Rig.Get("mouthCornerDepressL"), 1.0f), "AU15 -> mouthCornerDepress at full");
        Check(Near(Rig.Get("jawChinRaiseDL"), 1.0f), "AU17 -> jawChinRaiseD at full");

        // AU17 means DEFIANCE, not effort. Both a raised chin and a clenched jaw can recruit it,
        // but they carry opposite meaning, and an earlier version drove this curve from
        // JawTension alone - so the chin lifted only while STRUGGLING and never while winning,
        // which inverts the expression the catalog actually specifies (PUNK_03, mEXP_09).
        {
            FRecordingRig Assert, Strainy;
            FMasteryEmbodimentPose A;
            A.ExpressionIntensity = 1.0f;
            A.ChinRaise = 0.8f;
            MasteryBackendMetaHuman::ApplyPose(Assert, A);

            FMasteryEmbodimentPose S;
            S.ExpressionIntensity = 1.0f;
            S.JawTension = 0.8f;
            MasteryBackendMetaHuman::ApplyPose(Strainy, S);

            std::printf("      AU17 assert=%.3f strain=%.3f\n",
                        Assert.Get("jawChinRaiseDL"), Strainy.Get("jawChinRaiseDL"));
            Check(Assert.Get("jawChinRaiseDL") > 3.0f * Strainy.Get("jawChinRaiseDL"),
                  "AU17 is dominated by assertion, not by strain");
        }

        // The catalog's names, not ARKit's. A backend using ARKit names on a MetaHuman does
        // nothing at all, which is a failure mode worth pinning down explicitly.
        Check(!Rig.WasWritten("browInnerUp") && !Rig.WasWritten("mouthSmile_L") &&
              !Rig.WasWritten("eyeSquint_L"),
              "uses MetaHuman curve names, NOT ARKit blendshape names");
    }

    // ------------------------------------------------------------------ [2] the invariant
    std::printf("\n[2] 'unknown is not excellence' reaches the rig\n");
    {
        FMasteryEmbodimentPose Untrusted = FullPose();
        Untrusted.ExpressionIntensity = 0.25f;   // what a low-trust signal yields upstream

        FRecordingRig Weak, Strong;
        MasteryBackendMetaHuman::ApplyPose(Weak, Untrusted);
        MasteryBackendMetaHuman::ApplyPose(Strong, FullPose());

        std::printf("      smile at intensity 0.25 -> %.3f | at 1.00 -> %.3f\n",
                    Weak.Get("mouthCornerPullL"), Strong.Get("mouthCornerPullL"));
        Check(Weak.Get("mouthCornerPullL") < Strong.Get("mouthCornerPullL") * 0.35f,
              "every channel scales by ExpressionIntensity");
        Check(Weak.Get("browRaiseInnerL") < 0.2f,
              "an untrusted pose cannot pull a strong brow");
    }

    // ------------------------------------------------------------------ [3] laterality
    std::printf("\n[3] asymmetry produces genuinely unequal left/right\n");
    {
        FMasteryEmbodimentPose Asym = FullPose();
        Asym.MouthCornerUp = 0.7f;
        Asym.Asymmetry = 1.0f;

        FRecordingRig Rig;
        MasteryBackendMetaHuman::ApplyPose(Rig, Asym);
        const float L = Rig.Get("mouthCornerPullL");
        const float R = Rig.Get("mouthCornerPullR");
        std::printf("      smile L %.3f  R %.3f  (delta %.3f)\n", L, R, L - R);
        Check(std::fabs(L - R) > 0.05f, "asymmetry meaningfully separates L and R");

        FRecordingRig Sym;
        FMasteryEmbodimentPose NoAsym = Asym; NoAsym.Asymmetry = 0.0f;
        MasteryBackendMetaHuman::ApplyPose(Sym, NoAsym);
        Check(Near(Sym.Get("mouthCornerPullL"), Sym.Get("mouthCornerPullR")),
              "zero asymmetry gives an exactly mirrored face");
    }

    // ------------------------------------------------------------------ [4] partial rig
    std::printf("\n[4] a rig missing curves degrades silently\n");
    {
        FRecordingRig Partial;
        Partial.Available = { "mouthCornerPullL", "mouthCornerPullR" };   // only the smile exists
        MasteryBackendMetaHuman::ApplyPose(Partial, FullPose());

        std::printf("      partial rig received %zu curve writes\n", Partial.Written.size());
        Check(Partial.Written.size() == 2, "only the exposed curves were written");
        Check(Partial.WasWritten("mouthCornerPullL"), "the exposed curve still got its value");
        Check(!Partial.WasWritten("browRaiseInnerL"), "absent curves were skipped, not forced");
    }

    // ------------------------------------------------------------------ [5] range fuzz
    std::printf("\n[5] fuzzed poses keep every written value in range\n");
    {
        std::mt19937 Rng(31337);
        std::uniform_real_distribution<float> U(-0.5f, 1.5f);   // deliberately out of band
        bool bAllOk = true;

        for (int i = 0; i < 50000 && bAllOk; ++i)
        {
            FMasteryEmbodimentPose P;
            P.BrowRaise = U(Rng); P.BrowFurrow = U(Rng);
            P.EyeWiden = U(Rng);  P.EyeNarrow = U(Rng);
            P.MouthCornerUp = U(Rng); P.MouthCornerDown = U(Rng);
            P.MouthTension = U(Rng);  P.JawTension = U(Rng);
            P.Asymmetry = U(Rng);     P.BlinkRate = U(Rng) * 40.0f;
            P.ExpressionIntensity = U(Rng);

            FRecordingRig Rig;
            MasteryBackendMetaHuman::ApplyPose(Rig, P);
            for (const auto& KV : Rig.Written)
            {
                if (!std::isfinite(KV.second) || KV.second < -0.0001f || KV.second > 1.0001f)
                {
                    bAllOk = false;
                    std::printf("      OUT OF RANGE: %s = %f\n", KV.first.c_str(), KV.second);
                    break;
                }
            }
        }
        Check(bAllOk, "50k fuzzed poses produce only finite, clamped curve values");
    }

    // ------------------------------------------------------------------ [6] Duchenne
    std::printf("\n[6] cheek raise separates a felt smile from a strained one\n");
    {
        FMasteryEmbodimentPose Felt = FullPose();
        Felt.MouthCornerUp = 1.0f; Felt.MouthTension = 0.0f;

        FMasteryEmbodimentPose Strained = FullPose();
        Strained.MouthCornerUp = 1.0f; Strained.MouthTension = 1.0f;

        FRecordingRig FeltRig, StrainedRig;
        MasteryBackendMetaHuman::ApplyPose(FeltRig, Felt);
        MasteryBackendMetaHuman::ApplyPose(StrainedRig, Strained);

        std::printf("      cheekRaise: relaxed smile %.3f  vs  tense smile %.3f\n",
                    FeltRig.Get("eyeCheekRaiseL"), StrainedRig.Get("eyeCheekRaiseL"));
        Check(FeltRig.Get("eyeCheekRaiseL") > StrainedRig.Get("eyeCheekRaiseL") + 0.3f,
              "AU6 accompanies a relaxed smile and withdraws under tension");
        Check(Near(StrainedRig.Get("mouthCornerPullL"), FeltRig.Get("mouthCornerPullL")),
              "the mouth itself is unchanged - only the eyes tell the difference");
    }

    // ------------------------------------------------------------------ grades, for the log
    std::printf("\n[i] FACS intensity grading (catalog section 3)\n");
    for (float V : {0.0f, 0.15f, 0.35f, 0.55f, 0.75f, 0.95f})
    {
        std::printf("      %.2f -> %s\n", V,
                    MasteryBackendMetaHuman::GradeName(MasteryBackendMetaHuman::GradeOf(V)));
    }

    std::printf("\n=== %s ===\n", TestsFailed == 0 ? "MetaHumanBackend=success" : "FAILURES PRESENT");
    return TestsFailed == 0 ? 0 : 1;
}
