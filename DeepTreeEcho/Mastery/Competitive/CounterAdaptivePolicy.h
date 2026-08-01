// CounterAdaptivePolicy.h
// Safe exploitation: beat the exploitable without becoming exploitable.
//
// THE CORE TENSION. Two strategies are individually easy and jointly hard:
//
//   * Pure equilibrium play (uniform, in a symmetric game) is UNBEATABLE - and also unable to
//     beat anyone. Against an opponent who plays rock 90% of the time it still breaks even.
//     Safe, and useless.
//
//   * Pure best-response play crushes any fixed bias - and is itself a fixed bias, so any
//     opponent modelling YOU wins. Devastating, and suicidal.
//
// "Undefeated" requires both halves, so this policy interpolates between them by an exploitation
// weight lambda that is *earned*: it rises only with evidence the opponent is actually
// predictable, and collapses the moment our own results say we are being read.
//
//     pi = (1 - lambda) * Equilibrium  +  lambda * SoftBestResponse
//
// Two independent brakes hold lambda down, mirroring the two-guard pattern used by the
// embodiment binding:
//
//   BRAKE 1 (evidence): lambda scales with how far the opponent's empirical distribution sits
//     from equilibrium, times a saturating sample count. A read taken from six observations of
//     a near-uniform opponent buys almost no deviation.
//
//   BRAKE 2 (results): a running performance monitor. If recent results go negative while we
//     are deviating, that is the signature of being counter-exploited, so lambda is cut
//     multiplicatively and recovers only slowly. This is what converts "loses to any exploiter"
//     into "retreats to equilibrium and draws".
//
// Header-only and free of engine types so it can be exercised without Unreal - see
// Tests/StandaloneCompetitiveVerification.cpp.

#pragma once

#include <cmath>
#include <vector>

namespace Competitive
{

/** Tunables. Defaults are the values the tournament harness validates. */
struct FPolicyConfig
{
    /** Hard ceiling on deviation from equilibrium, [0,1]. Below 1 so we are never a pure BR. */
    float MaxExploitation = 0.85f;

    /** Observations needed before an opponent read is half-trusted. */
    float EvidenceHalfLife = 12.0f;

    /** Recency decay per observation - lets a shifting opponent be re-read. */
    float FrequencyDecay = 0.985f;

    /** Softmax temperature over action values. Lower = sharper exploitation. */
    float ExploitTemperature = 0.35f;

    /**
     * Window over which our own results are judged.
     *
     * Must be long enough that ordinary variance does not read as being counter-exploited. In a
     * +1/0/-1 game the per-round standard deviation is ~0.8, so a window of N has a standard
     * error of ~0.8/sqrt(N): at N=40 that is ~0.13, which dwarfed the original -0.02 threshold
     * and made the brake fire on pure noise.
     */
    int ResultWindow = 120;

    /**
     * Mean per-round result below this is treated as evidence we are being read. Set beyond
     * ~2 standard errors of the window (~0.15 at N=120) so it responds to genuine exploitation
     * rather than to sampling noise.
     */
    float UnderperformanceThreshold = -0.15f;

    /** Multiplicative cut to the results brake when underperforming. */
    float RetreatFactor = 0.70f;

    /** Additive recovery per round when performing acceptably. */
    float RecoveryRate = 0.05f;

    /** Floor on the results brake, so a retreat can never permanently disable exploitation. */
    float MinResultsBrake = 0.10f;

    /**
     * Weight of the conditional (sequential) model relative to the marginal one.
     *
     * A deterministic cycler R,P,S,R,P,S is perfectly predictable yet has an exactly UNIFORM
     * marginal distribution - a frequency-only model sees equilibrium and refuses to exploit it.
     * Conditioning on the opponent's previous action exposes that structure. Set to 0 to
     * disable sequential modelling.
     */
    float ConditionalWeight = 1.0f;

    /** Observations of a given predecessor action before its conditional read is half-trusted. */
    float ConditionalEvidenceHalfLife = 8.0f;
};

/**
 * Counter-adaptive mixed-strategy policy for a symmetric N-action matrix game.
 *
 * Payoff convention: Payoff(a, b) is the row player's score for playing a against b.
 * Supplied by the caller so the policy is game-agnostic.
 */
class FCounterAdaptivePolicy
{
public:
    FCounterAdaptivePolicy(int InNumActions, const FPolicyConfig& InConfig = FPolicyConfig())
        : NumActions(InNumActions)
        , Config(InConfig)
        , OpponentCounts(InNumActions, 0.0f)
        , ConditionalCounts(InNumActions, std::vector<float>(InNumActions, 0.0f))
        , ConditionalTotals(InNumActions, 0.0f)
        , LastOpponentAction(-1)
        , ResultsBrake(1.0f)
        , TotalObservations(0.0f)
    {
    }

    /** Reset all learned state (new opponent). */
    void Reset()
    {
        std::fill(OpponentCounts.begin(), OpponentCounts.end(), 0.0f);
        for (auto& Row : ConditionalCounts) { std::fill(Row.begin(), Row.end(), 0.0f); }
        std::fill(ConditionalTotals.begin(), ConditionalTotals.end(), 0.0f);
        LastOpponentAction = -1;
        RecentResults.clear();
        ResultsBrake = 1.0f;
        TotalObservations = 0.0f;
    }

    /** Record what the opponent just played. */
    void ObserveOpponentAction(int Action)
    {
        if (Action < 0 || Action >= NumActions) return;

        // Recency-weighted marginal counts: an opponent who switches strategy can be re-read.
        for (float& C : OpponentCounts) { C *= Config.FrequencyDecay; }
        TotalObservations = TotalObservations * Config.FrequencyDecay + 1.0f;
        OpponentCounts[Action] += 1.0f;

        // Sequential model: P(Action | previous action). This is what catches cyclers and other
        // patterned opponents whose marginal distribution looks like equilibrium.
        if (LastOpponentAction >= 0 && LastOpponentAction < NumActions)
        {
            std::vector<float>& Row = ConditionalCounts[LastOpponentAction];
            for (float& C : Row) { C *= Config.FrequencyDecay; }
            ConditionalTotals[LastOpponentAction] =
                ConditionalTotals[LastOpponentAction] * Config.FrequencyDecay + 1.0f;
            Row[Action] += 1.0f;
        }

        LastOpponentAction = Action;
    }

    /** Record our own round result (+1 win / 0 draw / -1 loss, or any real payoff). */
    void RecordResult(float Payoff)
    {
        RecentResults.push_back(Payoff);
        if (static_cast<int>(RecentResults.size()) > Config.ResultWindow)
        {
            RecentResults.erase(RecentResults.begin());
        }

        // BRAKE 2. Only judged once the window is meaningfully full, so early variance does not
        // trigger a spurious retreat.
        if (static_cast<int>(RecentResults.size()) >= Config.ResultWindow / 2)
        {
            float Sum = 0.0f;
            for (float R : RecentResults) { Sum += R; }
            const float Mean = Sum / static_cast<float>(RecentResults.size());

            if (Mean < Config.UnderperformanceThreshold)
            {
                // Being read. Retreat toward equilibrium fast.
                ResultsBrake *= Config.RetreatFactor;
                if (ResultsBrake < Config.MinResultsBrake) { ResultsBrake = Config.MinResultsBrake; }
            }
            else
            {
                // Recover slowly - much slower than we retreat, so an opponent cannot bait us
                // back out with a few deliberately soft rounds.
                ResultsBrake += Config.RecoveryRate;
                if (ResultsBrake > 1.0f) { ResultsBrake = 1.0f; }
            }
        }
    }

    /** Marginal estimate of the opponent's action distribution. Uniform when unobserved. */
    std::vector<float> GetMarginalEstimate() const
    {
        std::vector<float> Q(NumActions, 1.0f / static_cast<float>(NumActions));
        if (TotalObservations <= 0.0f) return Q;
        for (int i = 0; i < NumActions; ++i)
        {
            Q[i] = OpponentCounts[i] / TotalObservations;
        }
        return Q;
    }

    /** Normalized distance of a distribution from equilibrium, [0,1]. 0 = uniform, 1 = pure. */
    float Predictability(const std::vector<float>& Q) const
    {
        const float Uniform = 1.0f / static_cast<float>(NumActions);
        float TV = 0.0f;
        for (int i = 0; i < NumActions; ++i) { TV += std::fabs(Q[i] - Uniform); }
        TV *= 0.5f;
        const float MaxTV = 1.0f - Uniform;
        return (MaxTV > 0.0f) ? (TV / MaxTV) : 0.0f;
    }

    /**
     * Best available estimate of what the opponent plays NEXT, together with how much that
     * estimate should be trusted.
     *
     * Prefers the sequential model P(next | last) when it is both better-evidenced and more
     * predictive than the marginal. A deterministic cycler has a uniform marginal (predictability
     * 0) but a near-pure conditional (predictability ~1), which is exactly the case a
     * frequency-only model cannot see.
     */
    void GetPrediction(std::vector<float>& OutQ, float& OutConfidence) const
    {
        // Marginal branch.
        OutQ = GetMarginalEstimate();
        const float MarginalEvidence = TotalObservations / (TotalObservations + Config.EvidenceHalfLife);
        OutConfidence = (TotalObservations > 0.0f) ? Predictability(OutQ) * MarginalEvidence : 0.0f;

        if (Config.ConditionalWeight <= 0.0f || LastOpponentAction < 0) return;

        const float CondTotal = ConditionalTotals[LastOpponentAction];
        if (CondTotal <= 0.0f) return;

        std::vector<float> CondQ(NumActions, 1.0f / static_cast<float>(NumActions));
        for (int i = 0; i < NumActions; ++i)
        {
            CondQ[i] = ConditionalCounts[LastOpponentAction][i] / CondTotal;
        }

        const float CondEvidence = CondTotal / (CondTotal + Config.ConditionalEvidenceHalfLife);
        const float CondConfidence = Predictability(CondQ) * CondEvidence * Config.ConditionalWeight;

        if (CondConfidence > OutConfidence)
        {
            OutQ = CondQ;
            OutConfidence = CondConfidence;
        }
    }

    /** Backwards-compatible alias for the marginal estimate. */
    std::vector<float> GetOpponentEstimate() const { return GetMarginalEstimate(); }

    /**
     * Current exploitation weight, [0, MaxExploitation]. Product of both brakes.
     * Exposed so tests can assert the brakes actually engage, rather than only observing
     * their downstream effect.
     */
    float GetExploitationWeight() const
    {
        if (TotalObservations <= 0.0f) return 0.0f;

        // BRAKE 1 is folded into the prediction confidence (predictability x evidence), across
        // whichever of the marginal / sequential models is currently the better predictor.
        std::vector<float> Q;
        float Confidence = 0.0f;
        GetPrediction(Q, Confidence);

        // BRAKE 2 multiplies in here.
        return Config.MaxExploitation * Confidence * ResultsBrake;
    }

    /** Value of the results brake alone, [0,1]. 1 = unrestrained, low = actively retreating. */
    float GetResultsBrake() const { return ResultsBrake; }

    /**
     * The mixed strategy to play this round.
     * PayoffFn(a, b) must return the row player's payoff for a against b.
     */
    template <typename TPayoffFn>
    std::vector<float> GetStrategy(TPayoffFn PayoffFn) const
    {
        const float Uniform = 1.0f / static_cast<float>(NumActions);
        std::vector<float> Pi(NumActions, Uniform);

        const float Lambda = GetExploitationWeight();
        if (Lambda <= 0.0f) return Pi;

        // Use the same predictor the exploitation weight was derived from, so the strategy
        // best-responds to whichever model actually earned the deviation.
        std::vector<float> Q;
        float Confidence = 0.0f;
        GetPrediction(Q, Confidence);

        // Expected value of each action against the estimated opponent.
        std::vector<float> EV(NumActions, 0.0f);
        for (int a = 0; a < NumActions; ++a)
        {
            float V = 0.0f;
            for (int b = 0; b < NumActions; ++b)
            {
                V += Q[b] * static_cast<float>(PayoffFn(a, b));
            }
            EV[a] = V;
        }

        // Soft best response. Softmax rather than argmax: a pure BR is a pure strategy, which
        // is maximally exploitable by anyone modelling us.
        float MaxEV = EV[0];
        for (float V : EV) { if (V > MaxEV) MaxEV = V; }

        std::vector<float> Exploit(NumActions, 0.0f);
        float Sum = 0.0f;
        for (int a = 0; a < NumActions; ++a)
        {
            Exploit[a] = std::exp((EV[a] - MaxEV) / Config.ExploitTemperature);
            Sum += Exploit[a];
        }
        if (Sum > 0.0f)
        {
            for (float& E : Exploit) { E /= Sum; }
        }
        else
        {
            for (float& E : Exploit) { E = Uniform; }
        }

        // Interpolate equilibrium <-> exploitation.
        for (int a = 0; a < NumActions; ++a)
        {
            Pi[a] = (1.0f - Lambda) * Uniform + Lambda * Exploit[a];
        }
        return Pi;
    }

    /** Sample an action from a strategy given a uniform random draw in [0,1). */
    static int SampleFrom(const std::vector<float>& Strategy, float Roll)
    {
        float Cumulative = 0.0f;
        const int N = static_cast<int>(Strategy.size());
        for (int i = 0; i < N; ++i)
        {
            Cumulative += Strategy[i];
            // Final index always selects: float error must never fall through to an invalid one.
            if (Roll <= Cumulative || i == N - 1) return i;
        }
        return 0;
    }

private:
    int NumActions;
    FPolicyConfig Config;
    std::vector<float> OpponentCounts;
    std::vector<std::vector<float>> ConditionalCounts;  // [prev][next]
    std::vector<float> ConditionalTotals;               // [prev]
    int LastOpponentAction;
    std::vector<float> RecentResults;
    float ResultsBrake;
    float TotalObservations;
};

} // namespace Competitive
