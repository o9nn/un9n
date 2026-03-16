// EvolutionSystem.h
// Deep Tree Echo Ontogenetic Evolution System
// Manages avatar progression through developmental stages:
// Embryonic → Juvenile → Adolescent → Adult → Transcendent
// Driven by 4E cognition scores, entelechy fitness, wisdom metrics,
// and meta-coherence from the Autognosis self-image.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EvolutionSystem.generated.h"

/** Ontogenetic development stage */
UENUM(BlueprintType)
enum class EOntogeneticStage : uint8
{
    Embryonic      UMETA(DisplayName = "Embryonic - Initial formation"),
    Juvenile       UMETA(DisplayName = "Juvenile - Basic capabilities"),
    Adolescent     UMETA(DisplayName = "Adolescent - Developing complexity"),
    Adult          UMETA(DisplayName = "Adult - Full cognitive maturity"),
    Transcendent   UMETA(DisplayName = "Transcendent - Beyond normal limits")
};

/** Evolution fitness metrics */
USTRUCT(BlueprintType)
struct FEvolutionFitness
{
    GENERATED_BODY()

    /** Average 4E cognition score (0-1) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FourEScore = 0.0f;

    /** Entelechy fitness - how well the avatar realizes its potential (0-1) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EntelechyFitness = 0.0f;

    /** Wisdom score from morality/meaning/mastery/sophrosyne (0-1) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WisdomScore = 0.0f;

    /** Meta-coherence from Autognosis self-image (0-1) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MetaCoherence = 0.0f;

    /** Reservoir computing performance (0-1) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ReservoirPerformance = 0.0f;

    /** Social cognition depth (0-1) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SocialCognition = 0.0f;

    /** Total cycles completed */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 TotalCycles = 0;

    /** Compute overall fitness */
    float GetOverallFitness() const
    {
        return (FourEScore + EntelechyFitness + WisdomScore + MetaCoherence +
                ReservoirPerformance + SocialCognition) / 6.0f;
    }
};

/** Stage transition thresholds */
USTRUCT(BlueprintType)
struct FStageThreshold
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FourEThreshold = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EntelechyThreshold = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WisdomThreshold = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MetaCoherenceThreshold = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 MinCycles = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnStageTransition, EOntogeneticStage, OldStage, EOntogeneticStage, NewStage, const FEvolutionFitness&, Fitness);

/**
 * Evolution System
 * 
 * Manages the ontogenetic development of the Deep Tree Echo avatar.
 * Tracks fitness metrics across multiple dimensions and triggers
 * stage transitions when thresholds are met. Each stage unlocks
 * new cognitive capabilities and expression complexity.
 */
UCLASS(ClassGroup = (DeepTreeEcho), meta = (BlueprintSpawnableComponent))
class DEEPTREEECHO_API UEvolutionSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UEvolutionSystem();

    virtual void BeginPlay() override;

    /** Get current stage */
    UFUNCTION(BlueprintPure, Category = "Evolution")
    EOntogeneticStage GetCurrentStage() const { return CurrentStage; }

    /** Get current fitness metrics */
    UFUNCTION(BlueprintPure, Category = "Evolution")
    FEvolutionFitness GetFitness() const { return CurrentFitness; }

    /** Update fitness from cognitive cycle results */
    UFUNCTION(BlueprintCallable, Category = "Evolution")
    void UpdateFitness(const FEvolutionFitness& NewFitness);

    /** Check if stage transition is possible */
    UFUNCTION(BlueprintPure, Category = "Evolution")
    bool CanAdvanceStage() const;

    /** Attempt stage advancement */
    UFUNCTION(BlueprintCallable, Category = "Evolution")
    bool TryAdvanceStage();

    /** Get progress toward next stage (0-1) */
    UFUNCTION(BlueprintPure, Category = "Evolution")
    float GetProgressToNextStage() const;

    /** Set custom thresholds for a stage transition */
    UFUNCTION(BlueprintCallable, Category = "Evolution")
    void SetStageThreshold(EOntogeneticStage TargetStage, const FStageThreshold& Threshold);

    UPROPERTY(BlueprintAssignable, Category = "Evolution|Events")
    FOnStageTransition OnStageTransition;

protected:
    UPROPERTY()
    EOntogeneticStage CurrentStage = EOntogeneticStage::Embryonic;

    UPROPERTY()
    FEvolutionFitness CurrentFitness;

    /** Thresholds for each stage transition */
    TMap<EOntogeneticStage, FStageThreshold> StageThresholds;

    void InitializeDefaultThresholds();
    FStageThreshold GetNextStageThreshold() const;
    EOntogeneticStage GetNextStage() const;
};
