// LorenzChaoticDynamics.h
// Deep Tree Echo - Lorenz Attractor Chaotic Dynamics for Micro-Expression Generation
// Copyright (c) 2025-2026 Deep Tree Echo Project
//
// Implements a Lorenz attractor system with RK4 integration to generate
// naturalistic, non-repeating micro-expressions that prevent uncanny-valley
// smoothness. Includes Lyapunov exponent monitoring to ensure the system
// remains in the chaotic regime.

#pragma once

#include "CoreMinimal.h"
#include "LorenzChaoticDynamics.generated.h"

// ============================================================================
// LORENZ ATTRACTOR STATE
// ============================================================================

USTRUCT(BlueprintType)
struct FLorenzState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Position = FVector(1.0f, 1.0f, 1.0f);

    // Lorenz system parameters (classic chaotic regime)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Sigma = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Rho = 28.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Beta = 8.0f / 3.0f;

    // Integration timestep
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DeltaT = 0.01f;
};

// ============================================================================
// CHAOTIC DYNAMICS TELEMETRY
// ============================================================================

USTRUCT(BlueprintType)
struct FChaoticDynamicsTelemetry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    float LyapunovExponent = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float TrajectoryDivergence = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float AttractorEnergy = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    bool bIsInChaoticRegime = true;

    UPROPERTY(BlueprintReadOnly)
    int32 TotalSteps = 0;

    UPROPERTY(BlueprintReadOnly)
    float ExpressionVolatility = 0.15f;
};

// ============================================================================
// LORENZ CHAOTIC DYNAMICS ENGINE
// ============================================================================

UCLASS(BlueprintType, Blueprintable)
class UNREALECHO_API ULorenzChaoticDynamics : public UObject
{
    GENERATED_BODY()

public:
    ULorenzChaoticDynamics();

    // ========================================================================
    // INITIALIZATION
    // ========================================================================

    /** Initialize with default Lorenz parameters */
    UFUNCTION(BlueprintCallable, Category = "ChaoticDynamics")
    void Initialize();

    /** Initialize with custom parameters */
    UFUNCTION(BlueprintCallable, Category = "ChaoticDynamics")
    void InitializeWithParams(float Sigma, float Rho, float Beta, float DeltaT);

    /** Reset to initial conditions */
    UFUNCTION(BlueprintCallable, Category = "ChaoticDynamics")
    void Reset();

    // ========================================================================
    // STEPPING
    // ========================================================================

    /** Advance the Lorenz system by one RK4 step */
    UFUNCTION(BlueprintCallable, Category = "ChaoticDynamics")
    FVector Step();

    /** Advance by multiple steps, return final position */
    UFUNCTION(BlueprintCallable, Category = "ChaoticDynamics")
    FVector StepMultiple(int32 NumSteps);

    /** Advance by real time (computes appropriate number of steps) */
    UFUNCTION(BlueprintCallable, Category = "ChaoticDynamics")
    FVector StepByTime(float DeltaTime);

    // ========================================================================
    // MICRO-EXPRESSION GENERATION
    // ========================================================================

    /**
     * Generate chaotic noise values for each facial region.
     * Returns a map of AU names to additive noise values.
     * Scale is controlled by ChaosIntensity (default 0.15).
     */
    UFUNCTION(BlueprintCallable, Category = "ChaoticDynamics|Expression")
    TMap<FName, float> GenerateMicroExpressionNoise(float ChaosIntensity = 0.15f);

    /**
     * Generate a single chaotic float in [0, 1] from the current state.
     * Useful for modulating individual parameters.
     */
    UFUNCTION(BlueprintCallable, Category = "ChaoticDynamics|Expression")
    float GetChaoticFloat(int32 Channel = 0) const;

    // ========================================================================
    // MONITORING
    // ========================================================================

    /** Get current Lyapunov exponent estimate */
    UFUNCTION(BlueprintCallable, Category = "ChaoticDynamics|Monitor")
    float GetLyapunovExponent() const;

    /** Check if system is in chaotic regime (positive Lyapunov exponent) */
    UFUNCTION(BlueprintCallable, Category = "ChaoticDynamics|Monitor")
    bool IsInChaoticRegime() const;

    /** Get full telemetry */
    UFUNCTION(BlueprintCallable, Category = "ChaoticDynamics|Monitor")
    FChaoticDynamicsTelemetry GetTelemetry() const;

    /** Get current attractor position */
    UFUNCTION(BlueprintCallable, Category = "ChaoticDynamics|Monitor")
    FVector GetCurrentPosition() const { return State.Position; }

    // ========================================================================
    // PARAMETER CONTROL
    // ========================================================================

    /** Set chaos intensity for micro-expression generation */
    UFUNCTION(BlueprintCallable, Category = "ChaoticDynamics|Params")
    void SetExpressionVolatility(float Volatility);

    /** Perturb the system to prevent convergence to periodic orbit */
    UFUNCTION(BlueprintCallable, Category = "ChaoticDynamics|Params")
    void Perturb(float Magnitude = 0.01f);

protected:
    UPROPERTY()
    FLorenzState State;

    UPROPERTY()
    FChaoticDynamicsTelemetry Telemetry;

    // Trajectory history for Lyapunov estimation
    TArray<FVector> TrajectoryHistory;
    static constexpr int32 MAX_HISTORY = 256;

    // Shadow trajectory for Lyapunov computation
    FVector ShadowPosition;
    float ShadowSeparation = 0.001f;

    // ========================================================================
    // INTERNAL: RK4 INTEGRATION
    // ========================================================================

    FVector LorenzDerivative(const FVector& P) const
    {
        return FVector(
            State.Sigma * (P.Y - P.X),
            P.X * (State.Rho - P.Z) - P.Y,
            P.X * P.Y - State.Beta * P.Z
        );
    }

    FVector RK4Step(const FVector& P, float dt) const
    {
        FVector k1 = LorenzDerivative(P) * dt;
        FVector k2 = LorenzDerivative(P + k1 * 0.5f) * dt;
        FVector k3 = LorenzDerivative(P + k2 * 0.5f) * dt;
        FVector k4 = LorenzDerivative(P + k3) * dt;
        return P + (k1 + k2 * 2.0f + k3 * 2.0f + k4) / 6.0f;
    }

    void UpdateLyapunovEstimate()
    {
        // Advance shadow trajectory
        FVector NewShadow = RK4Step(ShadowPosition, State.DeltaT);

        // Compute divergence
        float Dist = FVector::Dist(State.Position, NewShadow);
        if (Dist > 0.0001f)
        {
            Telemetry.TrajectoryDivergence = Dist;

            // Accumulate Lyapunov exponent
            float Lambda = FMath::Loge(Dist / ShadowSeparation) / State.DeltaT;
            float Alpha = 0.01f; // Exponential moving average
            Telemetry.LyapunovExponent = Telemetry.LyapunovExponent * (1.0f - Alpha) + Lambda * Alpha;

            // Renormalize shadow trajectory
            FVector Direction = (NewShadow - State.Position).GetSafeNormal();
            ShadowPosition = State.Position + Direction * ShadowSeparation;
        }
        else
        {
            ShadowPosition = NewShadow;
        }

        Telemetry.bIsInChaoticRegime = (Telemetry.LyapunovExponent > 0.0f);
        Telemetry.AttractorEnergy = State.Position.Size();
    }

    void RecordTrajectory()
    {
        TrajectoryHistory.Add(State.Position);
        if (TrajectoryHistory.Num() > MAX_HISTORY)
        {
            TrajectoryHistory.RemoveAt(0);
        }
    }
};
