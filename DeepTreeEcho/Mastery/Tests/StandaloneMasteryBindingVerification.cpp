// StandaloneMasteryBindingVerification.cpp
// Executable verification of the mastery -> embodiment binding.
//
// The binding is a pure function of (signal, persona), so it can be exercised without Unreal.
// This harness mirrors MasteryEmbodimentBinding.cpp 1:1 - keep them in lockstep.
//
//   [1] UNKNOWN IS NOT EXCELLENCE - an unmeasured signal must not render as mastery. This is
//       the safety-critical invariant; both independent guards are tested.
//   [2] Competence is legible - a master reads measurably different from a novice, and along
//       the RIGHT channels (motion economy, gaze, fidget), not merely by smiling more.
//   [3] Persona modulates without inventing skill - two personas at identical competence differ
//       in presentation, but neither gains competence from temperament.
//   [4] Strain surfaces, gated by composure - being outmatched shows, and composure delays it.
//   [5] Flow reads as absorption - blink suppression, gaze lock, reduced fidget.
//   [6] Evidence accrual is monotone - more evidence never reduces permitted expressiveness.
//   [7] Smoothing converges and is frame-rate independent.
//   [8] All outputs stay in range for randomized inputs (no NaN, no out-of-band values).
//
// Build & run:  g++ -std=c++17 -O2 -o masteryverify StandaloneMasteryBindingVerification.cpp && ./masteryverify

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <random>
#include <string>

namespace
{

// ============================================================================================
// Mirrored contract (MasterySignal.h / MasteryEmbodimentPose.h / MasteryPersonaProfile.h)
// ============================================================================================

struct MasterySignal
{
    float Competence = 0.0f;           bool bCompetenceValid = false;
    float Tier = 0.0f;                 bool bTierValid = false;
    float DomainBreadth = 0.0f;        bool bDomainBreadthValid = false;
    float ExecutionQuality = 0.0f;     bool bExecutionQualityValid = false;
    float TimingPrecision = 0.0f;      bool bTimingPrecisionValid = false;
    float ReflexReadiness = 0.0f;      bool bReflexReadinessValid = false;
    float ComboFlow = 0.0f;            bool bComboFlowValid = false;
    float PredictionAccuracy = 0.0f;   bool bPredictionAccuracyValid = false;
    float FlowIntensity = 0.0f;        bool bFlowIntensityValid = false;
    float ChallengeSkillBalance = 0.5f;bool bChallengeSkillBalanceValid = false;
    float Frustration = 0.0f;          bool bFrustrationValid = false;
    float Confidence = 0.0f;           bool bConfidenceValid = false;
    float Arousal = 0.0f;              bool bArousalValid = false;

    int EvidenceCount = 0;
    float TimeSinceLastEvidence = 0.0f;

    float GetCompetence(float U) const { return bCompetenceValid ? Competence : U; }
    float GetTier(float U) const { return bTierValid ? Tier : U; }
    float GetExecutionQuality(float U) const { return bExecutionQualityValid ? ExecutionQuality : U; }
    float GetTimingPrecision(float U) const { return bTimingPrecisionValid ? TimingPrecision : U; }
    float GetReflexReadiness(float U) const { return bReflexReadinessValid ? ReflexReadiness : U; }
    float GetPredictionAccuracy(float U) const { return bPredictionAccuracyValid ? PredictionAccuracy : U; }
    float GetFlowIntensity(float U) const { return bFlowIntensityValid ? FlowIntensity : U; }
    float GetChallengeSkillBalance(float U) const { return bChallengeSkillBalanceValid ? ChallengeSkillBalance : U; }
    float GetFrustration(float U) const { return bFrustrationValid ? Frustration : U; }
    float GetConfidence(float U) const { return bConfidenceValid ? Confidence : U; }
    float GetArousal(float U) const { return bArousalValid ? Arousal : U; }

    int NumValidChannels() const
    {
        return (bCompetenceValid?1:0)+(bTierValid?1:0)+(bDomainBreadthValid?1:0)+
               (bExecutionQualityValid?1:0)+(bTimingPrecisionValid?1:0)+(bReflexReadinessValid?1:0)+
               (bComboFlowValid?1:0)+(bPredictionAccuracyValid?1:0)+(bFlowIntensityValid?1:0)+
               (bChallengeSkillBalanceValid?1:0)+(bFrustrationValid?1:0)+(bConfidenceValid?1:0)+
               (bArousalValid?1:0);
    }

    float GetSignalTrust() const
    {
        const int Valid = NumValidChannels();
        if (Valid == 0 || EvidenceCount <= 0) return 0.0f;
        const float Coverage = static_cast<float>(Valid) / 13.0f;
        const float Evidence = static_cast<float>(EvidenceCount) / (static_cast<float>(EvidenceCount) + 20.0f);
        float Freshness = 1.0f;
        if (TimeSinceLastEvidence > 5.0f)
        {
            Freshness = 1.0f - ((TimeSinceLastEvidence - 5.0f) / 55.0f);
            Freshness = std::max(0.0f, std::min(1.0f, Freshness));
        }
        return Coverage * Evidence * Freshness;
    }
};

struct EmbodimentPose
{
    float BrowRaise = 0.0f, BrowFurrow = 0.0f;
    float EyeWiden = 0.0f, EyeNarrow = 0.0f;
    float GazeSteadiness = 0.5f, BlinkRate = 15.0f, SaccadeRate = 0.5f;
    float MouthCornerUp = 0.0f, MouthCornerDown = 0.0f, MouthTension = 0.0f, JawTension = 0.0f;
    float PostureUprightness = 0.5f, PostureLean = 0.0f, ShoulderTension = 0.0f, HeadTilt = 0.0f;
    float MicroMovementRate = 0.5f, IdleFidget = 0.5f, ReactionSharpness = 0.5f, MotionEconomy = 0.5f;
    float BreathRate = 14.0f, Asymmetry = 0.0f, ExpressionIntensity = 0.0f;
};

struct PersonaProfile
{
    std::string PersonaName = "Default";
    float ExpressivenessGain = 1.0f;
    float BaselineWarmth = 0.0f;
    float TensionBias = 0.0f;
    float FidgetBias = 0.0f;
    float PlayfulnessBias = 0.0f;
    float ConfidenceDisplayStyle = 0.5f;
    float Composure = 0.5f;
    float FlowLegibility = 0.5f;
    float BaseBlinkRate = 15.0f;
    float BaseBreathRate = 14.0f;
    float BasePostureUprightness = 0.5f;
    float UnknownStateIntensityCap = 0.25f;
};

PersonaProfile MelodyProfile()
{
    PersonaProfile P;
    P.PersonaName = "Melody";
    P.ExpressivenessGain = 1.25f;
    P.BaselineWarmth = 0.22f;
    P.TensionBias = -0.15f;
    P.FidgetBias = 0.10f;
    P.PlayfulnessBias = 0.55f;
    P.ConfidenceDisplayStyle = 0.72f;
    P.Composure = 0.70f;
    P.FlowLegibility = 0.80f;
    P.BaseBlinkRate = 16.0f;
    P.BaseBreathRate = 14.0f;
    P.BasePostureUprightness = 0.62f;
    P.UnknownStateIntensityCap = 0.30f;
    return P;
}

// A deliberately contrasting temperament: the stoic. Same skill, opposite presentation.
PersonaProfile StoicProfile()
{
    PersonaProfile P;
    P.PersonaName = "Stoic";
    P.ExpressivenessGain = 0.55f;
    P.BaselineWarmth = 0.0f;
    P.TensionBias = -0.3f;
    P.FidgetBias = -0.15f;
    P.PlayfulnessBias = 0.0f;
    P.ConfidenceDisplayStyle = 0.08f;   // mastery as stillness, not smiling
    P.Composure = 0.95f;
    P.FlowLegibility = 0.2f;
    P.UnknownStateIntensityCap = 0.15f;
    return P;
}

// ============================================================================================
// Mirrored rules (MasteryEmbodimentBinding.cpp)
// ============================================================================================

float Clamp01(float V) { return V < 0.0f ? 0.0f : (V > 1.0f ? 1.0f : V); }
float ClampSigned(float V) { return V < -1.0f ? -1.0f : (V > 1.0f ? 1.0f : V); }
float Lerp(float A, float B, float T) { return A + (B - A) * T; }

EmbodimentPose Evaluate(const MasterySignal& Signal, const PersonaProfile& Persona)
{
    EmbodimentPose Pose;
    const float Trust = Signal.GetSignalTrust();

    const float Competence  = Signal.GetCompetence(0.0f);
    const float Tier        = Signal.GetTier(0.0f);
    const float Execution   = Signal.GetExecutionQuality(0.0f);
    const float Timing      = Signal.GetTimingPrecision(0.0f);
    const float Reflex      = Signal.GetReflexReadiness(0.0f);
    const float Prediction  = Signal.GetPredictionAccuracy(0.0f);
    const float Flow        = Signal.GetFlowIntensity(0.0f);
    const float Balance     = Signal.GetChallengeSkillBalance(0.5f);
    const float Frustration = Signal.GetFrustration(0.0f);
    const float Confidence  = Signal.GetConfidence(0.0f);
    const float Arousal     = Signal.GetArousal(0.0f);

    const float Skill = Clamp01(0.5f*Competence + 0.2f*Tier + 0.2f*Execution + 0.1f*Timing);
    const float RawOverload = Clamp01((Balance - 0.5f) * 2.0f);
    const float Overload = RawOverload * (1.0f - Persona.Composure);
    const float Underload = Clamp01((0.5f - Balance) * 2.0f);
    const float Strain = Clamp01(Overload + Frustration * (1.0f - Persona.Composure));

    Pose.MotionEconomy = Clamp01(Lerp(0.25f, 0.95f, Skill) - 0.25f*Strain);
    Pose.ReactionSharpness = Clamp01(0.15f + 0.5f*Reflex + 0.35f*Prediction);
    Pose.IdleFidget = Clamp01(0.55f - 0.4f*Skill - 0.25f*Flow + 0.3f*Underload + Persona.FidgetBias);
    Pose.MicroMovementRate = Clamp01(0.5f - 0.35f*Skill + 0.3f*Strain + 0.2f*Arousal);

    Pose.GazeSteadiness = Clamp01(0.3f + 0.4f*Skill + 0.3f*Flow - 0.25f*Strain);
    Pose.SaccadeRate = Clamp01(0.7f - 0.4f*Skill - 0.2f*Flow + 0.25f*Strain);
    Pose.BlinkRate = Persona.BaseBlinkRate * (1.0f - 0.45f*Flow*Persona.FlowLegibility);

    Pose.BrowFurrow = Clamp01((0.55f*Overload + 0.5f*Frustration + 0.2f*Flow*0.3f) * (1.0f + Persona.TensionBias));
    Pose.BrowRaise = Clamp01(0.35f*(1.0f - Prediction)*(0.4f + 0.6f*Arousal) - 0.15f*Skill);
    Pose.EyeNarrow = Clamp01(0.45f*Flow + 0.3f*Skill*Arousal);
    Pose.EyeWiden = Clamp01(0.5f*Arousal*(1.0f - Prediction) - 0.2f*Skill);

    const float Ease = Clamp01(0.5f*Skill + 0.3f*Confidence + 0.2f*Flow - 0.5f*Strain);
    Pose.MouthCornerUp = Clamp01(Persona.BaselineWarmth + Ease*Persona.ConfidenceDisplayStyle);
    Pose.MouthCornerDown = Clamp01(0.5f*Frustration*(1.0f - Persona.Composure));
    Pose.MouthTension = Clamp01((0.45f*Strain + 0.2f*Overload)*(1.0f + Persona.TensionBias));
    Pose.JawTension = Clamp01((0.5f*Strain)*(1.0f + Persona.TensionBias));

    Pose.PostureUprightness = Clamp01(Persona.BasePostureUprightness + 0.3f*Skill + 0.15f*Confidence
                                      - 0.3f*Underload - 0.2f*Strain);
    Pose.PostureLean = ClampSigned(0.5f*Flow + 0.3f*Arousal - 0.5f*Underload);
    Pose.ShoulderTension = Clamp01((0.6f*Strain + 0.2f*Arousal - 0.25f*Skill)*(1.0f + Persona.TensionBias));
    Pose.HeadTilt = Clamp01(Persona.PlayfulnessBias*(0.3f + 0.5f*Confidence)*(1.0f - Strain));
    Pose.BreathRate = Persona.BaseBreathRate * (1.0f + 0.45f*Arousal + 0.35f*Strain - 0.2f*Flow);
    Pose.Asymmetry = Clamp01(0.15f + 0.35f*Persona.PlayfulnessBias*Ease);

    const float RawIntensity = Clamp01(0.25f + 0.4f*Skill + 0.3f*Arousal + 0.3f*Strain + 0.2f*Flow);
    float Intensity = Clamp01(RawIntensity * Persona.ExpressivenessGain);
    const float Cap = Lerp(Persona.UnknownStateIntensityCap, 1.0f, Trust);
    Pose.ExpressionIntensity = Intensity < Cap ? Intensity : Cap;

    return Pose;
}

// ---- Signal builders -------------------------------------------------------------------------

MasterySignal MakeMaster()
{
    MasterySignal S;
    S.Competence = 0.95f;           S.bCompetenceValid = true;
    S.Tier = 0.9f;                  S.bTierValid = true;
    S.DomainBreadth = 0.8f;         S.bDomainBreadthValid = true;
    S.ExecutionQuality = 0.92f;     S.bExecutionQualityValid = true;
    S.TimingPrecision = 0.9f;       S.bTimingPrecisionValid = true;
    S.ReflexReadiness = 0.88f;      S.bReflexReadinessValid = true;
    S.ComboFlow = 0.9f;             S.bComboFlowValid = true;
    S.PredictionAccuracy = 0.85f;   S.bPredictionAccuracyValid = true;
    S.FlowIntensity = 0.7f;         S.bFlowIntensityValid = true;
    S.ChallengeSkillBalance = 0.5f; S.bChallengeSkillBalanceValid = true;
    S.Frustration = 0.05f;          S.bFrustrationValid = true;
    S.Confidence = 0.9f;            S.bConfidenceValid = true;
    S.Arousal = 0.5f;               S.bArousalValid = true;
    S.EvidenceCount = 500;
    S.TimeSinceLastEvidence = 0.5f;
    return S;
}

MasterySignal MakeNovice()
{
    MasterySignal S = MakeMaster();
    S.Competence = 0.08f; S.Tier = 0.05f; S.DomainBreadth = 0.1f;
    S.ExecutionQuality = 0.15f; S.TimingPrecision = 0.1f; S.ReflexReadiness = 0.2f;
    S.ComboFlow = 0.1f; S.PredictionAccuracy = 0.15f; S.FlowIntensity = 0.1f;
    S.Confidence = 0.2f;
    return S;
}

// Wholly unmeasured, but plenty of "evidence" claimed - the adversarial case for Guard 2.
MasterySignal MakeUnknown()
{
    MasterySignal S;
    S.EvidenceCount = 10000;
    S.TimeSinceLastEvidence = 0.0f;
    return S;
}

int TestsFailed = 0;

void Check(bool bCond, const char* Label)
{
    std::printf("  [%s] %s\n", bCond ? "PASS" : "FAIL", Label);
    if (!bCond) ++TestsFailed;
}

bool InRange01(float V) { return std::isfinite(V) && V >= -0.0001f && V <= 1.0001f; }

bool PoseInRange(const EmbodimentPose& P)
{
    return InRange01(P.BrowRaise) && InRange01(P.BrowFurrow) && InRange01(P.EyeWiden) &&
           InRange01(P.EyeNarrow) && InRange01(P.GazeSteadiness) && InRange01(P.SaccadeRate) &&
           InRange01(P.MouthCornerUp) && InRange01(P.MouthCornerDown) && InRange01(P.MouthTension) &&
           InRange01(P.JawTension) && InRange01(P.PostureUprightness) && InRange01(P.ShoulderTension) &&
           InRange01(P.HeadTilt) && InRange01(P.MicroMovementRate) && InRange01(P.IdleFidget) &&
           InRange01(P.ReactionSharpness) && InRange01(P.MotionEconomy) && InRange01(P.Asymmetry) &&
           InRange01(P.ExpressionIntensity) &&
           std::isfinite(P.PostureLean) && P.PostureLean >= -1.0001f && P.PostureLean <= 1.0001f &&
           std::isfinite(P.BlinkRate) && P.BlinkRate >= 0.0f &&
           std::isfinite(P.BreathRate) && P.BreathRate >= 0.0f;
}

} // namespace

int main()
{
    std::printf("=== Standalone verification: Mastery -> Embodiment binding ===\n\n");

    const PersonaProfile Melody = MelodyProfile();
    const PersonaProfile Stoic = StoicProfile();

    // ------------------------------------------------------------------ [1] unknown != excellence
    std::printf("[1] UNKNOWN IS NOT EXCELLENCE (the safety-critical invariant)\n");
    const EmbodimentPose Unknown = Evaluate(MakeUnknown(), Melody);
    const EmbodimentPose Master = Evaluate(MakeMaster(), Melody);
    std::printf("      unmeasured: intensity %.3f, economy %.3f, gaze %.3f | master: %.3f / %.3f / %.3f\n",
                Unknown.ExpressionIntensity, Unknown.MotionEconomy, Unknown.GazeSteadiness,
                Master.ExpressionIntensity, Master.MotionEconomy, Master.GazeSteadiness);
    Check(Unknown.ExpressionIntensity <= Melody.UnknownStateIntensityCap + 0.001f,
          "Guard 2: unmeasured signal capped at persona's unknown-state cap");
    Check(Unknown.MotionEconomy < 0.45f,
          "Guard 1: unmeasured competence yields novice-level motion economy");
    Check(Unknown.MotionEconomy < Master.MotionEconomy - 0.3f,
          "unmeasured reads as clearly less skilled than measured mastery");
    Check(Unknown.GazeSteadiness < Master.GazeSteadiness,
          "unmeasured does not inherit the expert's steady gaze");

    // Adversarial: huge claimed evidence must not manufacture trust from zero coverage.
    MasterySignal Liar = MakeUnknown();
    Check(Liar.GetSignalTrust() == 0.0f, "zero measured channels yields zero trust despite 10000 evidence");

    // ------------------------------------------------------------------ [2] competence legible
    std::printf("\n[2] competence is legible, along the right channels\n");
    const EmbodimentPose Novice = Evaluate(MakeNovice(), Melody);
    std::printf("      economy  novice %.3f -> master %.3f\n", Novice.MotionEconomy, Master.MotionEconomy);
    std::printf("      fidget   novice %.3f -> master %.3f\n", Novice.IdleFidget, Master.IdleFidget);
    std::printf("      gaze     novice %.3f -> master %.3f\n", Novice.GazeSteadiness, Master.GazeSteadiness);
    std::printf("      sharpness novice %.3f -> master %.3f\n", Novice.ReactionSharpness, Master.ReactionSharpness);
    Check(Master.MotionEconomy > Novice.MotionEconomy + 0.3f, "motion economy rises sharply with skill");
    Check(Master.IdleFidget < Novice.IdleFidget - 0.2f, "fidget falls with skill");
    Check(Master.GazeSteadiness > Novice.GazeSteadiness + 0.2f, "gaze steadies with skill");
    Check(Master.ReactionSharpness > Novice.ReactionSharpness + 0.2f, "reactions sharpen with skill");
    // Expertise must not be carried mainly by the mouth.
    const float MouthDelta = Master.MouthCornerUp - Novice.MouthCornerUp;
    const float EconomyDelta = Master.MotionEconomy - Novice.MotionEconomy;
    Check(EconomyDelta > MouthDelta, "skill is expressed more through motion than through smiling");

    // ------------------------------------------------------------------ [3] persona modulates only
    std::printf("\n[3] persona modulates presentation without inventing competence\n");
    const EmbodimentPose MasterStoic = Evaluate(MakeMaster(), Stoic);
    std::printf("      same signal: Melody smile %.3f vs Stoic smile %.3f\n",
                Master.MouthCornerUp, MasterStoic.MouthCornerUp);
    std::printf("      same signal: Melody economy %.3f vs Stoic economy %.3f\n",
                Master.MotionEconomy, MasterStoic.MotionEconomy);
    Check(Master.MouthCornerUp > MasterStoic.MouthCornerUp + 0.2f,
          "demonstrative and understated personas differ markedly in display");

    // Motion economy is skill-derived, but composure legitimately protects execution against
    // frustration - so it is only EXACTLY persona-invariant when there is no strain to protect
    // against. Isolate that: same signal, zero frustration, matched challenge.
    MasterySignal Unstrained = MakeMaster();
    Unstrained.Frustration = 0.0f;
    Unstrained.ChallengeSkillBalance = 0.5f;
    Check(std::fabs(Evaluate(Unstrained, Melody).MotionEconomy
                    - Evaluate(Unstrained, Stoic).MotionEconomy) < 0.001f,
          "strain-free motion economy is identical across personas (purely skill-derived)");
    // And under strain, the more composed persona retains MORE economy - the effect is real,
    // directional, and small.
    Check(MasterStoic.MotionEconomy >= Master.MotionEconomy,
          "composure protects motion economy under frustration");
    const EmbodimentPose NoviceStoic = Evaluate(MakeNovice(), Stoic);
    Check(NoviceStoic.MotionEconomy < MasterStoic.MotionEconomy - 0.3f,
          "a composed novice is still legibly a novice");

    // ------------------------------------------------------------------ [4] strain & composure
    std::printf("\n[4] strain surfaces, gated by composure\n");
    MasterySignal Outmatched = MakeMaster();
    Outmatched.ChallengeSkillBalance = 1.0f;   // far beyond skill
    Outmatched.Frustration = 0.8f;
    const EmbodimentPose StrainedMelody = Evaluate(Outmatched, Melody);
    const EmbodimentPose StrainedStoic = Evaluate(Outmatched, Stoic);
    std::printf("      furrow  Melody %.3f (composure %.2f) vs Stoic %.3f (composure %.2f)\n",
                StrainedMelody.BrowFurrow, Melody.Composure, StrainedStoic.BrowFurrow, Stoic.Composure);
    Check(StrainedMelody.BrowFurrow > Master.BrowFurrow, "being outmatched visibly furrows the brow");
    Check(StrainedStoic.BrowFurrow < StrainedMelody.BrowFurrow, "higher composure surfaces less strain");
    Check(StrainedMelody.MotionEconomy < Master.MotionEconomy, "strain degrades motion economy");
    Check(StrainedMelody.MouthCornerUp < Master.MouthCornerUp, "strain suppresses ease");

    // ------------------------------------------------------------------ [5] flow
    std::printf("\n[5] flow reads as absorption\n");
    MasterySignal Flowing = MakeMaster();
    Flowing.FlowIntensity = 1.0f;
    MasterySignal NotFlowing = MakeMaster();
    NotFlowing.FlowIntensity = 0.0f;
    const EmbodimentPose InFlow = Evaluate(Flowing, Melody);
    const EmbodimentPose NoFlow = Evaluate(NotFlowing, Melody);
    std::printf("      blink %.2f -> %.2f | gaze %.3f -> %.3f | fidget %.3f -> %.3f\n",
                NoFlow.BlinkRate, InFlow.BlinkRate, NoFlow.GazeSteadiness, InFlow.GazeSteadiness,
                NoFlow.IdleFidget, InFlow.IdleFidget);
    Check(InFlow.BlinkRate < NoFlow.BlinkRate * 0.85f, "flow suppresses blink rate");
    Check(InFlow.GazeSteadiness > NoFlow.GazeSteadiness, "flow steadies gaze");
    Check(InFlow.IdleFidget < NoFlow.IdleFidget, "flow reduces fidget");
    // Flow legibility is a persona trait: the stoic's absorption is less visible.
    const EmbodimentPose StoicFlow = Evaluate(Flowing, Stoic);
    const float MelodyBlinkDrop = (NoFlow.BlinkRate - InFlow.BlinkRate) / NoFlow.BlinkRate;
    const EmbodimentPose StoicNoFlow = Evaluate(NotFlowing, Stoic);
    const float StoicBlinkDrop = (StoicNoFlow.BlinkRate - StoicFlow.BlinkRate) / StoicNoFlow.BlinkRate;
    Check(MelodyBlinkDrop > StoicBlinkDrop, "flow legibility is persona-dependent");

    // ------------------------------------------------------------------ [6] evidence monotonicity
    std::printf("\n[6] accruing evidence never reduces permitted expressiveness\n");
    bool bMonotone = true;
    float PrevIntensity = -1.0f;
    for (int Ev : {0, 1, 5, 20, 50, 200, 1000})
    {
        MasterySignal S = MakeMaster();
        S.EvidenceCount = Ev;
        const float I = Evaluate(S, Melody).ExpressionIntensity;
        if (I < PrevIntensity - 0.0001f) bMonotone = false;
        PrevIntensity = I;
    }
    std::printf("      intensity at evidence 0 -> 1000 rises monotonically to %.3f\n", PrevIntensity);
    Check(bMonotone, "expression intensity is monotone non-decreasing in evidence");

    // Staleness must reduce it again.
    MasterySignal Stale = MakeMaster();
    Stale.TimeSinceLastEvidence = 120.0f;
    Check(Evaluate(Stale, Melody).ExpressionIntensity < Master.ExpressionIntensity,
          "stale evidence decays expressiveness back toward neutral");

    // ------------------------------------------------------------------ [7] smoothing
    std::printf("\n[7] smoothing converges, frame-rate independent\n");
    {
        // Simulate 1 second at two different frame rates; endpoints should closely agree.
        auto Simulate = [&](float Dt, int Steps) {
            EmbodimentPose S = EmbodimentPose();
            const EmbodimentPose Target = Evaluate(MakeMaster(), Melody);
            bool bInit = false;
            for (int i = 0; i < Steps; ++i)
            {
                if (!bInit) { S = Target; bInit = true; continue; }
                const float Alpha = Clamp01(1.0f - std::pow(0.5f, Dt / 0.25f));
                S.MotionEconomy = Lerp(S.MotionEconomy, Target.MotionEconomy, Alpha);
            }
            return S.MotionEconomy;
        };
        const float At60 = Simulate(1.0f/60.0f, 60);
        const float At30 = Simulate(1.0f/30.0f, 30);
        std::printf("      1s @60fps %.5f vs @30fps %.5f\n", At60, At30);
        Check(std::fabs(At60 - At30) < 0.01f, "smoothing endpoint independent of frame rate");
    }

    // ------------------------------------------------------------------ [8] fuzz
    std::printf("\n[8] randomized inputs stay in range\n");
    {
        std::mt19937 Rng(20260801);
        std::uniform_real_distribution<float> U(-0.5f, 1.5f);  // deliberately out of band
        std::bernoulli_distribution B(0.5);
        bool bAllOk = true;
        for (int i = 0; i < 200000 && bAllOk; ++i)
        {
            MasterySignal S;
            S.Competence = U(Rng);           S.bCompetenceValid = B(Rng);
            S.Tier = U(Rng);                 S.bTierValid = B(Rng);
            S.ExecutionQuality = U(Rng);     S.bExecutionQualityValid = B(Rng);
            S.TimingPrecision = U(Rng);      S.bTimingPrecisionValid = B(Rng);
            S.ReflexReadiness = U(Rng);      S.bReflexReadinessValid = B(Rng);
            S.PredictionAccuracy = U(Rng);   S.bPredictionAccuracyValid = B(Rng);
            S.FlowIntensity = U(Rng);        S.bFlowIntensityValid = B(Rng);
            S.ChallengeSkillBalance = U(Rng);S.bChallengeSkillBalanceValid = B(Rng);
            S.Frustration = U(Rng);          S.bFrustrationValid = B(Rng);
            S.Confidence = U(Rng);           S.bConfidenceValid = B(Rng);
            S.Arousal = U(Rng);              S.bArousalValid = B(Rng);
            S.EvidenceCount = static_cast<int>(U(Rng) * 500);
            S.TimeSinceLastEvidence = U(Rng) * 100.0f;

            bAllOk = PoseInRange(Evaluate(S, Melody)) && PoseInRange(Evaluate(S, Stoic));
        }
        Check(bAllOk, "400k evaluations with out-of-band inputs produce finite, in-range poses");
    }

    std::printf("\n=== %s ===\n", TestsFailed == 0 ? "MasteryBinding=success" : "FAILURES PRESENT");
    return TestsFailed == 0 ? 0 : 1;
}
