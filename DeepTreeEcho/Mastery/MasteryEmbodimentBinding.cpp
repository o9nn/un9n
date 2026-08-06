// MasteryEmbodimentBinding.cpp
// Implementation of the mastery -> embodiment rules.
//
// Keep this file and Tests/StandaloneMasteryBindingVerification.cpp in lockstep: the test
// mirrors these rules so they can be exercised without Unreal.

#include "MasteryEmbodimentBinding.h"

namespace
{
    float Clamp01(float V)
    {
        return V < 0.0f ? 0.0f : (V > 1.0f ? 1.0f : V);
    }

    float ClampSigned(float V)
    {
        return V < -1.0f ? -1.0f : (V > 1.0f ? 1.0f : V);
    }

    float Lerp(float A, float B, float T)
    {
        return A + (B - A) * T;
    }
}

FMasteryEmbodimentPose MasteryEmbodimentBinding::Evaluate(const FMasterySignal& Signal,
                                                           const FMasteryPersonaProfile& Persona)
{
    FMasteryEmbodimentPose Pose;

    // Trust gates how far we may depart from neutral at all.
    const float Trust = Signal.GetSignalTrust();

    // GUARD 1: every unmeasured channel falls back to a novice-leaning default, stated at the
    // point of use. Competence unknown -> assume NOT competent. Flow unknown -> assume not in
    // flow. Frustration unknown -> assume calm (the charitable default, and the one that avoids
    // inventing distress the system never observed).
    const float Competence   = Signal.GetCompetence(0.0f);
    const float Tier         = Signal.GetTier(0.0f);
    const float Execution    = Signal.GetExecutionQuality(0.0f);
    const float Timing       = Signal.GetTimingPrecision(0.0f);
    const float Reflex       = Signal.GetReflexReadiness(0.0f);
    const float ComboFlow    = Signal.GetComboFlow(0.0f);
    const float Prediction   = Signal.GetPredictionAccuracy(0.0f);
    const float Flow         = Signal.GetFlowIntensity(0.0f);
    const float Balance      = Signal.GetChallengeSkillBalance(0.5f);  // 0.5 = matched
    const float Frustration  = Signal.GetFrustration(0.0f);
    const float Confidence   = Signal.GetConfidence(0.0f);
    const float Arousal      = Signal.GetArousal(0.0f);

    // Composite competence: sustained skill blended with how cleanly it is being executed now.
    const float Skill = Clamp01(0.5f * Competence + 0.2f * Tier + 0.2f * Execution + 0.1f * Timing);

    // Overload: how far challenge exceeds skill. Composure suppresses how much of it surfaces.
    const float RawOverload = Clamp01((Balance - 0.5f) * 2.0f);
    const float Overload = RawOverload * (1.0f - Persona.Composure);

    // Underload: challenge well below skill -> boredom.
    const float Underload = Clamp01((0.5f - Balance) * 2.0f);

    // Strain the persona actually shows.
    const float Strain = Clamp01(Overload + Frustration * (1.0f - Persona.Composure));

    // ---- Motion quality: the primary carrier of legible expertise -------------------------

    // Economy of motion rises with skill and falls with strain. This is the single strongest
    // "she is good at this" cue.
    //
    // ComboFlow belongs here rather than nowhere. Chaining actions cleanly, without a reset
    // between them, IS economy of motion - and until now this channel was read from the signal
    // and then dropped, which meant it still counted toward signal coverage (and so toward the
    // trust that unlocks expressiveness) while informing no part of the pose. A channel that buys
    // amplitude without paying for it undermines the trust model.
    const float ExecutionFluency = Clamp01(0.75f * Skill + 0.25f * ComboFlow);
    Pose.MotionEconomy = Clamp01(Lerp(0.25f, 0.95f, ExecutionFluency) - 0.25f * Strain);

    // Reaction sharpness comes from reflex readiness and prediction - knowing what is coming
    // removes wind-up.
    // Coefficients still sum to 1.0 with the baseline, so the ceiling is unchanged.
    Pose.ReactionSharpness = Clamp01(0.15f + 0.40f * Reflex + 0.25f * Prediction
                                     + 0.20f * ComboFlow);

    // Fidget falls with skill and flow, rises with underload (boredom) and persona bias.
    Pose.IdleFidget = Clamp01(0.55f - 0.4f * Skill - 0.25f * Flow + 0.3f * Underload + Persona.FidgetBias);

    // Micro-movement: composed stillness at high skill; restlessness under strain.
    Pose.MicroMovementRate = Clamp01(0.5f - 0.35f * Skill + 0.3f * Strain + 0.2f * Arousal);

    // ---- Gaze: the second strongest expertise cue -------------------------------------------

    // The expert's gaze is already where it needs to be.
    Pose.GazeSteadiness = Clamp01(0.3f + 0.4f * Skill + 0.3f * Flow - 0.25f * Strain);

    // Searching behavior is the inverse: low skill hunts visually.
    Pose.SaccadeRate = Clamp01(0.7f - 0.4f * Skill - 0.2f * Flow + 0.25f * Strain);

    // Blink suppression is a real correlate of absorption, scaled by how legible this persona's
    // flow is meant to be.
    Pose.BlinkRate = Persona.BaseBlinkRate * (1.0f - 0.45f * Flow * Persona.FlowLegibility);

    // ---- Face --------------------------------------------------------------------------------

    // Concentration furrow: driven by load, not by skill. An expert under real pressure still
    // furrows; the difference is that less pressure reaches them.
    Pose.BrowFurrow = Clamp01((0.55f * Overload + 0.5f * Frustration + 0.2f * Flow * 0.3f)
                              * (1.0f + Persona.TensionBias));

    // Brow raise: surprise/uncertainty. Falls with prediction accuracy - little surprises her.
    Pose.BrowRaise = Clamp01(0.35f * (1.0f - Prediction) * (0.4f + 0.6f * Arousal) - 0.15f * Skill);

    // Target lock: narrowed eyes under focused engagement.
    Pose.EyeNarrow = Clamp01(0.45f * Flow + 0.3f * Skill * Arousal);

    // Startle: high arousal without the competence to have anticipated it.
    Pose.EyeWiden = Clamp01(0.5f * Arousal * (1.0f - Prediction) - 0.2f * Skill);

    // Ease. Here is where ConfidenceDisplayStyle earns its keep: the SAME competence renders as
    // a visible smile in a demonstrative persona and as near-nothing in an understated one.
    //
    // Positive coefficients sum to exactly 1.0 so Ease approaches its ceiling asymptotically
    // instead of clamping. An earlier version summed to 1.3, which saturated at mastery and
    // pinned a demonstrative persona at a near-maximum grin - that read as smugness, and it made
    // the mouth a bigger skill-tell than motion economy, inverting the design principle above.
    const float Ease = Clamp01(0.5f * Skill + 0.3f * Confidence + 0.2f * Flow - 0.5f * Strain);
    Pose.MouthCornerUp = Clamp01(Persona.BaselineWarmth + Ease * Persona.ConfidenceDisplayStyle);

    Pose.MouthCornerDown = Clamp01(0.5f * Frustration * (1.0f - Persona.Composure));
    Pose.MouthTension = Clamp01((0.45f * Strain + 0.2f * Overload) * (1.0f + Persona.TensionBias));
    Pose.JawTension = Clamp01((0.5f * Strain) * (1.0f + Persona.TensionBias));

    // ---- Body ---------------------------------------------------------------------------------

    Pose.PostureUprightness = Clamp01(Persona.BasePostureUprightness + 0.3f * Skill
                                      + 0.15f * Confidence - 0.3f * Underload - 0.2f * Strain);

    // Engagement presses in; boredom withdraws.
    Pose.PostureLean = ClampSigned(0.5f * Flow + 0.3f * Arousal - 0.5f * Underload);

    Pose.ShoulderTension = Clamp01((0.6f * Strain + 0.2f * Arousal - 0.25f * Skill)
                                   * (1.0f + Persona.TensionBias));

    // Playful appraisal - suppressed under strain, and SIGNED.
    //
    // Magnitude is playfulness gated by strain, as before. The sign comes from whether the
    // situation is still a question or already an answer: unresolved reading of an opponent tilts
    // inquisitively (+, WONDER_02), settled amusement tilts mirthfully (-, JOY_02). Both
    // directions occur in the authored set, so a magnitude-only channel could never reach half
    // of it.
    const float Inquisitive = Clamp01(1.0f - Prediction);
    Pose.HeadTilt = ClampSigned(Persona.PlayfulnessBias * (1.0f - Strain)
                                * (0.9f * Inquisitive - 0.8f * Ease));

    Pose.BreathRate = Persona.BaseBreathRate * (1.0f + 0.45f * Arousal + 0.35f * Strain - 0.2f * Flow);

    // AU17 as assertion. Rises with confidence that is being EXERCISED - confidence alone, with
    // nothing to push against, is just contentment. So it needs arousal (something is at stake)
    // and it is suppressed by frustration, because a chin lifted while losing reads as delusion
    // rather than defiance. Demonstrative personas show it more; understated ones barely at all.
    //
    // The 0.6 gain puts this on the authored catalog's scale rather than on an abstract 0..1 AU
    // scale. Chin raise is a SLIGHT lift everywhere it appears in the catalog - PUNK_03 "Defiant
    // Confidence", the most assertive entry in the whole library, sits at 0.20. An ungained
    // formula produced roughly 0.49 for a confident winner, which is not defiance, it is a
    // chin-up sneer. The arousal gate has almost no floor for the same reason: assertion with
    // nothing to assert against is just contentment, and should read as a resting chin.
    Pose.ChinRaise = Clamp01(0.6f * (0.55f * Confidence + 0.25f * Skill)
                             * (0.10f + 0.90f * Arousal)
                             * (1.0f - 0.7f * Frustration)
                             * (0.3f + 0.7f * Persona.ConfidenceDisplayStyle));

    // Genuine affect is asymmetric; perfectly symmetric faces read as synthetic.
    //
    // Driven by ENGAGEMENT, not by ease. The authored catalog makes asymmetry the signature of
    // the challenge register - "want to play" and "what you've got" are both marked asymmetry
    // High - and asymmetry is what separates the PUNK smirk from the JOY smile. An earlier
    // version scaled it by Ease, which had it backwards: a hard-fought exchange flattened her
    // face toward perfect symmetry exactly when the stakes were highest, which is precisely when
    // a synthetic-looking face is most costly.
    //
    // The 0.55 span (rather than 0.35) is a REACHABILITY fix, measured rather than guessed. The
    // catalog's signature smirks - PUNK_01 "Confident Smirk" and PUNK_02 "Mischievous Grin" -
    // both sit at an asymmetry of 0.67 in pose terms (mouth_smile_L 0.5/0.6 against R 0.3/0.4).
    // A 0.35 span capped the binding near 0.41 even at maximum playfulness and engagement, so
    // those two expressions were nearest-neighbour to roughly 0.002% of a 1.5M-sample sweep of
    // the signal space - present in the library, unreachable in practice. Melody could be
    // defiant (PUNK_03, whose authored asymmetry is only 0.33) but could not actually smirk.
    const float Engaged = Clamp01(0.5f * Confidence + 0.5f * Arousal);
    Pose.Asymmetry = Clamp01(0.15f + 0.55f * Persona.PlayfulnessBias * Engaged);

    // ---- Expression intensity, and GUARD 2 -----------------------------------------------------

    // How much is there to express at all.
    const float RawIntensity = Clamp01(0.25f + 0.4f * Skill + 0.3f * Arousal + 0.3f * Strain + 0.2f * Flow);

    float Intensity = Clamp01(RawIntensity * Persona.ExpressivenessGain);

    // GUARD 2 (independent of Guard 1): an untrusted signal cannot drive a strong pose, whatever
    // the rules above computed. At zero trust we are capped at the persona's unknown-state cap;
    // at full trust the cap lifts entirely. Interpolating (rather than hard-switching) avoids a
    // visible pop as evidence accumulates.
    const float Cap = Lerp(Persona.UnknownStateIntensityCap, 1.0f, Trust);
    Pose.ExpressionIntensity = Intensity < Cap ? Intensity : Cap;

    return Pose;
}

FMasteryEmbodimentPose MasteryEmbodimentBinding::EvaluateSmoothed(const FMasterySignal& Signal,
                                                                  const FMasteryPersonaProfile& Persona,
                                                                  FMasteryBindingState& State,
                                                                  float DeltaTime,
                                                                  float HalfLifeSeconds)
{
    const FMasteryEmbodimentPose Target = Evaluate(Signal, Persona);

    if (!State.bInitialized)
    {
        State.SmoothedPose = Target;
        State.bInitialized = true;
        return State.SmoothedPose;
    }

    // Frame-rate independent exponential smoothing.
    float Alpha = 1.0f;
    if (HalfLifeSeconds > 0.0f && DeltaTime > 0.0f)
    {
        Alpha = 1.0f - FMath::Pow(0.5f, DeltaTime / HalfLifeSeconds);
    }
    Alpha = Clamp01(Alpha);

    FMasteryEmbodimentPose& S = State.SmoothedPose;
    S.BrowRaise           = Lerp(S.BrowRaise,           Target.BrowRaise,           Alpha);
    S.BrowFurrow          = Lerp(S.BrowFurrow,          Target.BrowFurrow,          Alpha);
    S.EyeWiden            = Lerp(S.EyeWiden,            Target.EyeWiden,            Alpha);
    S.EyeNarrow           = Lerp(S.EyeNarrow,           Target.EyeNarrow,           Alpha);
    S.GazeSteadiness      = Lerp(S.GazeSteadiness,      Target.GazeSteadiness,      Alpha);
    S.BlinkRate           = Lerp(S.BlinkRate,           Target.BlinkRate,           Alpha);
    S.SaccadeRate         = Lerp(S.SaccadeRate,         Target.SaccadeRate,         Alpha);
    S.MouthCornerUp       = Lerp(S.MouthCornerUp,       Target.MouthCornerUp,       Alpha);
    S.MouthCornerDown     = Lerp(S.MouthCornerDown,     Target.MouthCornerDown,     Alpha);
    S.MouthTension        = Lerp(S.MouthTension,        Target.MouthTension,        Alpha);
    S.JawTension          = Lerp(S.JawTension,          Target.JawTension,          Alpha);
    S.ChinRaise           = Lerp(S.ChinRaise,           Target.ChinRaise,           Alpha);
    S.PostureUprightness  = Lerp(S.PostureUprightness,  Target.PostureUprightness,  Alpha);
    S.PostureLean         = Lerp(S.PostureLean,         Target.PostureLean,         Alpha);
    S.ShoulderTension     = Lerp(S.ShoulderTension,     Target.ShoulderTension,     Alpha);
    S.HeadTilt            = Lerp(S.HeadTilt,            Target.HeadTilt,            Alpha);
    S.MicroMovementRate   = Lerp(S.MicroMovementRate,   Target.MicroMovementRate,   Alpha);
    S.IdleFidget          = Lerp(S.IdleFidget,          Target.IdleFidget,          Alpha);
    S.ReactionSharpness   = Lerp(S.ReactionSharpness,   Target.ReactionSharpness,   Alpha);
    S.MotionEconomy       = Lerp(S.MotionEconomy,       Target.MotionEconomy,       Alpha);
    S.BreathRate          = Lerp(S.BreathRate,          Target.BreathRate,          Alpha);
    S.Asymmetry           = Lerp(S.Asymmetry,           Target.Asymmetry,           Alpha);
    S.ExpressionIntensity = Lerp(S.ExpressionIntensity, Target.ExpressionIntensity, Alpha);

    return S;
}
