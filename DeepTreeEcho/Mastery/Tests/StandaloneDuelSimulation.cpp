// StandaloneDuelSimulation.cpp
// Tier-1 integration test: the whole stack on a real game.
//
// Every prior harness tested a pure function against hand-written inputs. This runs actual
// matches and asserts the pipeline end to end:
//
//     gameplay -> telemetry -> FMasterySignal -> embodiment binding -> pose
//
// The questions it answers, which no unit test can:
//
//   [1] Does the GAME reward skill? If a master does not beat a novice, every mastery number
//       downstream is measuring noise, and nothing else in this file means anything. This is
//       the load-bearing test.
//   [2] Do the mastery signals derived from REAL play track real skill, monotonically?
//   [3] Does "unknown is not excellence" survive contact with real data - specifically, does a
//       round too short to measure anything correctly refuse to render as competence?
//   [4] Does the embodiment binding produce visibly different bodies for a master and a novice
//       when driven by measured rather than synthetic signal?
//   [5] Is the simulation deterministic (same seed -> same result)? Required for any of this
//       to be a regression test rather than a coin flip.
//
// Build & run:
//   g++ -std=c++17 -O2 -o duelsim StandaloneDuelSimulation.cpp && ./duelsim

#include <cstdio>
#include <string>
#include <vector>
#include <cmath>

#include "../Simulation/DuelAgent.h"

// The binding is UE-flavoured (FMath, FString); mirror its rules here exactly, as the other
// standalone harnesses do. Kept in lockstep with MasteryEmbodimentBinding.cpp.
namespace Bind
{
    float Clamp01(float V) { return V < 0.0f ? 0.0f : (V > 1.0f ? 1.0f : V); }
    float Lerp(float A, float B, float T) { return A + (B - A) * T; }

    struct Pose
    {
        float MotionEconomy = 0.5f, GazeSteadiness = 0.5f, IdleFidget = 0.5f;
        float ReactionSharpness = 0.5f, BrowFurrow = 0.0f, MouthCornerUp = 0.0f;
        float ExpressionIntensity = 0.0f, PostureUprightness = 0.5f;
    };

    struct Persona
    {
        float ExpressivenessGain = 1.25f, BaselineWarmth = 0.22f, TensionBias = -0.15f;
        float FidgetBias = 0.10f, ConfidenceDisplayStyle = 0.72f, Composure = 0.70f;
        float BasePostureUprightness = 0.62f, UnknownStateIntensityCap = 0.30f;
    };

    Pose Evaluate(const FMasterySignal& S, const Persona& P)
    {
        Pose Out;
        const float Trust = S.GetSignalTrust();

        const float Competence = S.GetCompetence(0.0f);
        const float Tier       = S.GetTier(0.0f);
        const float Execution  = S.GetExecutionQuality(0.0f);
        const float Timing     = S.GetTimingPrecision(0.0f);
        const float Reflex     = S.GetReflexReadiness(0.0f);
        const float Prediction = S.GetPredictionAccuracy(0.0f);
        const float Flow       = S.GetFlowIntensity(0.0f);
        const float Balance    = S.GetChallengeSkillBalance(0.5f);
        const float Frust      = S.GetFrustration(0.0f);
        const float Conf       = S.GetConfidence(0.0f);
        const float Arousal    = S.GetArousal(0.0f);

        const float Skill = Clamp01(0.5f*Competence + 0.2f*Tier + 0.2f*Execution + 0.1f*Timing);
        const float Overload = Clamp01((Balance - 0.5f) * 2.0f) * (1.0f - P.Composure);
        const float Underload = Clamp01((0.5f - Balance) * 2.0f);
        const float Strain = Clamp01(Overload + Frust * (1.0f - P.Composure));

        Out.MotionEconomy = Clamp01(Lerp(0.25f, 0.95f, Skill) - 0.25f*Strain);
        Out.ReactionSharpness = Clamp01(0.15f + 0.5f*Reflex + 0.35f*Prediction);
        Out.IdleFidget = Clamp01(0.55f - 0.4f*Skill - 0.25f*Flow + 0.3f*Underload + P.FidgetBias);
        Out.GazeSteadiness = Clamp01(0.3f + 0.4f*Skill + 0.3f*Flow - 0.25f*Strain);
        Out.BrowFurrow = Clamp01((0.55f*Overload + 0.5f*Frust + 0.06f*Flow) * (1.0f + P.TensionBias));
        const float Ease = Clamp01(0.5f*Skill + 0.3f*Conf + 0.2f*Flow - 0.5f*Strain);
        Out.MouthCornerUp = Clamp01(P.BaselineWarmth + Ease*P.ConfidenceDisplayStyle);
        Out.PostureUprightness = Clamp01(P.BasePostureUprightness + 0.3f*Skill + 0.15f*Conf
                                         - 0.3f*Underload - 0.2f*Strain);

        const float Raw = Clamp01(0.25f + 0.4f*Skill + 0.3f*Arousal + 0.3f*Strain + 0.2f*Flow);
        const float Cap = Lerp(P.UnknownStateIntensityCap, 1.0f, Trust);
        const float I = Clamp01(Raw * P.ExpressivenessGain);
        Out.ExpressionIntensity = I < Cap ? I : Cap;
        return Out;
    }
}

namespace
{

using namespace Duel;

struct MatchOutcome
{
    int WinsA = 0, WinsB = 0, Draws = 0;
    FRoundTelemetry TelA, TelB;
    int Rounds = 0;
};

void Accumulate(FRoundTelemetry& Dst, const FRoundTelemetry& Src)
{
    Dst.AttacksThrown += Src.AttacksThrown;
    Dst.AttacksLanded += Src.AttacksLanded;
    Dst.AttacksWhiffed += Src.AttacksWhiffed;
    Dst.PunishOpportunities += Src.PunishOpportunities;
    Dst.PunishesConverted += Src.PunishesConverted;
    Dst.TimesPunished += Src.TimesPunished;
    Dst.BlocksAttempted += Src.BlocksAttempted;
    Dst.DodgesAttempted += Src.DodgesAttempted;
    Dst.DodgesSuccessful += Src.DodgesSuccessful;
    Dst.FramesInRange += Src.FramesInRange;
    Dst.TotalFrames += Src.TotalFrames;
    Dst.DamageDealt += Src.DamageDealt;
    Dst.DamageTaken += Src.DamageTaken;
    Dst.ReactionSamples += Src.ReactionSamples;
    Dst.ReactionFrameSum += Src.ReactionFrameSum;
}

MatchOutcome RunMatch(const FAgentSkill& SkillA, const FAgentSkill& SkillB,
                      int Rounds, unsigned Seed)
{
    MatchOutcome Out;
    Out.Rounds = Rounds;

    for (int r = 0; r < Rounds; ++r)
    {
        FGameState S;
        S.Reset();
        FScriptedAgent A(SkillA, Seed + r * 31u);
        FScriptedAgent B(SkillB, Seed + r * 31u + 7919u);
        A.Reset(); B.Reset();

        // Reaction latency is a property of the agent; record it as telemetry so the signal
        // derivation can read it back out rather than being told.
        S.TelA.ReactionSamples = 1; S.TelA.ReactionFrameSum = SkillA.ReactionFrames;
        S.TelB.ReactionSamples = 1; S.TelB.ReactionFrameSum = SkillB.ReactionFrames;

        while (!S.IsOver())
        {
            const float D = S.Distance();
            const EAction ActA = A.ChooseAction(S.A, S.B, D);
            const EAction ActB = B.ChooseAction(S.B, S.A, D);
            StepFrame(S, ActA, ActB);
        }

        const int W = S.Winner();
        if (W > 0) ++Out.WinsA; else if (W < 0) ++Out.WinsB; else ++Out.Draws;
        Accumulate(Out.TelA, S.TelA);
        Accumulate(Out.TelB, S.TelB);
    }
    return Out;
}

int TestsFailed = 0;
void Check(bool bCond, const std::string& Label)
{
    std::printf("  [%s] %s\n", bCond ? "PASS" : "FAIL", Label.c_str());
    if (!bCond) ++TestsFailed;
}

} // namespace

int main()
{
    std::printf("=== Tier-1 integration: real gameplay -> mastery signal -> embodiment ===\n\n");

    constexpr int ROUNDS = 60;

    // ------------------------------------------------------------------ [1] does skill matter?
    std::printf("[1] does the GAME reward skill? (load-bearing - everything else depends on it)\n");
    const MatchOutcome MvN = RunMatch(FAgentSkill::Master(), FAgentSkill::Novice(), ROUNDS, 1234);
    const MatchOutcome AvN = RunMatch(FAgentSkill::Average(), FAgentSkill::Novice(), ROUNDS, 1234);
    const MatchOutcome MvA = RunMatch(FAgentSkill::Master(), FAgentSkill::Average(), ROUNDS, 1234);
    const MatchOutcome MvM = RunMatch(FAgentSkill::Master(), FAgentSkill::Master(), ROUNDS, 1234);

    auto WinRate = [](const MatchOutcome& M) { return float(M.WinsA) / float(M.Rounds); };
    std::printf("      master  vs novice : %d-%d-%d  (win rate %.2f)\n", MvN.WinsA, MvN.Draws, MvN.WinsB, WinRate(MvN));
    std::printf("      average vs novice : %d-%d-%d  (win rate %.2f)\n", AvN.WinsA, AvN.Draws, AvN.WinsB, WinRate(AvN));
    std::printf("      master  vs average: %d-%d-%d  (win rate %.2f)\n", MvA.WinsA, MvA.Draws, MvA.WinsB, WinRate(MvA));
    std::printf("      master  vs master : %d-%d-%d  (win rate %.2f, expect ~0.5)\n", MvM.WinsA, MvM.Draws, MvM.WinsB, WinRate(MvM));

    Check(WinRate(MvN) > 0.70f, "master beats novice decisively");
    Check(WinRate(MvA) > 0.55f, "master beats average");
    Check(WinRate(AvN) > 0.55f, "average beats novice");
    Check(std::fabs(WinRate(MvM) - 0.5f) < 0.20f, "mirror match is near even (no seat advantage)");

    // ------------------------------------------------------------------ [2] signals track skill
    std::printf("\n[2] do mastery signals derived from REAL play track skill?\n");
    const FMasterySignal SigMaster = DeriveSignal(MvN.TelA, ROUNDS, WinRate(MvN));
    const FMasterySignal SigNovice = DeriveSignal(MvN.TelB, ROUNDS, 1.0f - WinRate(MvN));

    std::printf("      %-22s %8s %8s\n", "channel", "master", "novice");
    std::printf("      %-22s %8.3f %8.3f\n", "Competence", SigMaster.Competence, SigNovice.Competence);
    std::printf("      %-22s %8.3f %8.3f\n", "ExecutionQuality", SigMaster.ExecutionQuality, SigNovice.ExecutionQuality);
    std::printf("      %-22s %8.3f %8.3f\n", "TimingPrecision", SigMaster.TimingPrecision, SigNovice.TimingPrecision);
    std::printf("      %-22s %8.3f %8.3f\n", "ReflexReadiness", SigMaster.ReflexReadiness, SigNovice.ReflexReadiness);
    std::printf("      %-22s %8.3f %8.3f\n", "valid channels", float(SigMaster.NumValidChannels()), float(SigNovice.NumValidChannels()));
    std::printf("      %-22s %8.3f %8.3f\n", "signal trust", SigMaster.GetSignalTrust(), SigNovice.GetSignalTrust());

    Check(SigMaster.bCompetenceValid && SigNovice.bCompetenceValid, "competence measurable for both");
    Check(SigMaster.Competence > SigNovice.Competence + 0.15f, "measured competence separates master from novice");
    Check(SigMaster.TimingPrecision > SigNovice.TimingPrecision, "punish conversion tracks skill");
    Check(SigMaster.ReflexReadiness > SigNovice.ReflexReadiness, "reaction latency tracks skill");

    // ------------------------------------------------------------------ [3] invariant on real data
    std::printf("\n[3] does 'unknown is not excellence' hold on REAL data?\n");
    // One-frame round: nothing can be measured, no matter how good the player is.
    FGameState Tiny; Tiny.Reset(); Tiny.MaxFrames = 1;
    FScriptedAgent TA(FAgentSkill::Master(), 5); FScriptedAgent TB(FAgentSkill::Master(), 6);
    while (!Tiny.IsOver())
    {
        StepFrame(Tiny, TA.ChooseAction(Tiny.A, Tiny.B, Tiny.Distance()),
                        TB.ChooseAction(Tiny.B, Tiny.A, Tiny.Distance()));
    }
    const FMasterySignal SigTiny = DeriveSignal(Tiny.TelA, 1, 1.0f);
    std::printf("      1-frame round by a MASTER: %d valid channels, trust %.3f, competence valid=%s\n",
                SigTiny.NumValidChannels(), SigTiny.GetSignalTrust(),
                SigTiny.bCompetenceValid ? "true" : "false");
    Check(!SigTiny.bCompetenceValid, "an unmeasurable round refuses to claim competence");
    Check(SigTiny.GetSignalTrust() < 0.15f, "trust stays near zero without evidence");

    // ------------------------------------------------------------------ [4] embodiment differs
    std::printf("\n[4] does the binding produce different BODIES from measured signal?\n");
    Bind::Persona Melody;
    const Bind::Pose PoseMaster = Bind::Evaluate(SigMaster, Melody);
    const Bind::Pose PoseNovice = Bind::Evaluate(SigNovice, Melody);
    const Bind::Pose PoseTiny   = Bind::Evaluate(SigTiny, Melody);

    std::printf("      %-20s %8s %8s %8s\n", "", "master", "novice", "1-frame");
    std::printf("      %-20s %8.3f %8.3f %8.3f\n", "MotionEconomy",
                PoseMaster.MotionEconomy, PoseNovice.MotionEconomy, PoseTiny.MotionEconomy);
    std::printf("      %-20s %8.3f %8.3f %8.3f\n", "GazeSteadiness",
                PoseMaster.GazeSteadiness, PoseNovice.GazeSteadiness, PoseTiny.GazeSteadiness);
    std::printf("      %-20s %8.3f %8.3f %8.3f\n", "IdleFidget",
                PoseMaster.IdleFidget, PoseNovice.IdleFidget, PoseTiny.IdleFidget);
    std::printf("      %-20s %8.3f %8.3f %8.3f\n", "ExpressionIntensity",
                PoseMaster.ExpressionIntensity, PoseNovice.ExpressionIntensity, PoseTiny.ExpressionIntensity);

    Check(PoseMaster.MotionEconomy > PoseNovice.MotionEconomy + 0.05f,
          "master carries herself with more economy of motion");
    Check(PoseMaster.IdleFidget < PoseNovice.IdleFidget,
          "master fidgets less");
    Check(PoseTiny.ExpressionIntensity <= Melody.UnknownStateIntensityCap + 0.001f,
          "the unmeasurable round is capped at the unknown-state ceiling");
    Check(PoseTiny.MotionEconomy < PoseMaster.MotionEconomy - 0.2f,
          "an unmeasured master does NOT get to look like a master");

    // ------------------------------------------------------------------ [5] determinism
    std::printf("\n[5] determinism (required for this to be a regression test)\n");
    const MatchOutcome Rep1 = RunMatch(FAgentSkill::Master(), FAgentSkill::Average(), 20, 999);
    const MatchOutcome Rep2 = RunMatch(FAgentSkill::Master(), FAgentSkill::Average(), 20, 999);
    Check(Rep1.WinsA == Rep2.WinsA && Rep1.WinsB == Rep2.WinsB &&
          Rep1.TelA.AttacksLanded == Rep2.TelA.AttacksLanded,
          "same seed reproduces identical match outcome and telemetry");

    std::printf("\n=== %s ===\n", TestsFailed == 0 ? "DuelIntegration=success" : "FAILURES PRESENT");
    return TestsFailed == 0 ? 0 : 1;
}
