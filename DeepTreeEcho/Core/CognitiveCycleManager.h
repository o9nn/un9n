// CognitiveCycleManager.h
// Central orchestration of the 12-step cognitive cycle with 3 concurrent streams
// Implements the echobeats architecture with sys6 triality integration

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CognitiveCycleManager.generated.h"

// Forward declarations
class UDeepTreeEchoReservoir;
class UWisdomCultivation;
class UEmbodied4ECognition;

/**
 * Cognitive mode enumeration
 */
UENUM(BlueprintType)
enum class ECognitiveModeType : uint8
{
    Expressive      UMETA(DisplayName = "Expressive"),
    Reflective      UMETA(DisplayName = "Reflective")
};

/**
 * Cognitive step type enumeration
 */
UENUM(BlueprintType)
enum class ECognitiveStepType : uint8
{
    Perceive        UMETA(DisplayName = "Perceive"),
    Orient          UMETA(DisplayName = "Orient"),
    Reflect         UMETA(DisplayName = "Reflect"),
    Integrate       UMETA(DisplayName = "Integrate"),
    Decide          UMETA(DisplayName = "Decide"),
    Simulate        UMETA(DisplayName = "Simulate"),
    Act             UMETA(DisplayName = "Act"),
    Observe         UMETA(DisplayName = "Observe"),
    Learn           UMETA(DisplayName = "Learn"),
    Consolidate     UMETA(DisplayName = "Consolidate"),
    Anticipate      UMETA(DisplayName = "Anticipate"),
    Transcend       UMETA(DisplayName = "Transcend")
};

/**
 * Stream role enumeration
 */
UENUM(BlueprintType)
enum class EStreamRole : uint8
{
    Perceiving      UMETA(DisplayName = "Perceiving"),
    Acting          UMETA(DisplayName = "Acting"),
    Reflecting      UMETA(DisplayName = "Reflecting")
};

/**
 * Configuration for a single cognitive step
 */
USTRUCT(BlueprintType)
struct FCognitiveStepConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 StepNumber = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECognitiveStepType StepType = ECognitiveStepType::Perceive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECognitiveModeType Mode = ECognitiveModeType::Expressive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PrimaryStreamID = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsPivotalStep = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 NestingLevel = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TriadicGroup = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Description;
};

/**
 * State of a consciousness stream
 */
USTRUCT(BlueprintType)
struct FStreamState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 StreamID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EStreamRole Role = EStreamRole::Perceiving;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CurrentPhase = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PhaseOffset = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ActivationLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Coherence = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<float> ReservoirState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString CurrentFocus;
};

/**
 * Triadic synchronization state
 */
USTRUCT(BlueprintType)
struct FTriadicSyncState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TriadIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<int32> TriadSteps; // e.g., {1, 5, 9}

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SyncQuality = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PhaseAlignment = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<float> CoherenceMatrix; // 3x3 matrix flattened
};

/**
 * Sys6 triality state for 30-step cycle
 */
USTRUCT(BlueprintType)
struct FSys6TrialityState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CurrentStep = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CurrentPhase = 1; // 1-3

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CurrentStage = 1; // 1-5

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 DyadState = 0; // 0=A, 1=B

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TriadState = 1; // 1-3

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<int32> ActiveThreads;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EntanglementLevel = 0.0f;
};

/**
 * Overall cognitive cycle state
 */
USTRUCT(BlueprintType)
struct FCognitiveCycleState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CurrentStep = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECognitiveModeType CurrentMode = ECognitiveModeType::Expressive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CycleCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CycleProgress = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FStreamState> Streams;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FTriadicSyncState TriadicSync;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FSys6TrialityState Sys6State;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float OverallCoherence = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RelevanceRealizationLevel = 0.0f;
};

/**
 * Delegate declarations
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCycleStepChanged, int32, OldStep, int32, NewStep);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnModeChanged, ECognitiveModeType, OldMode, ECognitiveModeType, NewMode);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPivotalStepReached, int32, Step);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTriadicSync, int32, TriadIndex, float, SyncQuality);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCycleCompleted, int32, CycleCount);

/**
 * Central manager for the 12-step cognitive cycle
 * Orchestrates 3 concurrent consciousness streams with triadic synchronization
 */
UCLASS(ClassGroup=(DeepTreeEcho), meta=(BlueprintSpawnableComponent))
class UNREALECHO_API UCognitiveCycleManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UCognitiveCycleManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, 
                               FActorComponentTickFunction* ThisTickFunction) override;

    // ========================================
    // CONFIGURATION
    // ========================================

    /** Duration of one complete 12-step cycle in seconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cycle Configuration")
    float CycleDuration = 12.0f;

    /** Enable automatic cycle progression */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cycle Configuration")
    bool bEnableAutoCycle = true;

    /** Enable triadic synchronization */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cycle Configuration")
    bool bEnableTriadicSync = true;

    /** Enable sys6 triality integration */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cycle Configuration")
    bool bEnableSys6Triality = true;

    /** Strength of inter-stream coupling at sync points */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cycle Configuration", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float StreamCouplingStrength = 0.3f;

    // ========================================
    // EVENTS
    // ========================================

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnCycleStepChanged OnStepChanged;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnModeChanged OnModeChanged;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnPivotalStepReached OnPivotalStep;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnTriadicSync OnTriadicSync;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnCycleCompleted OnCycleCompleted;

    // ========================================
    // CYCLE CONTROL
    // ========================================

    /** Initialize the cognitive cycle */
    UFUNCTION(BlueprintCallable, Category = "Cycle Control")
    void InitializeCycle();

    /** Advance to the next step */
    UFUNCTION(BlueprintCallable, Category = "Cycle Control")
    void AdvanceStep();

    /** Jump to a specific step */
    UFUNCTION(BlueprintCallable, Category = "Cycle Control")
    void JumpToStep(int32 Step);

    /** Pause the cycle */
    UFUNCTION(BlueprintCallable, Category = "Cycle Control")
    void PauseCycle();

    /** Resume the cycle */
    UFUNCTION(BlueprintCallable, Category = "Cycle Control")
    void ResumeCycle();

    /** Reset the cycle to step 1 */
    UFUNCTION(BlueprintCallable, Category = "Cycle Control")
    void ResetCycle();

    // ========================================
    // STATE QUERIES
    // ========================================

    /** Get the current step number (1-12) */
    UFUNCTION(BlueprintPure, Category = "State")
    int32 GetCurrentStep() const;

    /** Get the current cognitive mode */
    UFUNCTION(BlueprintPure, Category = "State")
    ECognitiveModeType GetCurrentMode() const;

    /** Get the configuration for the current step */
    UFUNCTION(BlueprintPure, Category = "State")
    FCognitiveStepConfig GetCurrentStepConfig() const;

    /** Get the configuration for a specific step */
    UFUNCTION(BlueprintPure, Category = "State")
    FCognitiveStepConfig GetStepConfig(int32 Step) const;

    /** Get the full cycle state */
    UFUNCTION(BlueprintPure, Category = "State")
    FCognitiveCycleState GetCycleState() const;

    /** Check if current step is a pivotal step */
    UFUNCTION(BlueprintPure, Category = "State")
    bool IsCurrentStepPivotal() const;

    /** Check if we're at a triadic sync point */
    UFUNCTION(BlueprintPure, Category = "State")
    bool IsTriadicSyncPoint() const;

    /** Get the current triadic group (0-3) */
    UFUNCTION(BlueprintPure, Category = "State")
    int32 GetCurrentTriadicGroup() const;

    // ========================================
    // STREAM MANAGEMENT
    // ========================================

    /** Get the state of a specific stream */
    UFUNCTION(BlueprintPure, Category = "Streams")
    FStreamState GetStreamState(int32 StreamID) const;

    /** Get the active stream for the current step */
    UFUNCTION(BlueprintPure, Category = "Streams")
    int32 GetActiveStreamID() const;

    /** Update a stream's focus */
    UFUNCTION(BlueprintCallable, Category = "Streams")
    void SetStreamFocus(int32 StreamID, const FString& Focus);

    /** Get inter-stream coherence */
    UFUNCTION(BlueprintPure, Category = "Streams")
    float GetInterStreamCoherence() const;

    /** Force synchronization of all streams */
    UFUNCTION(BlueprintCallable, Category = "Streams")
    void ForceSynchronize();

    // ========================================
    // SYS6 TRIALITY
    // ========================================

    /** Get the current sys6 state */
    UFUNCTION(BlueprintPure, Category = "Sys6")
    FSys6TrialityState GetSys6State() const;

    /** Get the current dyad state (A or B) */
    UFUNCTION(BlueprintPure, Category = "Sys6")
    FString GetCurrentDyadState() const;

    /** Get the current triad state (1, 2, or 3) */
    UFUNCTION(BlueprintPure, Category = "Sys6")
    int32 GetCurrentTriadState() const;

    /** Get the entanglement level */
    UFUNCTION(BlueprintPure, Category = "Sys6")
    float GetEntanglementLevel() const;

    // ========================================
    // INTEGRATION
    // ========================================

    /** Process input through the current step */
    UFUNCTION(BlueprintCallable, Category = "Processing")
    TArray<float> ProcessInput(const TArray<float>& Input);

    /** Get the relevance realization level */
    UFUNCTION(BlueprintPure, Category = "Processing")
    float GetRelevanceRealizationLevel() const;

protected:
    // Component references
    UPROPERTY()
    UDeepTreeEchoReservoir* ReservoirComponent;

    UPROPERTY()
    UWisdomCultivation* WisdomComponent;

    UPROPERTY()
    UEmbodied4ECognition* Embodied4EComponent;

    // Internal state
    FCognitiveCycleState CycleState;
    TArray<FCognitiveStepConfig> StepConfigs;

    float StepTimer = 0.0f;
    float Sys6Timer = 0.0f;
    bool bIsPaused = false;

    // Internal methods
    void InitializeStepConfigs();
    void InitializeStreams();
    void InitializeSys6State();

    void UpdateCycleTimer(float DeltaTime);
    void UpdateStreams(float DeltaTime);
    void UpdateSys6State(float DeltaTime);

    void ProcessStepTransition(int32 OldStep, int32 NewStep);
    void ProcessTriadicSync();
    void ProcessModeTransition(ECognitiveModeType OldMode, ECognitiveModeType NewMode);

    void SynchronizeStreamsAtTriad();
    void UpdateStreamCoherence();
    void PropagateActivation();

    void UpdateSys6DoubleStepPattern();
    void UpdateThreadMultiplexing();

    float ComputeStreamCoherence(int32 StreamA, int32 StreamB) const;
    float ComputeOverallCoherence() const;
    float ComputeRelevanceRealization() const;

    void FindComponentReferences();
};
