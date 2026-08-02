// DuelAgent.h
// Agents whose skill is parameterized, and the telemetry -> FMasterySignal derivation.
//
// THE POINT OF THIS FILE. Until now every FMasterySignal in the test suite was hand-written:
// "assume competence 0.95, assume flow 0.7". That proves the binding rules are self-consistent
// but proves nothing about whether the contract can be SATISFIED from real play. This derives
// every channel from measurements a game actually produces - hit rate, punish conversion,
// reaction latency, whiff discipline - and hands the result to the same binding the synthetic
// tests exercise.
//
// Skill is expressed as four dials, each of which corresponds to something a human player
// visibly has or lacks:
//
//   ReactionFrames   - frames of delay before responding to what the opponent just did. At 60fps
//                      a strong player is ~12 frames (200ms); a weak one ~30 (500ms).
//   PunishAwareness  - probability of recognising and converting an opponent's recovery window.
//                      This is the single biggest skill differentiator in real fighting games.
//   SpacingSkill     - how reliably they hold the range where their attack reaches and the
//                      opponent's does not.
//   WhiffDiscipline  - probability of NOT throwing an unprovoked attack. Low discipline is what
//                      "mashing" means, and it feeds the opponent punish opportunities.

#pragma once

#include <random>
#include "DuelGame.h"
#include "../MasterySignal.h"

namespace Duel
{

struct FAgentSkill
{
    int   ReactionFrames  = 20;
    float PunishAwareness = 0.5f;
    float SpacingSkill    = 0.5f;
    float WhiffDiscipline = 0.5f;

    static FAgentSkill Novice()  { return { 32, 0.08f, 0.15f, 0.20f }; }
    static FAgentSkill Average() { return { 20, 0.45f, 0.50f, 0.55f }; }
    static FAgentSkill Master()  { return { 10, 0.92f, 0.90f, 0.90f }; }
};

/**
 * A scripted agent. Deliberately NOT a learning agent: to verify the game rewards skill, skill
 * has to be an independent variable I control, not an emergent one I hope for.
 */
class FScriptedAgent
{
public:
    FScriptedAgent(const FAgentSkill& InSkill, unsigned Seed)
        : Skill(InSkill), Rng(Seed) {}

    void Reset() { PerceivedPhase = EPhase::Neutral; PerceptionDelay = 0; PendingReaction = -1; }

    EAction ChooseAction(const FFighter& Me, const FFighter& Opponent, float Distance)
    {
        std::uniform_real_distribution<float> U(0.0f, 1.0f);

        // Perception is LAGGED: the agent acts on a delayed view of the opponent, which is what
        // makes ReactionFrames a real handicap rather than a cosmetic number.
        if (Opponent.Phase != TrueLastPhase)
        {
            TrueLastPhase = Opponent.Phase;
            PerceptionDelay = Skill.ReactionFrames;
            PendingPhase = Opponent.Phase;
        }
        if (PerceptionDelay > 0)
        {
            if (--PerceptionDelay == 0) { PerceivedPhase = PendingPhase; }
        }

        if (Me.IsCommitted()) return EAction::Wait;   // input ignored anyway

        const bool bInRange = Distance <= ATTACK_RANGE;
        const bool bOppPunishable = (PerceivedPhase == EPhase::AttackRecovery ||
                                     PerceivedPhase == EPhase::DodgeRecovery);
        const bool bOppAttacking = (PerceivedPhase == EPhase::AttackStartup ||
                                    PerceivedPhase == EPhase::AttackActive);

        // 1. PUNISH. The highest-value read in the game.
        if (bOppPunishable && bInRange && U(Rng) < Skill.PunishAwareness && Me.Stamina >= ATTACK_COST)
        {
            return EAction::Attack;
        }

        // 2. DEFEND against a telegraphed attack. Dodge if we can afford it, else block.
        if (bOppAttacking && bInRange)
        {
            if (U(Rng) < Skill.PunishAwareness * 0.7f && Me.Stamina >= DODGE_COST) return EAction::Dodge;
            if (U(Rng) < 0.6f) return EAction::Block;
        }

        // 3. SPACING. Skilled players hover just outside range and step in to strike.
        const float IdealRange = ATTACK_RANGE * 0.92f;
        if (U(Rng) < Skill.SpacingSkill)
        {
            if (Distance > IdealRange + 0.25f) return EAction::MoveToward;
            if (Distance < IdealRange - 0.45f) return EAction::MoveAway;
        }
        else if (U(Rng) < 0.35f)
        {
            return (U(Rng) < 0.5f) ? EAction::MoveToward : EAction::MoveAway;  // aimless drifting
        }

        // 4. OFFENSE. Attack in range - gated by discipline, which is what stops mashing.
        if (bInRange && Me.Stamina >= ATTACK_COST)
        {
            const float AttackChance = 0.10f + 0.25f * Skill.PunishAwareness;
            if (U(Rng) < AttackChance) return EAction::Attack;
        }
        else if (!bInRange && U(Rng) > Skill.WhiffDiscipline)
        {
            return EAction::Attack;   // undisciplined whiff from out of range
        }

        // 5. Recover stamina.
        if (Me.Stamina < 30.0f) return EAction::Wait;
        return EAction::Wait;
    }

private:
    FAgentSkill Skill;
    std::mt19937 Rng;
    EPhase PerceivedPhase = EPhase::Neutral;
    EPhase PendingPhase = EPhase::Neutral;
    EPhase TrueLastPhase = EPhase::Neutral;
    int PerceptionDelay = 0;
    int PendingReaction = -1;
};

/**
 * Derive an FMasterySignal from measured play.
 *
 * Every channel is set ONLY when the telemetry actually supports it. Channels the round could
 * not measure are left invalid rather than defaulted to a plausible-looking number - which is
 * the whole reason the contract carries validity bits. A round in which nobody ever attacked
 * genuinely does not know the player's execution quality, and must say so.
 */
inline FMasterySignal DeriveSignal(const FRoundTelemetry& Tel, int RoundsPlayed, float WinRate)
{
    FMasterySignal S;

    // --- Execution: did attacks land? Needs attacks to have been thrown. ---
    if (Tel.AttacksThrown >= 5)
    {
        S.ExecutionQuality = Tel.HitRate();
        S.bExecutionQualityValid = true;
    }

    // --- Timing precision: punish conversion. Needs punish windows to have existed. ---
    if (Tel.PunishOpportunities >= 3)
    {
        S.TimingPrecision = Tel.PunishRate();
        S.bTimingPrecisionValid = true;
    }

    // --- Reflex: mean reaction latency, normalized. 8 frames ~ elite, 36 ~ slow. ---
    if (Tel.ReactionSamples >= 3)
    {
        const float Frames = Tel.MeanReactionFrames();
        S.ReflexReadiness = std::max(0.0f, std::min(1.0f, (36.0f - Frames) / 28.0f));
        S.bReflexReadinessValid = true;
    }

    // --- Prediction: reading attacks well enough to dodge them. ---
    if (Tel.DodgesAttempted >= 3)
    {
        S.PredictionAccuracy = Tel.DodgeRate();
        S.bPredictionAccuracyValid = true;
    }

    // --- Competence: composite of what we measured, weighted by outcome. ---
    if (RoundsPlayed >= 3)
    {
        float Sum = 0.0f; int N = 0;
        if (S.bExecutionQualityValid) { Sum += S.ExecutionQuality; ++N; }
        if (S.bTimingPrecisionValid)  { Sum += S.TimingPrecision;  ++N; }
        if (S.bReflexReadinessValid)  { Sum += S.ReflexReadiness;  ++N; }
        if (N > 0)
        {
            S.Competence = 0.6f * (Sum / N) + 0.4f * WinRate;
            S.bCompetenceValid = true;
            S.Tier = S.Competence;
            S.bTierValid = true;
        }
    }

    // --- Challenge/skill balance from the damage exchange. 0.5 = even. ---
    const float TotalDamage = Tel.DamageDealt + Tel.DamageTaken;
    if (TotalDamage > 20.0f)
    {
        S.ChallengeSkillBalance = std::max(0.0f, std::min(1.0f, Tel.DamageTaken / TotalDamage));
        S.bChallengeSkillBalanceValid = true;

        // Frustration: losing the exchange while also whiffing is the signature of tilt.
        const float Losing = std::max(0.0f, S.ChallengeSkillBalance - 0.5f) * 2.0f;
        S.Frustration = std::max(0.0f, std::min(1.0f, Losing * (0.5f + 0.5f * Tel.WhiffRate())));
        S.bFrustrationValid = true;

        S.Confidence = 1.0f - S.ChallengeSkillBalance;
        S.bConfidenceValid = true;
    }

    // --- Flow: winning the exchange cleanly, with few whiffs, while engaged. ---
    if (Tel.TotalFrames > 300 && Tel.AttacksThrown >= 5)
    {
        const float Engagement = float(Tel.FramesInRange) / float(Tel.TotalFrames);
        const float Cleanliness = 1.0f - Tel.WhiffRate();
        const float Winning = S.bChallengeSkillBalanceValid ? (1.0f - S.ChallengeSkillBalance) : 0.5f;
        S.FlowIntensity = std::max(0.0f, std::min(1.0f, Engagement * Cleanliness * Winning * 1.4f));
        S.bFlowIntensityValid = true;
    }

    // --- Arousal from engagement density. ---
    if (Tel.TotalFrames > 0)
    {
        S.Arousal = std::max(0.0f, std::min(1.0f, float(Tel.FramesInRange) / float(Tel.TotalFrames) * 1.3f));
        S.bArousalValid = true;
    }

    // Provenance: attacks are the countable unit of evidence here.
    S.EvidenceCount = Tel.AttacksThrown + Tel.PunishOpportunities;
    S.TimeSinceLastEvidence = 0.0f;

    return S;
}

} // namespace Duel
