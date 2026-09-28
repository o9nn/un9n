// StandaloneDuelBody.cpp
// Does the fight reach the body, and does the body then show who is better at fighting?
//
// Closes the loop the body backend left open: FDriver::React() existed, and nothing outside the
// tests called it, so MotionEconomy and ReactionSharpness - the two channels that only exist in
// how a body answers a cue - were never exercised by anything real. DuelBodyCues.h now feeds duel
// events to React(). This harness checks the whole chain on actual play:
//
//     duel frames -> telemetry -> FMasterySignal -> real binding -> pose
//                 -> frame events -> React() cues  ----------------->  body driver
//
// Two comparisons, because each alone can mislead:
//
//   IN MATCH   - each fighter's body driven by her own events. Realistic, but confounded: the
//                novice gets hit far more, so she would move more even with identical poses.
//   CONTROLLED - both poses driven by the SAME event stream. Any difference is the pose's, and
//                therefore the measurement's - the fight supplies the cues, never the skill.
//
// Overcorrection is measured per target segment rather than as "peak minus target", because in
// live play cues arrive back to back and a single held target rarely exists. It is reported as a
// fraction of the distance travelled, so a big recoil and a small lean are comparable.
//
// Onset is measured from each CUE to the frame the body begins answering it. The wind-up happens
// before the target moves - that is what a wind-up is - so timing from the target change would
// measure only spring speed. The first version of this harness did that, and it also hid a driver
// bug: React() restarted a single shared timer on every cue, so in a fight a slow character's
// wind-ups were reset before they finished and dense cues went unanswered. Now each cue is
// matched to its own firing, and an unanswered or dropped cue fails the test.
//
// Build & run:
//   g++ -std=c++17 -O2 -I StandaloneShim -o duelbody StandaloneDuelBody.cpp

#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

#include "../MasteryEmbodimentBinding.cpp"      // real rules, single translation unit
#include "../Personas/MelodyPersona.h"
#include "../Simulation/DuelAgent.h"
#include "../Simulation/DuelBodyCues.h"

namespace
{

using namespace Duel;

int TestsFailed = 0;

void Check(bool bCond, const std::string& Label)
{
    std::printf("  [%s] %s\n", bCond ? "PASS" : "FAIL", Label.c_str());
    if (!bCond) ++TestsFailed;
}

constexpr int ROUNDS = 60;
constexpr unsigned SEED = 1234;

/** Every frame of a round: both fighters' events. Recorded once, replayable into any body. */
struct FRecordedRound
{
    std::vector<FFrameEvents> A, B;
};

struct FMatch
{
    int WinsA = 0;
    FRoundTelemetry TelA, TelB;
    std::vector<FRecordedRound> Rounds;
    // Event-flag totals, to cross-check the instrumentation against telemetry.
    int StartedA = 0, LandedA = 0, WhiffedA = 0, DodgedB = 0, PunishedB = 0;
};

void Accumulate(FRoundTelemetry& D, const FRoundTelemetry& S)
{
    D.AttacksThrown += S.AttacksThrown;   D.AttacksLanded += S.AttacksLanded;
    D.AttacksWhiffed += S.AttacksWhiffed; D.PunishOpportunities += S.PunishOpportunities;
    D.PunishesConverted += S.PunishesConverted; D.TimesPunished += S.TimesPunished;
    D.BlocksAttempted += S.BlocksAttempted; D.DodgesAttempted += S.DodgesAttempted;
    D.DodgesSuccessful += S.DodgesSuccessful; D.FramesInRange += S.FramesInRange;
    D.TotalFrames += S.TotalFrames; D.DamageDealt += S.DamageDealt; D.DamageTaken += S.DamageTaken;
    D.ReactionSamples += S.ReactionSamples; D.ReactionFrameSum += S.ReactionFrameSum;
}

/** Same loop as StandaloneDuelSimulation::RunMatch, plus recording each frame's events. */
FMatch Play(const FAgentSkill& SkillA, const FAgentSkill& SkillB)
{
    FMatch M;
    for (int r = 0; r < ROUNDS; ++r)
    {
        FGameState S;
        S.Reset();
        FScriptedAgent A(SkillA, SEED + r * 31u);
        FScriptedAgent B(SkillB, SEED + r * 31u + 7919u);
        A.Reset(); B.Reset();
        S.TelA.ReactionSamples = 1; S.TelA.ReactionFrameSum = SkillA.ReactionFrames;
        S.TelB.ReactionSamples = 1; S.TelB.ReactionFrameSum = SkillB.ReactionFrames;

        FRecordedRound Rec;
        while (!S.IsOver())
        {
            const float D = S.Distance();
            const EAction ActA = A.ChooseAction(S.A, S.B, D);
            const EAction ActB = B.ChooseAction(S.B, S.A, D);
            StepFrame(S, ActA, ActB);
            Rec.A.push_back(S.EventsA);
            Rec.B.push_back(S.EventsB);
            M.StartedA += S.EventsA.bAttackStarted;
            M.LandedA += S.EventsA.bLandedHit;
            M.WhiffedA += S.EventsA.bWhiffed;
            M.DodgedB += S.EventsB.bDodgedHit;
            M.PunishedB += S.EventsB.bWasPunished;
        }
        if (S.Winner() > 0) ++M.WinsA;
        Accumulate(M.TelA, S.TelA);
        Accumulate(M.TelB, S.TelB);
        M.Rounds.push_back(Rec);
    }
    return M;
}

/** How a body answered a stream of events. */
struct FBodyStats
{
    int Cues = 0;
    int Segments = 0;
    float MeanOvershoot = 0.0f;      // per target change, distance past the target / distance travelled
    float MeanOnsetFrames = 0.0f;    // per cue, frames from the cue until the body begins to answer it
    int OnsetSamples = 0;
    int Dropped = 0;
};

FBodyStats Drive(const std::vector<FRecordedRound>& Rounds, bool bSideA,
                 const FMasteryEmbodimentPose& Pose)
{
    FBodyStats St;
    double OvershootSum = 0.0, OnsetSum = 0.0;

    for (size_t r = 0; r < Rounds.size(); ++r)
    {
        DuelBodyCues::FDuelBody Body(0xB0D1u + (uint32)r);
        const std::vector<FFrameEvents>& Ev = bSideA ? Rounds[r].A : Rounds[r].B;

        float Target = 0.0f, Start = 0.0f, Peak = 0.0f;
        int Age = 0;
        std::vector<int> CueFrames;   // FIFO: equal timers for a fixed pose fire in arrival order
        size_t NextToFire = 0;
        bool bOpen = false;

        auto Close = [&]() {
            if (!bOpen) return;
            ++St.Segments;
            OvershootSum += Peak / std::fabs(Target - Start);
        };

        // Half a second past the final frame. The last cues of a round - the KO blow above all -
        // arrive after the game has stopped, and a body does not freeze when the round does: she
        // still recoils from the hit that ended it. Without the tail those cues are cut off
        // mid-wind-up and read as unanswered.
        const size_t Tail = FPS / 2;
        const FFrameEvents Quiet;
        for (size_t f = 0; f < Ev.size() + Tail; ++f)
        {
            const FFrameEvents& E = (f < Ev.size()) ? Ev[f] : Quiet;
            if (DuelBodyCues::CueFor(E) != 0.0f) CueFrames.push_back((int)f);
            const int32 FiredBefore = Body.Driver.ReactionsFired();
            const FBodyMotionFrame& F = Body.Step(E, Pose);
            for (int32 k = FiredBefore; k < Body.Driver.ReactionsFired(); ++k, ++NextToFire)
            {
                OnsetSum += (int)f - CueFrames[NextToFire];
                ++St.OnsetSamples;
            }
            if (!bOpen || std::fabs(F.PostureTarget - Target) > 1e-4f)
            {
                Close();
                bOpen = true;
                Target = F.PostureTarget;
                Start = F.PostureSpring;
                Peak = 0.0f; Age = 0;
                // Only segments that ask for real travel are informative.
                if (std::fabs(Target - Start) < 1.0f) bOpen = false;
                continue;
            }
            ++Age;
            const float Dir = (Target > Start) ? 1.0f : -1.0f;
            const float Past = (F.PostureSpring - Target) * Dir;   // > 0 means beyond the target
            if (Past > Peak) Peak = Past;
        }
        Close();
        St.Cues += Body.CuesSent;
        St.Dropped += Body.Driver.ReactionsDropped();
    }
    St.MeanOvershoot = St.Segments ? (float)(OvershootSum / St.Segments) : 0.0f;
    St.MeanOnsetFrames = St.OnsetSamples ? (float)(OnsetSum / St.OnsetSamples) : 0.0f;
    return St;
}

void Print(const char* Label, const FBodyStats& S)
{
    std::printf("    %-26s cues=%5d answered=%5d dropped=%d overshoot=%5.1f%% wind-up=%4.1f frames (%3.0f ms)\n",
                Label, S.Cues, S.OnsetSamples, S.Dropped, S.MeanOvershoot * 100.0f, S.MeanOnsetFrames,
                S.MeanOnsetFrames * 1000.0f / FPS);
}

} // namespace

int main()
{
    std::printf("=== Duel -> body: does the fight reach the body, and does skill show in it? ===\n\n");

    const FMatch M = Play(FAgentSkill::Master(), FAgentSkill::Novice());
    const float WinRate = float(M.WinsA) / ROUNDS;

    // ---- 1. The instrumentation reports what the telemetry counts --------------------------
    std::printf("--- 1. frame events agree with telemetry ---\n");
    std::printf("    started %d/%d  landed %d/%d  whiffed %d/%d  dodged %d/%d  punished %d/%d\n",
                M.StartedA, M.TelA.AttacksThrown, M.LandedA, M.TelA.AttacksLanded,
                M.WhiffedA, M.TelA.AttacksWhiffed, M.DodgedB, M.TelB.DodgesSuccessful,
                M.PunishedB, M.TelB.TimesPunished);
    Check(M.StartedA == M.TelA.AttacksThrown && M.LandedA == M.TelA.AttacksLanded &&
          M.WhiffedA == M.TelA.AttacksWhiffed && M.DodgedB == M.TelB.DodgesSuccessful &&
          M.PunishedB == M.TelB.TimesPunished,
          "every event flag totals exactly to its telemetry counter");

    // The existing duel harness asserts the master wins >70%; the events must not have changed play.
    std::printf("    master win rate %.2f\n", WinRate);
    Check(WinRate > 0.70f, "instrumenting the game did not change it - master still wins decisively");

    // ---- 2. Poses from measured play, through the real binding -----------------------------
    const FMasteryPersonaProfile Melody = MelodyPersona::Profile();
    const FMasteryEmbodimentPose PM =
        MasteryEmbodimentBinding::Evaluate(DeriveSignal(M.TelA, ROUNDS, WinRate), Melody);
    const FMasteryEmbodimentPose PN =
        MasteryEmbodimentBinding::Evaluate(DeriveSignal(M.TelB, ROUNDS, 1.0f - WinRate), Melody);
    std::printf("\n--- 2. poses from measured play ---\n");
    std::printf("    master: economy=%.2f sharpness=%.2f intensity=%.2f\n",
                PM.MotionEconomy, PM.ReactionSharpness, PM.ExpressionIntensity);
    std::printf("    novice: economy=%.2f sharpness=%.2f intensity=%.2f\n",
                PN.MotionEconomy, PN.ReactionSharpness, PN.ExpressionIntensity);

    // ---- 3. In match: each body driven by its own fight ------------------------------------
    std::printf("\n--- 3. in match (each fighter's own events) ---\n");
    const FBodyStats InM = Drive(M.Rounds, true,  PM);
    const FBodyStats InN = Drive(M.Rounds, false, PN);
    Print("master body, master events", InM);
    Print("novice body, novice events", InN);
    Check(InM.Cues > 0 && InN.Cues > 0, "the fight reaches both bodies as React() cues");
    Check(InM.Dropped == 0 && InN.Dropped == 0 && InM.OnsetSamples == InM.Cues && InN.OnsetSamples == InN.Cues,
          "every cue is answered - none reset, none dropped, none left pending");
    Check(InM.MeanOvershoot < InN.MeanOvershoot, "in play, the master overcorrects less");
    Check(InM.MeanOnsetFrames < InN.MeanOnsetFrames, "in play, the master winds up less");

    // ---- 4. Controlled: identical cues, different poses ------------------------------------
    std::printf("\n--- 4. controlled (both bodies answer the NOVICE's events) ---\n");
    const FBodyStats CtlM = Drive(M.Rounds, false, PM);
    const FBodyStats CtlN = InN;
    Print("master body, novice events", CtlM);
    Print("novice body, novice events", CtlN);
    Check(CtlM.Cues == CtlN.Cues, "identical event stream -> identical cue count (cues carry no skill)");
    Check(CtlM.MeanOvershoot < 0.5f * CtlN.MeanOvershoot,
          "same hits, same cues: the master's body overshoots under half as far");
    Check(CtlM.MeanOnsetFrames + 2.0f < CtlN.MeanOnsetFrames,
          "same hits, same cues: the master's body starts answering at least 2 frames sooner");

    // ---- 5. Determinism ------------------------------------------------------------------------
    std::printf("\n--- 5. determinism ---\n");
    {
        const FBodyStats Again = Drive(M.Rounds, false, PN);
        Check(Again.Cues == InN.Cues && Again.MeanOvershoot == InN.MeanOvershoot &&
              Again.MeanOnsetFrames == InN.MeanOnsetFrames,
              "replaying the same fight gives a bit-identical body");
    }

    std::printf("\n=== %s ===\n", TestsFailed == 0 ? "DuelBody=success" : "FAILURES PRESENT");
    return TestsFailed == 0 ? 0 : 1;
}
