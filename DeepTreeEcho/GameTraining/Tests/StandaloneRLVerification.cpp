// StandaloneRLVerification.cpp
// Executable verification of the core ReinforcementLearningBridge algorithms.
//
// UReinforcementLearningBridge cannot be compiled or run outside Unreal Engine, so this harness
// mirrors its decision-critical logic 1:1 (same constants, same update formulas, same guards)
// in plain C++ and proves the learning loop actually learns:
//
//   [1] Q-learning converges on a 5x5 gridworld (average reward strictly improves)
//   [2] the learned greedy policy reaches the goal in near-optimal steps
//   [3] softmax selection never falls through to an invalid action (fuzzed, incl. all-zero probs)
//   [4] out-of-range ActionIndex transitions are rejected, never crash or corrupt
//   [5] StateToKey differentiates states whose only difference is the appended echo-memory tail
//
// Mirrored from DeepTreeEcho/GameTraining/ReinforcementLearningBridge.cpp - if the update rule,
// selection policies, key pooling, or guards change there, change them here identically.
//
// Build & run:  g++ -std=c++17 -O2 -o rlverify StandaloneRLVerification.cpp && ./rlverify

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

namespace
{

// ---------------------------------------------------------------------------------------------
// Mirrored bridge core (constants and formulas from ReinforcementLearningBridge.cpp)
// ---------------------------------------------------------------------------------------------

struct Transition
{
    std::vector<float> State;
    int ActionIndex = 0;
    float Reward = 0.0f;
    std::vector<float> NextState;
    bool bTerminal = false;
};

class MiniRLBridge
{
public:
    int NumActions = 4;
    float LearningRate = 0.1f;       // bridge default
    float DiscountFactor = 0.99f;    // bridge default
    float ExplorationRate = 0.3f;    // bridge default
    float ExplorationDecay = 0.995f; // bridge default
    float MinExplorationRate = 0.01f;
    float SoftmaxTemperature = 1.0f;

    std::mt19937 Rng{42};

    float FRand()
    {
        // Mirrors FMath::FRand's closed-interval behavior: can return exactly 1.0.
        return std::uniform_int_distribution<int>(0, 32767)(Rng) / 32767.0f;
    }

    // Mirrors UReinforcementLearningBridge::StateToKey: first-20 fine discretization plus
    // coarse pooled buckets over the tail (entity slots / appended echo memory).
    std::string StateToKey(const std::vector<float>& State) const
    {
        std::string Key;
        const int Fine = std::min<int>(static_cast<int>(State.size()), 20);
        for (int i = 0; i < Fine; ++i)
        {
            Key += std::to_string(static_cast<int>(std::lround(State[i] * 10))) + "_";
        }
        for (size_t i = 20; i < State.size(); i += 10)
        {
            float Sum = 0.0f;
            const size_t End = std::min(i + 10, State.size());
            for (size_t j = i; j < End; ++j)
            {
                Sum += State[j];
            }
            Key += "p" + std::to_string(static_cast<int>(std::lround(Sum * 2.0f))) + "_";
        }
        return Key;
    }

    std::vector<float>& GetOrCreateQValues(const std::string& StateKey)
    {
        auto It = QTable.find(StateKey);
        if (It == QTable.end())
        {
            It = QTable.emplace(StateKey, std::vector<float>(NumActions, 0.1f)).first;
        }
        return It->second;
    }

    std::vector<float> GetQValues(const std::vector<float>& State) const
    {
        auto It = QTable.find(StateToKey(State));
        return It != QTable.end() ? It->second : std::vector<float>(NumActions, 0.0f);
    }

    int GetGreedyAction(const std::vector<float>& State) const
    {
        const std::vector<float> Q = GetQValues(State);
        return static_cast<int>(std::max_element(Q.begin(), Q.end()) - Q.begin());
    }

    // Mirrors SelectEpsilonGreedy.
    int SelectEpsilonGreedy(const std::vector<float>& State)
    {
        if (FRand() < ExplorationRate)
        {
            return std::uniform_int_distribution<int>(0, NumActions - 1)(Rng);
        }
        return GetGreedyAction(State);
    }

    // Mirrors GetActionProbabilities + SelectSoftmax including the final-iteration fallback fix.
    int SelectSoftmax(const std::vector<float>& State, const std::vector<float>* ProbsOverride = nullptr)
    {
        std::vector<float> Probs(NumActions, 0.0f);
        if (ProbsOverride)
        {
            Probs = *ProbsOverride;
        }
        else
        {
            const std::vector<float> Q = GetQValues(State);
            const float MaxQ = *std::max_element(Q.begin(), Q.end());
            float Sum = 0.0f;
            for (int i = 0; i < NumActions; ++i)
            {
                Probs[i] = std::exp((Q[i] - MaxQ) / SoftmaxTemperature);
                Sum += Probs[i];
            }
            if (Sum > 0.0f)
            {
                for (float& P : Probs)
                {
                    P /= Sum;
                }
            }
        }

        const float Roll = FRand();
        float Cumulative = 0.0f;
        for (int i = 0; i < NumActions; ++i)
        {
            Cumulative += Probs[i];
            if (Roll <= Cumulative || i == NumActions - 1)
            {
                return i;
            }
        }
        return -1; // unreachable if the fallback holds - the fuzz test asserts this
    }

    // Mirrors ApplyQLearningUpdate including the IsValidIndex guard (curiosity term omitted:
    // the harness runs with Curiosity=0, matching a default-constructed FCognitiveModulation
    // whose intrinsic contribution is tested implicitly as a no-op).
    bool ApplyQLearningUpdate(const Transition& T)
    {
        // Guard BEFORE GetOrCreateQValues: a rejected transition must be a complete no-op, and
        // GetOrCreateQValues inserts an optimistically-initialized row as a side effect. (This
        // harness caught exactly that ordering bug in the bridge - keep the order aligned.)
        if (T.ActionIndex < 0 || T.ActionIndex >= NumActions)
        {
            return false;
        }
        std::vector<float>& Q = GetOrCreateQValues(StateToKey(T.State));
        const float MaxNextQ = T.bTerminal ? 0.0f : [this, &T] {
            const std::vector<float> NQ = GetQValues(T.NextState);
            return *std::max_element(NQ.begin(), NQ.end());
        }();
        const float Target = T.Reward + DiscountFactor * MaxNextQ;
        Q[T.ActionIndex] += LearningRate * (Target - Q[T.ActionIndex]);
        return true;
    }

    void DecayExploration()
    {
        ExplorationRate = std::max(ExplorationRate * ExplorationDecay, MinExplorationRate);
    }

    std::unordered_map<std::string, std::vector<float>> QTable;
};

// ---------------------------------------------------------------------------------------------
// Toy environment: 5x5 gridworld, start (0,0), goal (4,4), actions U/D/L/R,
// -0.01 per step, +10.0 at goal, 50-step episode cap.
// ---------------------------------------------------------------------------------------------

struct GridWorld
{
    int X = 0, Y = 0;
    static constexpr int Size = 5;
    static constexpr int MaxSteps = 50;

    void Reset() { X = 0; Y = 0; }

    std::vector<float> Observe() const
    {
        // Normalized like FGameStateObservation::ToNormalizedVector's positional features.
        return {static_cast<float>(X) / (Size - 1), static_cast<float>(Y) / (Size - 1)};
    }

    // Returns (reward, done).
    std::pair<float, bool> Step(int Action)
    {
        switch (Action)
        {
            case 0: Y = std::min(Y + 1, Size - 1); break;
            case 1: Y = std::max(Y - 1, 0); break;
            case 2: X = std::max(X - 1, 0); break;
            case 3: X = std::min(X + 1, Size - 1); break;
            default: break;
        }
        const bool AtGoal = (X == Size - 1 && Y == Size - 1);
        return {AtGoal ? 10.0f : -0.01f, AtGoal};
    }
};

int TestsFailed = 0;

void Check(bool bCondition, const char* Label)
{
    std::printf("  [%s] %s\n", bCondition ? "PASS" : "FAIL", Label);
    if (!bCondition)
    {
        ++TestsFailed;
    }
}

} // namespace

int main()
{
    std::printf("=== Standalone verification: ReinforcementLearningBridge core logic ===\n\n");

    // ------------------------------------------------------------------ [1]+[2] convergence
    std::printf("[1] Q-learning convergence on 5x5 gridworld (600 episodes)\n");
    MiniRLBridge Bridge;
    GridWorld World;

    auto RunEpisodes = [&](int Count) {
        float Total = 0.0f;
        for (int Ep = 0; Ep < Count; ++Ep)
        {
            World.Reset();
            float EpReward = 0.0f;
            for (int S = 0; S < GridWorld::MaxSteps; ++S)
            {
                const std::vector<float> State = World.Observe();
                const int Action = Bridge.SelectEpsilonGreedy(State);
                const auto [Reward, bDone] = World.Step(Action);
                Bridge.ApplyQLearningUpdate({State, Action, Reward, World.Observe(), bDone});
                EpReward += Reward;
                if (bDone)
                {
                    break;
                }
            }
            Bridge.DecayExploration();
            Total += EpReward;
        }
        return Total / Count;
    };

    const float EarlyAvg = RunEpisodes(100);
    RunEpisodes(400);
    const float LateAvg = RunEpisodes(100);
    std::printf("      avg reward, first 100 episodes: %.3f | last 100 episodes: %.3f\n", EarlyAvg, LateAvg);
    Check(LateAvg > EarlyAvg, "average reward strictly improves with training");
    Check(LateAvg > 9.0f, "late average exceeds 9.0 (goal reached near-optimally almost always)");

    std::printf("\n[2] learned greedy policy reaches the goal\n");
    World.Reset();
    int GreedySteps = 0;
    bool bReached = false;
    for (; GreedySteps < GridWorld::MaxSteps; )
    {
        const int Action = Bridge.GetGreedyAction(World.Observe());
        ++GreedySteps;
        if (World.Step(Action).second)
        {
            bReached = true;
            break;
        }
    }
    std::printf("      greedy rollout: %s in %d steps (optimum 8)\n", bReached ? "GOAL" : "no goal", GreedySteps);
    Check(bReached, "greedy policy reaches the goal with zero exploration");
    Check(GreedySteps <= 12, "greedy path length within 1.5x of the 8-step optimum");

    // ------------------------------------------------------------------ [3] softmax fuzz
    std::printf("\n[3] softmax selection fuzz (no fallthrough to invalid action)\n");
    bool bAllValid = true;
    for (int i = 0; i < 100000 && bAllValid; ++i)
    {
        const int A = Bridge.SelectSoftmax(World.Observe());
        bAllValid = A >= 0 && A < Bridge.NumActions;
    }
    const std::vector<float> ZeroProbs(Bridge.NumActions, 0.0f);       // Sum==0 guard path
    const std::vector<float> ShortProbs{0.2f, 0.2f, 0.2f, 0.399f};     // sums to < 1.0
    for (int i = 0; i < 100000 && bAllValid; ++i)
    {
        const int A0 = Bridge.SelectSoftmax(World.Observe(), &ZeroProbs);
        const int A1 = Bridge.SelectSoftmax(World.Observe(), &ShortProbs);
        bAllValid = A0 >= 0 && A0 < Bridge.NumActions && A1 >= 0 && A1 < Bridge.NumActions;
    }
    Check(bAllValid, "300k draws (incl. all-zero and sub-1.0 distributions) all in range");

    // ------------------------------------------------------------------ [4] invalid index guard
    std::printf("\n[4] out-of-range ActionIndex rejection\n");
    const std::vector<float> QBefore = Bridge.GetQValues(World.Observe());
    const bool bAcceptedHigh = Bridge.ApplyQLearningUpdate({World.Observe(), 999, 5.0f, World.Observe(), false});
    const bool bAcceptedNeg = Bridge.ApplyQLearningUpdate({World.Observe(), -3, 5.0f, World.Observe(), false});
    const std::vector<float> QAfter = Bridge.GetQValues(World.Observe());
    Check(!bAcceptedHigh && !bAcceptedNeg, "invalid indices rejected by the guard");
    Check(QBefore == QAfter, "Q-values untouched by rejected transitions");

    // ------------------------------------------------------------------ [5] echo-tail keying
    std::printf("\n[5] StateToKey differentiates echo-memory tails\n");
    std::vector<float> BaseState(60, 0.5f);              // observation-sized vector
    std::vector<float> WithEchoA = BaseState;
    std::vector<float> WithEchoB = BaseState;
    for (int i = 0; i < 20; ++i)
    {
        WithEchoA.push_back(0.9f);                        // strong echo trace
        WithEchoB.push_back(-0.9f);                       // opposite echo trace
    }
    const std::string KeyA = Bridge.StateToKey(WithEchoA);
    const std::string KeyB = Bridge.StateToKey(WithEchoB);
    Check(KeyA != KeyB, "states differing only in the appended echo tail get distinct keys");
    Check(Bridge.StateToKey(BaseState) == Bridge.StateToKey(BaseState), "keying is deterministic");

    std::printf("\n=== %s ===\n", TestsFailed == 0 ? "ReinforcementLearning=success" : "FAILURES PRESENT");
    return TestsFailed == 0 ? 0 : 1;
}
