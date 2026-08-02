// DuelGame.h
// A frame-accurate 1v1 duel, built to be the smallest game with REAL skill depth.
//
// The RPS tournament proved the counter-adaptive policy is sound, but a matrix game has no
// space, no timing and no commitment - the three things actual gameplay skill is made of. This
// adds all three while staying headless and deterministic:
//
//   SPACE       - fighters occupy a 1D line; attacks only connect inside a range, so positioning
//                 is a decision rather than a formality.
//   TIMING      - every attack has startup / active / recovery frames. You are COMMITTED once you
//                 press: a whiffed attack leaves you helpless for its recovery, which the
//                 opponent can punish. This is what makes reaction latency matter.
//   RESOURCES   - stamina gates blocking and dodging, so turtling is not a free strategy.
//
// Those mechanics are what let mastery be *measured* rather than declared: hit rate, punish
// conversion, reaction latency and whiff rate all fall out of play, and map directly onto
// FMasterySignal channels. No engine types - runs under plain g++.

#pragma once

#include <algorithm>
#include <cmath>
#include <vector>

namespace Duel
{

constexpr int   FPS               = 60;
constexpr float ARENA_MIN         = 0.0f;
constexpr float ARENA_MAX         = 10.0f;
constexpr float MOVE_SPEED        = 0.14f;   // units per frame
constexpr float ATTACK_RANGE      = 1.6f;
constexpr float ATTACK_DAMAGE     = 10.0f;
constexpr float BLOCK_DAMAGE_MULT = 0.25f;
constexpr float MAX_HEALTH        = 100.0f;
constexpr float MAX_STAMINA       = 100.0f;

// Frame data. Recovery > startup is what creates the punish game.
constexpr int ATTACK_STARTUP  = 6;
constexpr int ATTACK_ACTIVE   = 3;
constexpr int ATTACK_RECOVERY = 14;
constexpr int DODGE_STARTUP   = 2;
constexpr int DODGE_INVULN    = 6;
constexpr int DODGE_RECOVERY  = 9;

constexpr float STAMINA_REGEN    = 0.35f;
constexpr float BLOCK_DRAIN      = 0.55f;
constexpr float DODGE_COST       = 14.0f;
constexpr float ATTACK_COST      = 7.0f;

enum class EAction : int
{
    Wait = 0,
    MoveToward,
    MoveAway,
    Attack,
    Block,
    Dodge,
    COUNT
};

enum class EPhase : int
{
    Neutral = 0,      // free to act
    AttackStartup,    // committed, not yet dangerous
    AttackActive,     // hitbox live
    AttackRecovery,   // helpless - PUNISHABLE
    DodgeStartup,
    DodgeInvuln,
    DodgeRecovery
};

struct FFighter
{
    float Position = 0.0f;
    float Health = MAX_HEALTH;
    float Stamina = MAX_STAMINA;
    EPhase Phase = EPhase::Neutral;
    int PhaseFrames = 0;      // frames remaining in current phase
    bool bBlocking = false;
    bool bAttackConnected = false;  // did the current attack land?

    bool IsNeutral() const { return Phase == EPhase::Neutral; }
    bool IsPunishable() const { return Phase == EPhase::AttackRecovery || Phase == EPhase::DodgeRecovery; }
    bool IsInvulnerable() const { return Phase == EPhase::DodgeInvuln; }
    bool IsCommitted() const { return Phase != EPhase::Neutral; }
};

/** Per-round telemetry. These are the raw measurements mastery signals are derived from. */
struct FRoundTelemetry
{
    int AttacksThrown = 0;
    int AttacksLanded = 0;
    int AttacksWhiffed = 0;      // finished active frames without connecting
    int PunishOpportunities = 0; // frames where opponent was punishable and we were in range
    int PunishesConverted = 0;   // attacks started during such a window that landed
    int TimesPunished = 0;       // we got hit while punishable
    int BlocksAttempted = 0;
    int DodgesAttempted = 0;
    int DodgesSuccessful = 0;    // invuln frames that actually avoided an active hitbox
    int FramesInRange = 0;
    int TotalFrames = 0;
    float DamageDealt = 0.0f;
    float DamageTaken = 0.0f;

    /** Sum of reaction latencies (frames) and their count, for mean reaction time. */
    int ReactionSamples = 0;
    int ReactionFrameSum = 0;

    float HitRate() const { return AttacksThrown > 0 ? float(AttacksLanded) / AttacksThrown : 0.0f; }
    float WhiffRate() const { return AttacksThrown > 0 ? float(AttacksWhiffed) / AttacksThrown : 0.0f; }
    float PunishRate() const { return PunishOpportunities > 0 ? float(PunishesConverted) / PunishOpportunities : 0.0f; }
    float DodgeRate() const { return DodgesAttempted > 0 ? float(DodgesSuccessful) / DodgesAttempted : 0.0f; }
    float MeanReactionFrames() const { return ReactionSamples > 0 ? float(ReactionFrameSum) / ReactionSamples : 0.0f; }
};

struct FGameState
{
    FFighter A;
    FFighter B;
    int Frame = 0;
    int MaxFrames = FPS * 60;   // 60 second rounds
    FRoundTelemetry TelA;
    FRoundTelemetry TelB;

    float Distance() const { return std::fabs(A.Position - B.Position); }
    bool IsOver() const { return A.Health <= 0.0f || B.Health <= 0.0f || Frame >= MaxFrames; }

    /** +1 A wins, -1 B wins, 0 draw/timeout. */
    int Winner() const
    {
        if (B.Health <= 0.0f && A.Health > 0.0f) return 1;
        if (A.Health <= 0.0f && B.Health > 0.0f) return -1;
        if (A.Health > B.Health) return 1;
        if (B.Health > A.Health) return -1;
        return 0;
    }

    void Reset()
    {
        A = FFighter(); B = FFighter();
        A.Position = 3.5f; B.Position = 6.5f;
        Frame = 0;
        TelA = FRoundTelemetry(); TelB = FRoundTelemetry();
    }
};

namespace Detail
{
    inline void BeginAction(FFighter& F, EAction Act, FRoundTelemetry& Tel)
    {
        F.bBlocking = false;
        if (!F.IsNeutral()) return;   // committed - input ignored

        switch (Act)
        {
        case EAction::Attack:
            if (F.Stamina >= ATTACK_COST)
            {
                F.Stamina -= ATTACK_COST;
                F.Phase = EPhase::AttackStartup;
                F.PhaseFrames = ATTACK_STARTUP;
                F.bAttackConnected = false;
                ++Tel.AttacksThrown;
            }
            break;
        case EAction::Dodge:
            if (F.Stamina >= DODGE_COST)
            {
                F.Stamina -= DODGE_COST;
                F.Phase = EPhase::DodgeStartup;
                F.PhaseFrames = DODGE_STARTUP;
                ++Tel.DodgesAttempted;
            }
            break;
        case EAction::Block:
            if (F.Stamina > 0.0f) { F.bBlocking = true; ++Tel.BlocksAttempted; }
            break;
        default:
            break;
        }
    }

    inline void ApplyMovement(FFighter& F, EAction Act, float OtherPos)
    {
        if (F.IsCommitted()) return;   // no movement while committed
        const float Dir = (OtherPos > F.Position) ? 1.0f : -1.0f;
        if (Act == EAction::MoveToward) F.Position += MOVE_SPEED * Dir;
        else if (Act == EAction::MoveAway) F.Position -= MOVE_SPEED * Dir;
        F.Position = std::max(ARENA_MIN, std::min(ARENA_MAX, F.Position));
    }

    /** Advance phase timers; returns true on the frame an attack's active window just ended. */
    inline bool AdvancePhase(FFighter& F)
    {
        if (F.Phase == EPhase::Neutral) return false;
        if (--F.PhaseFrames > 0) return false;

        switch (F.Phase)
        {
        case EPhase::AttackStartup:
            F.Phase = EPhase::AttackActive; F.PhaseFrames = ATTACK_ACTIVE; return false;
        case EPhase::AttackActive:
            F.Phase = EPhase::AttackRecovery; F.PhaseFrames = ATTACK_RECOVERY; return true;
        case EPhase::DodgeStartup:
            F.Phase = EPhase::DodgeInvuln; F.PhaseFrames = DODGE_INVULN; return false;
        case EPhase::DodgeInvuln:
            F.Phase = EPhase::DodgeRecovery; F.PhaseFrames = DODGE_RECOVERY; return false;
        default:
            F.Phase = EPhase::Neutral; F.PhaseFrames = 0; return false;
        }
    }

    inline void ResolveHit(FGameState& S, FFighter& Attacker, FFighter& Defender,
                           FRoundTelemetry& AtkTel, FRoundTelemetry& DefTel,
                           bool bDefenderWasPunishable)
    {
        if (Attacker.Phase != EPhase::AttackActive || Attacker.bAttackConnected) return;
        if (S.Distance() > ATTACK_RANGE) return;

        if (Defender.IsInvulnerable())
        {
            ++DefTel.DodgesSuccessful;
            return;   // dodged cleanly - attack passes through, still whiffs
        }

        float Damage = ATTACK_DAMAGE;
        if (Defender.bBlocking && Defender.Stamina > 0.0f)
        {
            Damage *= BLOCK_DAMAGE_MULT;
        }

        Defender.Health -= Damage;
        Attacker.bAttackConnected = true;
        ++AtkTel.AttacksLanded;
        AtkTel.DamageDealt += Damage;
        DefTel.DamageTaken += Damage;

        if (bDefenderWasPunishable) { ++AtkTel.PunishesConverted; ++DefTel.TimesPunished; }
    }
}

/**
 * Advance one frame. Both fighters act simultaneously - there is no turn order, which is what
 * makes reads and commitment meaningful.
 */
inline void StepFrame(FGameState& S, EAction ActA, EAction ActB)
{
    const bool bBPunishableAtInput = S.B.IsPunishable();
    const bool bAPunishableAtInput = S.A.IsPunishable();
    const bool bInRange = S.Distance() <= ATTACK_RANGE;

    // Count punish opportunities: opponent helpless AND we are in range AND free to act.
    if (bBPunishableAtInput && bInRange && S.A.IsNeutral()) ++S.TelA.PunishOpportunities;
    if (bAPunishableAtInput && bInRange && S.B.IsNeutral()) ++S.TelB.PunishOpportunities;

    Detail::BeginAction(S.A, ActA, S.TelA);
    Detail::BeginAction(S.B, ActB, S.TelB);

    Detail::ApplyMovement(S.A, ActA, S.B.Position);
    Detail::ApplyMovement(S.B, ActB, S.A.Position);

    // Hits resolve before phases advance, so active frames are honoured exactly.
    Detail::ResolveHit(S, S.A, S.B, S.TelA, S.TelB, bBPunishableAtInput);
    Detail::ResolveHit(S, S.B, S.A, S.TelB, S.TelA, bAPunishableAtInput);

    if (Detail::AdvancePhase(S.A) && !S.A.bAttackConnected) ++S.TelA.AttacksWhiffed;
    if (Detail::AdvancePhase(S.B) && !S.B.bAttackConnected) ++S.TelB.AttacksWhiffed;

    // Stamina economy.
    S.A.Stamina += S.A.bBlocking ? -BLOCK_DRAIN : STAMINA_REGEN;
    S.B.Stamina += S.B.bBlocking ? -BLOCK_DRAIN : STAMINA_REGEN;
    S.A.Stamina = std::max(0.0f, std::min(MAX_STAMINA, S.A.Stamina));
    S.B.Stamina = std::max(0.0f, std::min(MAX_STAMINA, S.B.Stamina));

    if (bInRange) { ++S.TelA.FramesInRange; ++S.TelB.FramesInRange; }
    ++S.TelA.TotalFrames; ++S.TelB.TotalFrames;
    ++S.Frame;
}

} // namespace Duel
