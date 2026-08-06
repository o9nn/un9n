// StandaloneExpressionRegister.cpp
// Can the binding reach every expression the artist authored - and does the competitive register
// answer to competitive states?
//
// The other harnesses in this module check that the binding is internally coherent, in range, and
// writable onto the rig. None of them can see a pose that is flawless in all three respects and
// still the WRONG FACE for the moment. "Self-consistent" and "correct register" are different
// properties, and only the first was ever measured.
//
// The artist's catalog supplies the ground truth. It separates the JOY register (warm, broadly
// symmetric) from the PUNK register (asymmetric smirk, narrowed eyes, raised chin) and names
// "Competitive or game-like interaction" as a trigger for the second - video_analysis_3db2fa59.md,
// and semantic_affective_patterns.md section 6.2. Placing those named expressions in the same
// space the binding writes into turns "she reads as a gamer girl" from an intention into a
// measurement.
//
// METHOD, and why it is reachability rather than classification. The obvious test - "a winning
// Melody must classify as PUNK" - is a trap, because it encodes MY guess about which authored
// expression a given competence ought to produce, and it can be satisfied by nudging prototypes
// until the guess comes true. That measures nothing. Reachability asks an objective question
// instead: sweep the whole signal x persona space and check that every authored expression is the
// nearest neighbour of SOME reachable pose. An expression no input can reach is a capability gap
// in exactly the sense StandaloneLive2DCoverage means it - present in the library, absent from
// the character.
//
// The sweep found one. PUNK_01 "Confident Smirk" and PUNK_02 "Mischievous Grin" both sit at
// asymmetry 0.67, while the binding's asymmetry span capped near 0.41 - so they were nearest to
// about 0.002% of 1.5M samples, and PUNK_05 and JOY_06 were never nearest to anything at all.
// Melody could be defiant but could not actually smirk. Widening the asymmetry span to reach the
// authored value fixed all four.
//
// This harness compiles the REAL binding, so the rules under test cannot drift from the rules
// that ship.
//
// Build & run:
//   g++ -std=c++17 -O2 -I StandaloneShim -o exprreg StandaloneExpressionRegister.cpp

#include <cstdio>
#include <map>
#include <string>

#include "../MasteryEmbodimentBinding.cpp"      // real rules, single translation unit
#include "../Character/AuthoredExpressionSet.h"
#include "../Personas/MelodyPersona.h"
#include "../Personas/CompetitivePersonas.h"

namespace
{

int TestsFailed = 0;

void Check(bool bCond, const std::string& Label)
{
    std::printf("  [%s] %s\n", bCond ? "PASS" : "FAIL", Label.c_str());
    if (!bCond) ++TestsFailed;
}

/** A well-measured signal: full channel coverage and ample evidence, so trust is high. */
FMasterySignal Measured()
{
    FMasterySignal S;
    S.bCompetenceValid = S.bTierValid = S.bDomainBreadthValid = true;
    S.bExecutionQualityValid = S.bTimingPrecisionValid = S.bReflexReadinessValid = true;
    S.bComboFlowValid = S.bPredictionAccuracyValid = S.bFlowIntensityValid = true;
    S.bChallengeSkillBalanceValid = S.bFrustrationValid = true;
    S.bConfidenceValid = S.bArousalValid = true;
    S.EvidenceCount = 400;
    S.TimeSinceLastEvidence = 0.0f;
    return S;
}

/** Winning a contested match: high competence, high confidence, real stakes. */
FMasterySignal WinningContested()
{
    FMasterySignal S = Measured();
    S.Competence = 0.90f;  S.Tier = 0.85f;  S.DomainBreadth = 0.80f;
    S.ExecutionQuality = 0.88f; S.TimingPrecision = 0.90f; S.ReflexReadiness = 0.85f;
    S.ComboFlow = 0.80f;   S.PredictionAccuracy = 0.85f;
    S.FlowIntensity = 0.75f;
    S.ChallengeSkillBalance = 0.55f;   // genuinely contested
    S.Frustration = 0.05f;
    S.Confidence = 0.90f;
    S.Arousal = 0.80f;                 // something is at stake
    return S;
}

/** The same competence with nothing on the line - a solved, unpressured position. */
FMasterySignal CoastingUnchallenged()
{
    FMasterySignal S = WinningContested();
    S.ChallengeSkillBalance = 0.15f;
    S.Arousal = 0.10f;
    S.FlowIntensity = 0.20f;
    return S;
}

/** Badly outmatched and frustrated. */
FMasterySignal Losing()
{
    FMasterySignal S = Measured();
    S.Competence = 0.30f; S.Tier = 0.25f; S.DomainBreadth = 0.30f;
    S.ExecutionQuality = 0.30f; S.TimingPrecision = 0.25f;
    S.ReflexReadiness = 0.30f; S.ComboFlow = 0.20f;
    S.PredictionAccuracy = 0.25f; S.FlowIntensity = 0.10f;
    S.ChallengeSkillBalance = 0.95f;
    S.Frustration = 0.80f;
    S.Confidence = 0.15f;
    S.Arousal = 0.85f;
    return S;
}

FMasteryPersonaProfile CompetitiveMelody()
{
    return CompetitivePersonas::Blend(
        { CompetitivePersonas::Aion(), CompetitivePersonas::Toga() },
        { 0.5f, 0.5f },
        TEXT("CompetitiveMelody"));
}

void Report(const char* Label, const FMasteryEmbodimentPose& P,
            const AuthoredExpressionSet::FClassification& C)
{
    std::printf("      %-24s -> %-30s [%s]\n",
                Label, C.Id, AuthoredExpressionSet::CategoryName(C.Category));
    std::printf("      %-24s    smile %.2f  asym %.2f  chin %.2f  narrow %.2f  tilt %+.2f  I %.2f\n",
                "", P.MouthCornerUp, P.Asymmetry, P.ChinRaise, P.EyeNarrow,
                P.HeadTilt, P.ExpressionIntensity);
}

} // namespace

int main()
{
    std::printf("=== Expression register: is every authored expression reachable? ===\n\n");

    const FMasteryPersonaProfile Competitive = CompetitiveMelody();

    // ------------------------------------------------------------------ [1] reachability census
    std::printf("[1] sweep the signal x persona space; census which expressions are reachable\n");
    {
        std::map<std::string, long> Hits;

        const TArray<FAuthoredExpression> Lib = AuthoredExpressionSet::Library();
        for (int32 i = 0; i < Lib.Num(); ++i)
        {
            Hits[std::string(Lib[i].Id)] = 0;      // every entry must be accounted for
        }

        TArray<FMasteryPersonaProfile> Personas;
        Personas.Add(MelodyPersona::Profile());
        Personas.Add(CompetitivePersonas::Aion());
        Personas.Add(CompetitivePersonas::Toga());
        Personas.Add(Competitive);

        const float Steps[] = { 0.0f, 0.25f, 0.5f, 0.75f, 1.0f };
        long Samples = 0;

        for (int32 p = 0; p < Personas.Num(); ++p)
        for (float Comp : Steps) for (float Exec : Steps)
        for (float Pred : Steps) for (float Flow : Steps)
        for (float Bal  : Steps) for (float Frus : Steps)
        for (float Conf : Steps) for (float Aro  : Steps)
        {
            FMasterySignal S = Measured();
            S.Competence = Comp; S.Tier = Comp; S.DomainBreadth = Comp;
            S.ExecutionQuality = Exec; S.TimingPrecision = Exec;
            S.ReflexReadiness = Exec;  S.ComboFlow = Exec;
            S.PredictionAccuracy = Pred; S.FlowIntensity = Flow;
            S.ChallengeSkillBalance = Bal; S.Frustration = Frus;
            S.Confidence = Conf; S.Arousal = Aro;

            const auto C = AuthoredExpressionSet::Classify(
                MasteryEmbodimentBinding::Evaluate(S, Personas[p]));
            ++Hits[std::string(C.Id)];
            ++Samples;
        }

        std::printf("      %ld samples over %d personas\n\n", Samples, Personas.Num());

        // A floor rather than "greater than zero": an expression reachable from a handful of
        // samples in a million is not meaningfully reachable, it is a rounding artifact. 0.01%
        // is low enough to admit genuinely rare expressions and high enough to catch the gap
        // that prompted this test (PUNK_01 sat at 0.002% before the asymmetry fix).
        const long Floor = Samples / 10000;
        int32 Unreachable = 0;
        int32 ExcludedButReached = 0;

        for (int32 i = 0; i < Lib.Num(); ++i)
        {
            const std::string Id(Lib[i].Id);
            const long N = Hits[Id];
            const bool bReached = N >= Floor;
            const bool bExpected = Lib[i].bMasteryReachable;

            const char* Note = "";
            if (bExpected && !bReached)      { Note = "<-- UNREACHABLE"; ++Unreachable; }
            else if (!bExpected && bReached) { Note = "<-- out of scope, yet reached";
                                               ++ExcludedButReached; }
            else if (!bExpected)             { Note = "(out of mastery scope, as declared)"; }

            std::printf("      %-32s %8ld  %6.3f%%  %s\n",
                        Id.c_str(), N,
                        100.0 * static_cast<double>(N) / static_cast<double>(Samples), Note);
        }

        std::printf("\n");
        Check(Unreachable == 0,
              "every in-scope authored expression is reachable from some measurable state");

        // The exclusion is asserted, not merely tolerated - the same discipline the Live2D
        // coverage harness applies to the lip-sync channel. If a competence signal ever starts
        // producing bliss, that is a finding too.
        Check(ExcludedButReached == 0,
              "and out-of-scope expressions stay out of reach of a competence signal");
    }

    // ------------------------------------------------------------------ [2] the competitive axis
    std::printf("\n[2] asymmetry is what separates the smirk from the smile\n");
    {
        const FMasteryEmbodimentPose Playful =
            MasteryEmbodimentBinding::Evaluate(WinningContested(), Competitive);

        FMasteryPersonaProfile Flat = Competitive;
        Flat.PlayfulnessBias = 0.0f;
        const FMasteryEmbodimentPose Deadpan =
            MasteryEmbodimentBinding::Evaluate(WinningContested(), Flat);

        Report("winning, playful", Playful, AuthoredExpressionSet::Classify(Playful));
        Report("winning, deadpan", Deadpan, AuthoredExpressionSet::Classify(Deadpan));

        // Identical competence, identical signal - only temperament differs. Skill must not move.
        Check(FMath::Abs(Playful.MotionEconomy - Deadpan.MotionEconomy) < 1e-5f,
              "temperament changes the face without touching measured competence");
        Check(AuthoredExpressionSet::Classify(Deadpan).Category != EAuthoredExpressionCategory::Punk,
              "with playfulness removed she leaves the competitive register");
        Check(Deadpan.Asymmetry < 0.25f,
              "and asymmetry collapses to the non-synthetic baseline");
    }

    // ------------------------------------------------------------------ [3] engagement, not ease
    std::printf("\n[3] REGRESSION: asymmetry tracks engagement, not ease\n");
    {
        // The bug: asymmetry was scaled by Ease, so the harder the fight the more symmetric -
        // and therefore the more synthetic - her face became, exactly when the stakes were
        // highest and a synthetic face costs the most.
        const FMasteryEmbodimentPose Contested =
            MasteryEmbodimentBinding::Evaluate(WinningContested(), Competitive);
        const FMasteryEmbodimentPose Coasting =
            MasteryEmbodimentBinding::Evaluate(CoastingUnchallenged(), Competitive);

        std::printf("      contested asym %.3f   coasting asym %.3f\n",
                    Contested.Asymmetry, Coasting.Asymmetry);
        Check(Contested.Asymmetry > Coasting.Asymmetry,
              "a contested exchange is MORE asymmetric than an unchallenged one");
    }

    // ------------------------------------------------------------------ [4] chin raise semantics
    std::printf("\n[4] REGRESSION: the chin lifts when winning, not when struggling\n");
    {
        // AU17 was driven by JawTension alone, inverting its meaning: the only way to raise the
        // chin was to be under strain, so a losing character asserted and a winning one never
        // did. The catalog glosses chin raise as "defiance, confidence" (mEXP_09, PUNK_03).
        const FMasteryEmbodimentPose Won =
            MasteryEmbodimentBinding::Evaluate(WinningContested(), Competitive);
        const FMasteryEmbodimentPose Lost =
            MasteryEmbodimentBinding::Evaluate(Losing(), Competitive);

        std::printf("      winning chin %.3f (jaw %.3f)   losing chin %.3f (jaw %.3f)\n",
                    Won.ChinRaise, Won.JawTension, Lost.ChinRaise, Lost.JawTension);

        Check(Won.ChinRaise > Lost.ChinRaise,
              "defiant chin raise is higher when winning than when outmatched");
        Check(Lost.JawTension > Won.JawTension,
              "jaw clench moves the other way - it is the strain channel");

        // And it stays on the catalog's scale. PUNK_03, the most assertive entry in the whole
        // library, is authored at 0.20; a winner should be near that, not at a chin-up sneer.
        Check(Won.ChinRaise < 0.45f,
              "chin raise stays on the authored scale rather than overdriving");
    }

    // ------------------------------------------------------------------ [5] signed head tilt
    std::printf("\n[5] both authored tilt directions are reachable\n");
    {
        // The authored set tilts both ways - WONDER_02 to +8 degrees, JOY_02 and
        // PHOTO_ExuberantLaugh to -4 and -5 - so an unsigned magnitude could only ever produce
        // half of it, and a character who tilts her head exactly one way reads as broken.
        FMasterySignal Puzzled = Measured();
        Puzzled.Competence = 0.5f; Puzzled.Tier = 0.5f; Puzzled.DomainBreadth = 0.5f;
        Puzzled.ExecutionQuality = 0.5f; Puzzled.TimingPrecision = 0.5f;
        Puzzled.ReflexReadiness = 0.5f; Puzzled.ComboFlow = 0.5f;
        Puzzled.PredictionAccuracy = 0.0f;      // cannot read this opponent yet
        Puzzled.FlowIntensity = 0.3f;
        Puzzled.ChallengeSkillBalance = 0.5f;
        Puzzled.Confidence = 0.3f; Puzzled.Arousal = 0.5f;

        const float Inquisitive = MasteryEmbodimentBinding::Evaluate(Puzzled, Competitive).HeadTilt;
        const float Mirthful = MasteryEmbodimentBinding::Evaluate(WinningContested(), Competitive).HeadTilt;

        std::printf("      unreadable opponent %+.2f   already winning %+.2f\n",
                    Inquisitive, Mirthful);
        Check(Inquisitive > 0.0f, "an unread opponent produces the inquisitive (+) tilt");
        Check(Mirthful < 0.0f,    "a settled winning position produces the mirthful (-) tilt");
    }

    // ------------------------------------------------------------------ [6] unknown is not mastery
    std::printf("\n[6] an unmeasured Melody cannot put on the winner's face\n");
    {
        FMasterySignal Unknown;                 // no valid channels, no evidence
        const FMasteryEmbodimentPose P =
            MasteryEmbodimentBinding::Evaluate(Unknown, Competitive);
        const auto C = AuthoredExpressionSet::Classify(P);
        Report("unmeasured", P, C);

        // The module's safety invariant, followed all the way to the expression register.
        Check(C.Category != EAuthoredExpressionCategory::Punk,
              "no evidence cannot buy the competitive register");
        Check(P.ExpressionIntensity <= 0.35f,
              "and the amplitude stays capped");
    }

    // ------------------------------------------------------------------ [7] library sanity
    std::printf("\n[7] every prototype classifies as itself\n");
    {
        const TArray<FAuthoredExpression> Lib = AuthoredExpressionSet::Library();
        int32 SelfMatched = 0;
        for (int32 i = 0; i < Lib.Num(); ++i)
        {
            FMasteryEmbodimentPose P;
            P.BrowRaise = Lib[i].BrowRaise;
            P.BrowFurrow = Lib[i].BrowFurrow;
            P.EyeWiden = Lib[i].EyeWiden;
            P.EyeNarrow = Lib[i].EyeNarrow;
            P.MouthCornerUp = Lib[i].MouthCornerUp;
            P.MouthCornerDown = Lib[i].MouthCornerDown;
            P.ChinRaise = Lib[i].ChinRaise;
            P.Asymmetry = Lib[i].Asymmetry;
            P.HeadTilt = Lib[i].HeadTilt;

            const auto C = AuthoredExpressionSet::Classify(P);
            if (C.Distance < 1e-4f) ++SelfMatched;
            else std::printf("      %-30s -> %-30s d=%.4f\n", Lib[i].Id, C.Id, C.Distance);
        }
        std::printf("      %d/%d prototypes are their own nearest neighbour\n",
                    SelfMatched, Lib.Num());
        Check(SelfMatched == Lib.Num(),
              "the library has no duplicate or shadowed prototypes");
    }

    std::printf("\n=== %s ===\n", TestsFailed == 0 ? "ExpressionRegister=success" : "FAILURES PRESENT");
    return TestsFailed == 0 ? 0 : 1;
}
