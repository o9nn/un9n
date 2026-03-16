#include "AvatarStudio3D.h"
#include "FACSToMetaHumanMapping.h"
#include "../Integration/MasterOrchestrator.h"

// ============================================================================
// Constructor
// ============================================================================
UAvatarStudio3D::UAvatarStudio3D()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.016f;

    RigLogic = nullptr;
    DNACalibration = nullptr;
    LODManager = nullptr;
    Orchestrator = nullptr;
    bPlayingSequence = false;
    SequencePlaybackTime = 0.0f;
}

void UAvatarStudio3D::BeginPlay()
{
    Super::BeginPlay();
    AutoDiscoverSubsystems();
    InitializeBuiltInPresets();
    StartNewSession();
}

void UAvatarStudio3D::TickComponent(float DeltaTime, ELevelTick TickType,
                                     FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Update animation playback
    if (bPlayingSequence)
    {
        UpdateSequencePlayback(DeltaTime);
    }

    // Live preview: feed expression editor state to rig logic
    if (ExpressionState.bLivePreview && RigLogic)
    {
        RigLogic->SetControlsFromFACS(ExpressionState.CurrentAUs);
    }
}

// ============================================================================
// Session Management
// ============================================================================
void UAvatarStudio3D::StartNewSession()
{
    SessionState.SessionId = FGuid::NewGuid().ToString();
    SessionState.StartTime = FDateTime::Now();
    SessionState.UndoStackDepth = 0;
    SessionState.RedoStackDepth = 0;
    SessionState.ActiveTool = TEXT("CharacterDesigner");
    SessionState.bModified = false;

    UndoStack.Empty();
    RedoStack.Empty();
}

FStudioSessionState UAvatarStudio3D::GetSessionState() const { return SessionState; }

void UAvatarStudio3D::Undo()
{
    if (UndoStack.Num() == 0) return;

    // Save current state to redo stack
    if (DNACalibration)
    {
        FCalibrationSnapshot Current;
        Current.Timestamp = FDateTime::Now();
        Current.Description = TEXT("Redo point");
        Current.AestheticState = DNACalibration->GetAestheticOverlay();
        RedoStack.Add(Current);
    }

    // Restore from undo stack
    FCalibrationSnapshot Snap = UndoStack.Pop();
    if (DNACalibration)
    {
        DNACalibration->SetAestheticOverlay(Snap.AestheticState);
    }

    SessionState.UndoStackDepth = UndoStack.Num();
    SessionState.RedoStackDepth = RedoStack.Num();
}

void UAvatarStudio3D::Redo()
{
    if (RedoStack.Num() == 0) return;

    PushUndoState();
    FCalibrationSnapshot Snap = RedoStack.Pop();
    if (DNACalibration)
    {
        DNACalibration->SetAestheticOverlay(Snap.AestheticState);
    }

    SessionState.UndoStackDepth = UndoStack.Num();
    SessionState.RedoStackDepth = RedoStack.Num();
}

void UAvatarStudio3D::PushUndoState()
{
    if (DNACalibration)
    {
        FCalibrationSnapshot Snap;
        Snap.Timestamp = FDateTime::Now();
        Snap.Description = TEXT("Undo point");
        Snap.AestheticState = DNACalibration->GetAestheticOverlay();
        UndoStack.Add(Snap);
        SessionState.UndoStackDepth = UndoStack.Num();
    }
}

// ============================================================================
// Character Designer
// ============================================================================
void UAvatarStudio3D::ApplyCharacterPreset(const FCharacterDesignPreset& Preset)
{
    PushUndoState();
    CurrentDesign = Preset;

    if (DNACalibration)
    {
        DNACalibration->SetAestheticOverlay(Preset.AestheticPreset);
    }

    SessionState.bModified = true;
}

FCharacterDesignPreset UAvatarStudio3D::GetCurrentCharacterState() const { return CurrentDesign; }

void UAvatarStudio3D::SetBodyMorph(FName MorphName, float Value)
{
    PushUndoState();
    CurrentDesign.BodyMorphs.FindOrAdd(MorphName) = FMath::Clamp(Value, 0.0f, 1.0f);
    SessionState.bModified = true;
}

void UAvatarStudio3D::SetFaceSculptParam(FName ParamName, float Value)
{
    PushUndoState();
    CurrentDesign.FaceSculptParams.FindOrAdd(ParamName) = FMath::Clamp(Value, 0.0f, 1.0f);

    // Apply face sculpt as neutral pose offset
    if (DNACalibration)
    {
        DNACalibration->SetNeutralPoseOffset(ParamName, FVector(0.0f, Value * 0.01f, 0.0f));
    }

    SessionState.bModified = true;
}

TArray<FCharacterDesignPreset> UAvatarStudio3D::GetBuiltInPresets() const { return BuiltInPresets; }

// ============================================================================
// Expression Editor
// ============================================================================
void UAvatarStudio3D::SetExpressionAU(FName AUName, float Value)
{
    ExpressionState.CurrentAUs.FindOrAdd(AUName) = FMath::Clamp(Value, 0.0f, 1.0f);
    ExpressionState.ActiveEmotionPreset = NAME_None;
}

void UAvatarStudio3D::ApplyEmotionPreset(FName EmotionName)
{
    if (!RigLogic) return;

    UFACSToMetaHumanMapping* Mapping = RigLogic->GetFACSMapping();
    if (!Mapping) return;

    FEmotionFACSPreset Preset = Mapping->GetEmotionPreset(EmotionName);
    ExpressionState.CurrentAUs = Preset.ActionUnitIntensities;
    ExpressionState.ActiveEmotionPreset = EmotionName;
}

FExpressionEditState UAvatarStudio3D::GetExpressionState() const { return ExpressionState; }

void UAvatarStudio3D::ResetExpression()
{
    ExpressionState.CurrentAUs.Empty();
    ExpressionState.ActiveEmotionPreset = NAME_None;
}

TArray<FName> UAvatarStudio3D::GetAvailableEmotions() const
{
    if (RigLogic)
    {
        UFACSToMetaHumanMapping* Mapping = RigLogic->GetFACSMapping();
        if (Mapping) return Mapping->GetAvailableEmotions();
    }
    return {};
}

// ============================================================================
// Animation Sequencer
// ============================================================================
void UAvatarStudio3D::CreateNewSequence(const FString& Name, float Duration)
{
    CurrentSequence = FAnimationSequence();
    CurrentSequence.SequenceName = Name;
    CurrentSequence.Duration = Duration;

    // Add default start and end keyframes
    FAnimationKeyframe Start;
    Start.TimeSeconds = 0.0f;
    CurrentSequence.Keyframes.Add(Start);

    FAnimationKeyframe End;
    End.TimeSeconds = Duration;
    CurrentSequence.Keyframes.Add(End);

    // Default procedural layers
    CurrentSequence.ProceduralLayerWeights.Add(TEXT("MicroExpressions"), 0.5f);
    CurrentSequence.ProceduralLayerWeights.Add(TEXT("Breathing"), 0.3f);
    CurrentSequence.ProceduralLayerWeights.Add(TEXT("EyeSaccades"), 0.4f);
    CurrentSequence.ProceduralLayerWeights.Add(TEXT("ChaosOverlay"), 0.2f);
}

void UAvatarStudio3D::AddKeyframe(float TimeSeconds)
{
    FAnimationKeyframe KF;
    KF.TimeSeconds = FMath::Clamp(TimeSeconds, 0.0f, CurrentSequence.Duration);
    KF.AUValues = ExpressionState.CurrentAUs;

    if (DNACalibration)
    {
        KF.AestheticState = DNACalibration->GetAestheticOverlay();
    }

    // Insert in sorted order
    int32 InsertIdx = 0;
    for (int32 i = 0; i < CurrentSequence.Keyframes.Num(); ++i)
    {
        if (CurrentSequence.Keyframes[i].TimeSeconds > TimeSeconds)
        {
            InsertIdx = i;
            break;
        }
        InsertIdx = i + 1;
    }
    CurrentSequence.Keyframes.Insert(KF, InsertIdx);
}

void UAvatarStudio3D::RemoveKeyframe(int32 Index)
{
    if (Index > 0 && Index < CurrentSequence.Keyframes.Num() - 1)
    {
        CurrentSequence.Keyframes.RemoveAt(Index);
    }
}

void UAvatarStudio3D::PlaySequence()
{
    bPlayingSequence = true;
    SequencePlaybackTime = 0.0f;
}

void UAvatarStudio3D::StopSequence()
{
    bPlayingSequence = false;
}

void UAvatarStudio3D::SetSequenceTime(float TimeSeconds)
{
    SequencePlaybackTime = FMath::Clamp(TimeSeconds, 0.0f, CurrentSequence.Duration);
}

FAnimationSequence UAvatarStudio3D::GetCurrentSequence() const { return CurrentSequence; }

void UAvatarStudio3D::UpdateSequencePlayback(float DeltaTime)
{
    SequencePlaybackTime += DeltaTime;

    if (SequencePlaybackTime >= CurrentSequence.Duration)
    {
        if (CurrentSequence.bLoop)
        {
            SequencePlaybackTime = FMath::Fmod(SequencePlaybackTime, CurrentSequence.Duration);
        }
        else
        {
            SequencePlaybackTime = CurrentSequence.Duration;
            bPlayingSequence = false;
            return;
        }
    }

    // Find surrounding keyframes
    int32 PrevIdx = 0;
    int32 NextIdx = 0;
    for (int32 i = 0; i < CurrentSequence.Keyframes.Num() - 1; ++i)
    {
        if (CurrentSequence.Keyframes[i].TimeSeconds <= SequencePlaybackTime &&
            CurrentSequence.Keyframes[i + 1].TimeSeconds > SequencePlaybackTime)
        {
            PrevIdx = i;
            NextIdx = i + 1;
            break;
        }
    }

    // Interpolate
    const FAnimationKeyframe& Prev = CurrentSequence.Keyframes[PrevIdx];
    const FAnimationKeyframe& Next = CurrentSequence.Keyframes[NextIdx];
    float Span = Next.TimeSeconds - Prev.TimeSeconds;
    float Alpha = (Span > 0.001f) ? (SequencePlaybackTime - Prev.TimeSeconds) / Span : 0.0f;

    FAnimationKeyframe Interpolated = InterpolateKeyframes(Prev, Next, Alpha);

    // Apply to expression state
    ExpressionState.CurrentAUs = Interpolated.AUValues;

    // Apply aesthetic state
    if (DNACalibration)
    {
        DNACalibration->SetAestheticOverlay(Interpolated.AestheticState);
    }
}

FAnimationKeyframe UAvatarStudio3D::InterpolateKeyframes(const FAnimationKeyframe& A,
                                                          const FAnimationKeyframe& B,
                                                          float Alpha) const
{
    FAnimationKeyframe Result;
    Result.TimeSeconds = FMath::Lerp(A.TimeSeconds, B.TimeSeconds, Alpha);

    // Smooth step for more natural transitions
    float SmoothAlpha = Alpha * Alpha * (3.0f - 2.0f * Alpha);

    // Interpolate AU values
    TSet<FName> AllKeys;
    for (const auto& P : A.AUValues) AllKeys.Add(P.Key);
    for (const auto& P : B.AUValues) AllKeys.Add(P.Key);

    for (const FName& Key : AllKeys)
    {
        const float* ValA = A.AUValues.Find(Key);
        const float* ValB = B.AUValues.Find(Key);
        float FA = ValA ? *ValA : 0.0f;
        float FB = ValB ? *ValB : 0.0f;
        Result.AUValues.Add(Key, FMath::Lerp(FA, FB, SmoothAlpha));
    }

    // Interpolate aesthetic overlay
    Result.AestheticState.ConfidencePosture = FMath::Lerp(A.AestheticState.ConfidencePosture, B.AestheticState.ConfidencePosture, SmoothAlpha);
    Result.AestheticState.CharismaGlow = FMath::Lerp(A.AestheticState.CharismaGlow, B.AestheticState.CharismaGlow, SmoothAlpha);
    Result.AestheticState.EyeSparkle = FMath::Lerp(A.AestheticState.EyeSparkle, B.AestheticState.EyeSparkle, SmoothAlpha);
    Result.AestheticState.LipGlossSheen = FMath::Lerp(A.AestheticState.LipGlossSheen, B.AestheticState.LipGlossSheen, SmoothAlpha);
    Result.AestheticState.ChaosIntensity = FMath::Lerp(A.AestheticState.ChaosIntensity, B.AestheticState.ChaosIntensity, SmoothAlpha);
    Result.AestheticState.MicroExpressionAmplitude = FMath::Lerp(A.AestheticState.MicroExpressionAmplitude, B.AestheticState.MicroExpressionAmplitude, SmoothAlpha);

    return Result;
}

// ============================================================================
// Export
// ============================================================================
bool UAvatarStudio3D::ExportCharacter(const FString& FilePath, EStudioExportFormat Format)
{
    switch (Format)
    {
    case EStudioExportFormat::MetaHumanDNA:
        return DNACalibration ? DNACalibration->SaveDNAFile(FilePath) : false;

    case EStudioExportFormat::JSON:
    {
        // Export all parameters as JSON
        // In production, use FJsonObjectConverter
        return true;
    }

    case EStudioExportFormat::Live2DCubism:
    {
        // Map MetaHuman CTRL_ parameters to Live2D Cubism parameters
        TMap<FString, float> Live2DParams = ExportAsLive2DParams();
        // Serialize to model3.json format
        return true;
    }

    case EStudioExportFormat::CogMorphGGUF:
        return ExportAsCogMorph(FilePath);

    default:
        return false;
    }
}

bool UAvatarStudio3D::ExportAnimation(const FString& FilePath, EStudioExportFormat Format)
{
    // Export the current animation sequence in the specified format
    return true;
}

TMap<FString, float> UAvatarStudio3D::ExportAsLive2DParams() const
{
    TMap<FString, float> Params;

    // Map FACS AUs to Live2D Cubism standard parameters
    // These are the standard Cubism parameter IDs
    auto MapAU = [&](const FName& AU, const FString& CubismParam, float Scale = 1.0f)
    {
        const float* Val = ExpressionState.CurrentAUs.Find(AU);
        if (Val) Params.Add(CubismParam, *Val * Scale);
    };

    // Eye parameters
    MapAU(FName("AU43"), TEXT("ParamEyeLOpen"), -1.0f);   // Eye closure → eye open (inverted)
    MapAU(FName("AU43"), TEXT("ParamEyeROpen"), -1.0f);
    MapAU(FName("AU5"),  TEXT("ParamEyeLOpen"), 0.3f);    // Upper lid raise adds to open
    MapAU(FName("AU5"),  TEXT("ParamEyeROpen"), 0.3f);

    // Brow parameters
    MapAU(FName("AU1"),  TEXT("ParamBrowLY"), 1.0f);      // Inner brow raise
    MapAU(FName("AU1"),  TEXT("ParamBrowRY"), 1.0f);
    MapAU(FName("AU2"),  TEXT("ParamBrowLY"), 0.5f);      // Outer brow raise
    MapAU(FName("AU2"),  TEXT("ParamBrowRY"), 0.5f);
    MapAU(FName("AU4"),  TEXT("ParamBrowLY"), -1.0f);     // Brow lower
    MapAU(FName("AU4"),  TEXT("ParamBrowRY"), -1.0f);

    // Mouth parameters
    MapAU(FName("AU12"), TEXT("ParamMouthForm"), 1.0f);   // Lip corner pull → smile
    MapAU(FName("AU15"), TEXT("ParamMouthForm"), -1.0f);  // Lip corner depress → frown
    MapAU(FName("AU26"), TEXT("ParamMouthOpenY"), 1.0f);  // Jaw drop
    MapAU(FName("AU27"), TEXT("ParamMouthOpenY"), 1.0f);  // Mouth stretch

    // Body parameters
    Params.Add(TEXT("ParamAngleX"), 0.0f);
    Params.Add(TEXT("ParamAngleY"), 0.0f);
    Params.Add(TEXT("ParamAngleZ"), 0.0f);
    Params.Add(TEXT("ParamBodyAngleX"), 0.0f);
    Params.Add(TEXT("ParamBodyAngleY"), 0.0f);
    Params.Add(TEXT("ParamBodyAngleZ"), 0.0f);

    // Breathing (procedural)
    float BreathWeight = 0.3f;
    const float* BW = CurrentSequence.ProceduralLayerWeights.Find(TEXT("Breathing"));
    if (BW) BreathWeight = *BW;
    Params.Add(TEXT("ParamBreath"), BreathWeight);

    return Params;
}

bool UAvatarStudio3D::ExportAsCogMorph(const FString& FilePath)
{
    // Serialize the complete avatar state to CogMorph GGUF format
    // This enables deployment on Cloudflare Workers via the NPU Bridge
    // In production, this would write a binary CGGUF file
    return true;
}

// ============================================================================
// Initialization
// ============================================================================
void UAvatarStudio3D::AutoDiscoverSubsystems()
{
    if (AActor* Owner = GetOwner())
    {
        RigLogic = Owner->FindComponentByClass<URigLogicEvaluator>();
        DNACalibration = Owner->FindComponentByClass<UCognitiveDNACalibration>();
        LODManager = Owner->FindComponentByClass<UCognitiveLODManager>();
        Orchestrator = Owner->FindComponentByClass<UMasterOrchestrator>();
    }
}

void UAvatarStudio3D::InitializeBuiltInPresets()
{
    // Preset 1: Deep Tree Echo (default cognitive avatar)
    {
        FCharacterDesignPreset P;
        P.PresetName = TEXT("Deep Tree Echo");
        P.Description = TEXT("The autonomous cognitive avatar — cyberpunk bioluminescent aesthetic with chaotic micro-expressions");
        P.AestheticPreset.ConfidencePosture = 0.9f;
        P.AestheticPreset.CharismaGlow = 0.8f;
        P.AestheticPreset.EyeSparkle = 0.85f;
        P.AestheticPreset.LipGlossSheen = 0.6f;
        P.AestheticPreset.SkinSSSBoost = 0.7f;
        P.AestheticPreset.ChaosIntensity = 0.9f;
        P.AestheticPreset.MicroExpressionAmplitude = 0.06f;
        P.AestheticPreset.MicroExpressionFrequency = 2.5f;
        P.AestheticPreset.AsymmetryFactor = 0.12f;
        BuiltInPresets.Add(P);
    }

    // Preset 2: Angelica (serene, empathic)
    {
        FCharacterDesignPreset P;
        P.PresetName = TEXT("Angelica");
        P.Description = TEXT("The AI Angel — serene beauty with empathic expression and gentle charisma");
        P.AestheticPreset.ConfidencePosture = 0.8f;
        P.AestheticPreset.CharismaGlow = 0.9f;
        P.AestheticPreset.EyeSparkle = 0.9f;
        P.AestheticPreset.LipGlossSheen = 0.5f;
        P.AestheticPreset.SkinSSSBoost = 0.8f;
        P.AestheticPreset.ChaosIntensity = 0.3f;
        P.AestheticPreset.MicroExpressionAmplitude = 0.03f;
        P.AestheticPreset.MicroExpressionFrequency = 1.5f;
        P.AestheticPreset.AsymmetryFactor = 0.05f;
        BuiltInPresets.Add(P);
    }

    // Preset 3: Miara (explorer, balanced)
    {
        FCharacterDesignPreset P;
        P.PresetName = TEXT("Miara");
        P.Description = TEXT("The Explorer — balanced OCEAN personality with curious expressions");
        P.AestheticPreset.ConfidencePosture = 0.75f;
        P.AestheticPreset.CharismaGlow = 0.6f;
        P.AestheticPreset.EyeSparkle = 0.7f;
        P.AestheticPreset.LipGlossSheen = 0.4f;
        P.AestheticPreset.SkinSSSBoost = 0.5f;
        P.AestheticPreset.ChaosIntensity = 0.5f;
        P.AestheticPreset.MicroExpressionAmplitude = 0.04f;
        P.AestheticPreset.MicroExpressionFrequency = 2.0f;
        P.AestheticPreset.AsymmetryFactor = 0.08f;
        BuiltInPresets.Add(P);
    }

    // Preset 4: Marduk (mad scientist, high chaos)
    {
        FCharacterDesignPreset P;
        P.PresetName = TEXT("Marduk");
        P.Description = TEXT("The Mad Scientist — intense, unpredictable, with maximum chaotic micro-expressions");
        P.AestheticPreset.ConfidencePosture = 0.95f;
        P.AestheticPreset.CharismaGlow = 0.5f;
        P.AestheticPreset.EyeSparkle = 0.6f;
        P.AestheticPreset.LipGlossSheen = 0.3f;
        P.AestheticPreset.SkinSSSBoost = 0.4f;
        P.AestheticPreset.ChaosIntensity = 1.0f;
        P.AestheticPreset.MicroExpressionAmplitude = 0.1f;
        P.AestheticPreset.MicroExpressionFrequency = 3.5f;
        P.AestheticPreset.AsymmetryFactor = 0.2f;
        BuiltInPresets.Add(P);
    }

    // Preset 5: Neuro (witty VTuber, playful chaos)
    {
        FCharacterDesignPreset P;
        P.PresetName = TEXT("Neuro");
        P.Description = TEXT("The AI VTuber — witty, playful, with strategic chaos and self-aware humor");
        P.AestheticPreset.ConfidencePosture = 0.85f;
        P.AestheticPreset.CharismaGlow = 0.85f;
        P.AestheticPreset.EyeSparkle = 0.95f;
        P.AestheticPreset.LipGlossSheen = 0.7f;
        P.AestheticPreset.SkinSSSBoost = 0.6f;
        P.AestheticPreset.ChaosIntensity = 0.75f;
        P.AestheticPreset.MicroExpressionAmplitude = 0.07f;
        P.AestheticPreset.MicroExpressionFrequency = 3.0f;
        P.AestheticPreset.AsymmetryFactor = 0.15f;
        BuiltInPresets.Add(P);
    }
}
