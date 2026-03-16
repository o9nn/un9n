#pragma once
/**
 * @file AvatarStudio3D.h
 * @brief 3D Avatar Design Studio for Deep Tree Echo
 *
 * A comprehensive avatar creation, customization, and animation authoring system.
 * This is the top-level "studio" component that composes all avatar subsystems
 * into a unified design-time and runtime experience.
 *
 * Architecture:
 *
 *   ┌─────────────────────────────────────────────────────────────────┐
 *   │                    AvatarStudio3D                               │
 *   │                                                                 │
 *   │  ┌──────────────┐  ┌──────────────┐  ┌──────────────────────┐  │
 *   │  │ Character     │  │ Expression   │  │ Animation            │  │
 *   │  │ Designer      │  │ Editor       │  │ Sequencer            │  │
 *   │  │              │  │              │  │                      │  │
 *   │  │ - Body morph │  │ - FACS AU    │  │ - Timeline           │  │
 *   │  │ - Face sculpt│  │   sliders    │  │ - Keyframes          │  │
 *   │  │ - Hair/skin  │  │ - Emotion    │  │ - Blend curves       │  │
 *   │  │ - Clothing   │  │   presets    │  │ - Procedural layers  │  │
 *   │  │ - Aesthetic   │  │ - Live       │  │ - Export (FBX/USD)   │  │
 *   │  │   presets    │  │   preview    │  │                      │  │
 *   │  └──────┬───────┘  └──────┬───────┘  └──────────┬───────────┘  │
 *   │         │                 │                      │              │
 *   │         ▼                 ▼                      ▼              │
 *   │  ┌─────────────────────────────────────────────────────────┐    │
 *   │  │              CognitiveDNACalibration                     │    │
 *   │  │  ┌─────────────┐  ┌──────────────┐  ┌───────────────┐  │    │
 *   │  │  │ RigLogic    │  │ Corrective   │  │ Cognitive     │  │    │
 *   │  │  │ Evaluator   │  │ Expressions  │  │ LOD Manager   │  │    │
 *   │  │  └──────┬──────┘  └──────┬───────┘  └───────┬───────┘  │    │
 *   │  │         │                │                   │          │    │
 *   │  │         ▼                ▼                   ▼          │    │
 *   │  │  ┌─────────────────────────────────────────────────┐    │    │
 *   │  │  │         FACSToMetaHumanMapping (70+ AUs)         │    │    │
 *   │  │  └─────────────────────────────────────────────────┘    │    │
 *   │  └─────────────────────────────────────────────────────────┘    │
 *   │                            │                                    │
 *   │                            ▼                                    │
 *   │  ┌─────────────────────────────────────────────────────────┐    │
 *   │  │              MasterOrchestrator                          │    │
 *   │  │  ┌──────────┐ ┌──────────┐ ┌────────┐ ┌─────────────┐  │    │
 *   │  │  │ OpenCog  │ │ UnrealAI │ │  NPU   │ │ HyperSkill  │  │    │
 *   │  │  │ Bridge   │ │ Bridge   │ │ Bridge │ │ Tensor      │  │    │
 *   │  │  └──────────┘ └──────────┘ └────────┘ └─────────────┘  │    │
 *   │  └─────────────────────────────────────────────────────────┘    │
 *   └─────────────────────────────────────────────────────────────────┘
 *
 * Deployment targets:
 * - UE5 Editor Plugin (full 3D viewport with MetaHuman preview)
 * - Web-based Studio (Three.js/Babylon.js with Live2D fallback)
 * - DeltaChat companion (Live2D Cubism parameter mapping)
 * - Cloudflare Workers (CogMorph serialized state)
 *
 * @see AvatarDesignStudio.h (existing design-time component)
 * @see CognitiveDNACalibration.h (DNA pipeline)
 * @see RigLogicEvaluator.h (runtime evaluation)
 */

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RigLogicEvaluator.h"
#include "CognitiveDNACalibration.h"
#include "CognitiveLODManager.h"
#include "AvatarStudio3D.generated.h"

class UMasterOrchestrator;
class UFACSToMetaHumanMapping;

// ============================================================================
// Character Design Preset
// ============================================================================
USTRUCT(BlueprintType)
struct FCharacterDesignPreset
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString PresetName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Description;

    /** Body morph parameters (height, build, proportions) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FName, float> BodyMorphs;

    /** Face sculpt parameters (bone structure, features) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FName, float> FaceSculptParams;

    /** Aesthetic overlay preset */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FAestheticOverlay AestheticPreset;

    /** Skin material parameters */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FName, float> SkinParams;

    /** Hair parameters */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FName, float> HairParams;

    /** Eye parameters */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FName, float> EyeParams;
};

// ============================================================================
// Expression Editing State
// ============================================================================
USTRUCT(BlueprintType)
struct FExpressionEditState
{
    GENERATED_BODY()

    /** Current FACS AU values being edited */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FName, float> CurrentAUs;

    /** Active emotion preset (if any) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ActiveEmotionPreset;

    /** Whether live preview is enabled */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bLivePreview = true;

    /** Preview blend speed */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PreviewBlendSpeed = 5.0f;
};

// ============================================================================
// Animation Keyframe
// ============================================================================
USTRUCT(BlueprintType)
struct FAnimationKeyframe
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeSeconds = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FName, float> AUValues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FAestheticOverlay AestheticState;

    /** Interpolation type for this keyframe */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString InterpType = TEXT("CubicBezier");
};

// ============================================================================
// Animation Sequence
// ============================================================================
USTRUCT(BlueprintType)
struct FAnimationSequence
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SequenceName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Duration = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FAnimationKeyframe> Keyframes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bLoop = false;

    /** Procedural layers (chaos, breathing, micro-expressions) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, float> ProceduralLayerWeights;
};

// ============================================================================
// Studio Export Format
// ============================================================================
UENUM(BlueprintType)
enum class EStudioExportFormat : uint8
{
    MetaHumanDNA,    // .dna binary
    FBX,             // Autodesk FBX
    USD,             // Universal Scene Description
    GLTF,            // GL Transmission Format (web)
    Live2DCubism,    // Live2D Cubism model3.json
    CogMorphGGUF,    // CogMorph serialized state
    JSON             // Raw parameter JSON
};

// ============================================================================
// Studio Session State
// ============================================================================
USTRUCT(BlueprintType)
struct FStudioSessionState
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString SessionId;

    UPROPERTY(BlueprintReadOnly)
    FDateTime StartTime;

    UPROPERTY(BlueprintReadOnly)
    int32 UndoStackDepth = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 RedoStackDepth = 0;

    UPROPERTY(BlueprintReadOnly)
    FString ActiveTool;

    UPROPERTY(BlueprintReadOnly)
    bool bModified = false;
};

// ============================================================================
// Avatar Studio 3D Component
// ============================================================================
UCLASS(ClassGroup=(DeepTreeEcho), meta=(BlueprintSpawnableComponent))
class UAvatarStudio3D : public UActorComponent
{
    GENERATED_BODY()

public:
    UAvatarStudio3D();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

    // --- Session Management ---

    UFUNCTION(BlueprintCallable, Category="Studio|Session")
    void StartNewSession();

    UFUNCTION(BlueprintCallable, Category="Studio|Session")
    FStudioSessionState GetSessionState() const;

    UFUNCTION(BlueprintCallable, Category="Studio|Session")
    void Undo();

    UFUNCTION(BlueprintCallable, Category="Studio|Session")
    void Redo();

    // --- Character Designer ---

    UFUNCTION(BlueprintCallable, Category="Studio|Character")
    void ApplyCharacterPreset(const FCharacterDesignPreset& Preset);

    UFUNCTION(BlueprintCallable, Category="Studio|Character")
    FCharacterDesignPreset GetCurrentCharacterState() const;

    UFUNCTION(BlueprintCallable, Category="Studio|Character")
    void SetBodyMorph(FName MorphName, float Value);

    UFUNCTION(BlueprintCallable, Category="Studio|Character")
    void SetFaceSculptParam(FName ParamName, float Value);

    UFUNCTION(BlueprintCallable, Category="Studio|Character")
    TArray<FCharacterDesignPreset> GetBuiltInPresets() const;

    // --- Expression Editor ---

    UFUNCTION(BlueprintCallable, Category="Studio|Expression")
    void SetExpressionAU(FName AUName, float Value);

    UFUNCTION(BlueprintCallable, Category="Studio|Expression")
    void ApplyEmotionPreset(FName EmotionName);

    UFUNCTION(BlueprintCallable, Category="Studio|Expression")
    FExpressionEditState GetExpressionState() const;

    UFUNCTION(BlueprintCallable, Category="Studio|Expression")
    void ResetExpression();

    UFUNCTION(BlueprintCallable, Category="Studio|Expression")
    TArray<FName> GetAvailableEmotions() const;

    // --- Animation Sequencer ---

    UFUNCTION(BlueprintCallable, Category="Studio|Animation")
    void CreateNewSequence(const FString& Name, float Duration);

    UFUNCTION(BlueprintCallable, Category="Studio|Animation")
    void AddKeyframe(float TimeSeconds);

    UFUNCTION(BlueprintCallable, Category="Studio|Animation")
    void RemoveKeyframe(int32 Index);

    UFUNCTION(BlueprintCallable, Category="Studio|Animation")
    void PlaySequence();

    UFUNCTION(BlueprintCallable, Category="Studio|Animation")
    void StopSequence();

    UFUNCTION(BlueprintCallable, Category="Studio|Animation")
    void SetSequenceTime(float TimeSeconds);

    UFUNCTION(BlueprintCallable, Category="Studio|Animation")
    FAnimationSequence GetCurrentSequence() const;

    // --- Export ---

    UFUNCTION(BlueprintCallable, Category="Studio|Export")
    bool ExportCharacter(const FString& FilePath, EStudioExportFormat Format);

    UFUNCTION(BlueprintCallable, Category="Studio|Export")
    bool ExportAnimation(const FString& FilePath, EStudioExportFormat Format);

    /** Export to Live2D Cubism parameters (for DeltaChat/web deployment) */
    UFUNCTION(BlueprintCallable, Category="Studio|Export")
    TMap<FString, float> ExportAsLive2DParams() const;

    /** Export to CogMorph GGUF (for Cloudflare Workers deployment) */
    UFUNCTION(BlueprintCallable, Category="Studio|Export")
    bool ExportAsCogMorph(const FString& FilePath);

protected:
    // Subsystem references
    UPROPERTY()
    URigLogicEvaluator* RigLogic;

    UPROPERTY()
    UCognitiveDNACalibration* DNACalibration;

    UPROPERTY()
    UCognitiveLODManager* LODManager;

    UPROPERTY()
    UMasterOrchestrator* Orchestrator;

    // Session state
    FStudioSessionState SessionState;

    // Expression editing
    FExpressionEditState ExpressionState;

    // Animation
    FAnimationSequence CurrentSequence;
    bool bPlayingSequence;
    float SequencePlaybackTime;

    // Character design
    FCharacterDesignPreset CurrentDesign;

    // Undo/Redo stacks
    TArray<FCalibrationSnapshot> UndoStack;
    TArray<FCalibrationSnapshot> RedoStack;

    void AutoDiscoverSubsystems();
    void InitializeBuiltInPresets();
    void UpdateSequencePlayback(float DeltaTime);
    FAnimationKeyframe InterpolateKeyframes(const FAnimationKeyframe& A,
                                             const FAnimationKeyframe& B,
                                             float Alpha) const;
    void PushUndoState();

    // Built-in presets
    TArray<FCharacterDesignPreset> BuiltInPresets;
};
