// CognitiveAnimationEditor.cpp
// Deep Tree Echo - Cognitive Animation Editor
// Copyright (c) 2025-2026 Deep Tree Echo Project

#include "CognitiveAnimationEditor.h"

// ============================================================================
// FAnimationTrack Implementation
// ============================================================================

float FAnimationTrack::Evaluate(float Time) const
{
    if (KeyTimes.Num() == 0) return 0.0f;
    if (KeyTimes.Num() == 1) return KeyValues[0];

    // Find surrounding keys
    int32 Before = 0;
    int32 After = KeyTimes.Num() - 1;

    for (int32 i = 0; i < KeyTimes.Num(); ++i)
    {
        if (KeyTimes[i] <= Time) Before = i;
        if (KeyTimes[i] >= Time) { After = i; break; }
    }

    if (Before == After) return KeyValues[Before];

    float Alpha = (Time - KeyTimes[Before]) / FMath::Max(KeyTimes[After] - KeyTimes[Before], KINDA_SMALL_NUMBER);
    Alpha = FMath::Clamp(Alpha, 0.0f, 1.0f);

    // Apply blend mode
    switch (BlendMode)
    {
        case EAnimBlendMode::Linear:
            break; // Alpha unchanged
        case EAnimBlendMode::SmoothStep:
            Alpha = Alpha * Alpha * (3.0f - 2.0f * Alpha);
            break;
        case EAnimBlendMode::EaseIn:
            Alpha = Alpha * Alpha;
            break;
        case EAnimBlendMode::EaseOut:
            Alpha = 1.0f - (1.0f - Alpha) * (1.0f - Alpha);
            break;
        case EAnimBlendMode::EaseInOut:
            Alpha = Alpha < 0.5f ? 2.0f * Alpha * Alpha : 1.0f - FMath::Pow(-2.0f * Alpha + 2.0f, 2.0f) / 2.0f;
            break;
        case EAnimBlendMode::Spring:
        {
            float c4 = (2.0f * PI) / 3.0f;
            Alpha = Alpha == 0.0f ? 0.0f : Alpha == 1.0f ? 1.0f :
                    FMath::Pow(2.0f, -10.0f * Alpha) * FMath::Sin((Alpha * 10.0f - 0.75f) * c4) + 1.0f;
            break;
        }
    }

    return FMath::Lerp(KeyValues[Before], KeyValues[After], Alpha);
}

void FAnimationTrack::AddKey(float Time, float Value)
{
    // Insert in sorted order
    int32 InsertIdx = 0;
    for (int32 i = 0; i < KeyTimes.Num(); ++i)
    {
        if (KeyTimes[i] > Time) { InsertIdx = i; break; }
        InsertIdx = i + 1;
    }
    KeyTimes.Insert(Time, InsertIdx);
    KeyValues.Insert(Value, InsertIdx);
}

void FAnimationTrack::RemoveKey(int32 Index)
{
    if (Index >= 0 && Index < KeyTimes.Num())
    {
        KeyTimes.RemoveAt(Index);
        KeyValues.RemoveAt(Index);
    }
}

// ============================================================================
// FCognitiveAnimationClip Implementation
// ============================================================================

void FCognitiveAnimationClip::EvaluateAtTime(float Time, FFACSActionUnitState& OutFACS,
    FHormoneBusState& OutHormones, FSuperHotGirlAesthetics& OutAesthetics) const
{
    if (bLooping && Duration > 0.0f)
    {
        Time = FMath::Fmod(Time, Duration);
    }

    // Evaluate FACS AU tracks
    for (const auto& Pair : FACSAUTracks)
    {
        OutFACS.ActionUnits.Add(Pair.Key, Pair.Value.Evaluate(Time));
    }

    // Evaluate hormone tracks
    for (const auto& Pair : HormoneTracks)
    {
        OutHormones.SetChannel(Pair.Key, Pair.Value.Evaluate(Time));
    }

    // Evaluate aesthetic tracks
    for (const auto& Pair : AestheticTracks)
    {
        float Value = Pair.Value.Evaluate(Time);
        if (Pair.Key == TEXT("ConfidencePosture")) OutAesthetics.ConfidencePosture = Value;
        else if (Pair.Key == TEXT("Charisma")) OutAesthetics.Charisma = Value;
        else if (Pair.Key == TEXT("EyeSparkle")) OutAesthetics.EyeSparkle = Value;
        else if (Pair.Key == TEXT("GracefulMovement")) OutAesthetics.GracefulMovement = Value;
        else if (Pair.Key == TEXT("EmissiveGlow")) OutAesthetics.EmissiveGlow = Value;
        else if (Pair.Key == TEXT("BioluminescentIntensity")) OutAesthetics.BioluminescentIntensity = Value;
    }
}

void FCognitiveAnimationClip::RecalculateDuration()
{
    Duration = 0.0f;
    for (const auto& Pair : FACSAUTracks)
    {
        if (Pair.Value.KeyTimes.Num() > 0)
            Duration = FMath::Max(Duration, Pair.Value.KeyTimes.Last());
    }
    for (const auto& Pair : HormoneTracks)
    {
        if (Pair.Value.KeyTimes.Num() > 0)
            Duration = FMath::Max(Duration, Pair.Value.KeyTimes.Last());
    }
    for (const auto& Pair : AestheticTracks)
    {
        if (Pair.Value.KeyTimes.Num() > 0)
            Duration = FMath::Max(Duration, Pair.Value.KeyTimes.Last());
    }
}

// ============================================================================
// UCognitiveAnimationEditor Implementation
// ============================================================================

UCognitiveAnimationEditor::UCognitiveAnimationEditor()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.016f; // 60 Hz for smooth playback
}

void UCognitiveAnimationEditor::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIsPlaying && !bIsPaused && ActiveClipIndex >= 0 && ActiveClipIndex < Clips.Num())
    {
        PlaybackTime += DeltaTime;
        const FCognitiveAnimationClip& Clip = Clips[ActiveClipIndex];

        if (!Clip.bLooping && PlaybackTime > Clip.Duration)
        {
            StopPlayback();
            return;
        }

        if (ConnectedStudio)
        {
            ApplyClipStateToStudio(Clip, PlaybackTime);
        }
    }
}

// ========================================================================
// Clip Management
// ========================================================================

int32 UCognitiveAnimationEditor::CreateClip(const FString& Name)
{
    FCognitiveAnimationClip NewClip;
    NewClip.ClipName = Name;
    Clips.Add(NewClip);
    return Clips.Num() - 1;
}

void UCognitiveAnimationEditor::DeleteClip(int32 ClipIndex)
{
    if (ClipIndex >= 0 && ClipIndex < Clips.Num())
    {
        if (ActiveClipIndex == ClipIndex) StopPlayback();
        Clips.RemoveAt(ClipIndex);
        if (ActiveClipIndex > ClipIndex) ActiveClipIndex--;
    }
}

int32 UCognitiveAnimationEditor::DuplicateClip(int32 SourceIndex, const FString& NewName)
{
    if (SourceIndex < 0 || SourceIndex >= Clips.Num()) return -1;

    FCognitiveAnimationClip Copy = Clips[SourceIndex];
    Copy.ClipName = NewName;
    Clips.Add(Copy);
    return Clips.Num() - 1;
}

// ========================================================================
// Track Editing
// ========================================================================

void UCognitiveAnimationEditor::AddFACSKey(int32 ClipIndex, int32 AUNumber, float Time, float Value)
{
    if (ClipIndex < 0 || ClipIndex >= Clips.Num()) return;

    FAnimationTrack& Track = Clips[ClipIndex].FACSAUTracks.FindOrAdd(AUNumber);
    if (Track.TrackName.IsEmpty())
    {
        Track.TrackName = FString::Printf(TEXT("AU%d"), AUNumber);
    }
    Track.AddKey(Time, FMath::Clamp(Value, 0.0f, 1.0f));
    Clips[ClipIndex].RecalculateDuration();
}

void UCognitiveAnimationEditor::AddHormoneKey(int32 ClipIndex, int32 ChannelIndex, float Time, float Value)
{
    if (ClipIndex < 0 || ClipIndex >= Clips.Num()) return;

    static const TCHAR* HormoneNames[] = {
        TEXT("CRH"), TEXT("ACTH"), TEXT("Cortisol"), TEXT("DopamineTonic"),
        TEXT("DopaminePhasic"), TEXT("Serotonin"), TEXT("Norepinephrine"), TEXT("Oxytocin"),
        TEXT("T3T4"), TEXT("Melatonin"), TEXT("Insulin"), TEXT("Glucagon"),
        TEXT("IL6"), TEXT("Anandamide"), TEXT("Reserved0"), TEXT("Reserved1")
    };

    FAnimationTrack& Track = Clips[ClipIndex].HormoneTracks.FindOrAdd(ChannelIndex);
    if (Track.TrackName.IsEmpty() && ChannelIndex >= 0 && ChannelIndex < 16)
    {
        Track.TrackName = HormoneNames[ChannelIndex];
    }
    Track.AddKey(Time, FMath::Clamp(Value, 0.0f, 1.0f));
    Clips[ClipIndex].RecalculateDuration();
}

void UCognitiveAnimationEditor::AddAestheticKey(int32 ClipIndex, const FString& ParamName, float Time, float Value)
{
    if (ClipIndex < 0 || ClipIndex >= Clips.Num()) return;

    FAnimationTrack& Track = Clips[ClipIndex].AestheticTracks.FindOrAdd(ParamName);
    Track.TrackName = ParamName;
    Track.AddKey(Time, FMath::Clamp(Value, 0.0f, 1.0f));
    Clips[ClipIndex].RecalculateDuration();
}

void UCognitiveAnimationEditor::SetTrackBlendMode(int32 ClipIndex, const FString& TrackName, EAnimBlendMode Mode)
{
    if (ClipIndex < 0 || ClipIndex >= Clips.Num()) return;

    FCognitiveAnimationClip& Clip = Clips[ClipIndex];
    for (auto& Pair : Clip.FACSAUTracks)
    {
        if (Pair.Value.TrackName == TrackName) { Pair.Value.BlendMode = Mode; return; }
    }
    for (auto& Pair : Clip.HormoneTracks)
    {
        if (Pair.Value.TrackName == TrackName) { Pair.Value.BlendMode = Mode; return; }
    }
    for (auto& Pair : Clip.AestheticTracks)
    {
        if (Pair.Value.TrackName == TrackName) { Pair.Value.BlendMode = Mode; return; }
    }
}

// ========================================================================
// Playback
// ========================================================================

void UCognitiveAnimationEditor::PlayClip(int32 ClipIndex, UAvatarDesignStudio* DesignStudio)
{
    if (ClipIndex < 0 || ClipIndex >= Clips.Num()) return;

    ActiveClipIndex = ClipIndex;
    ConnectedStudio = DesignStudio;
    PlaybackTime = 0.0f;
    bIsPlaying = true;
    bIsPaused = false;
}

void UCognitiveAnimationEditor::PausePlayback()
{
    bIsPaused = true;
}

void UCognitiveAnimationEditor::ResumePlayback()
{
    bIsPaused = false;
}

void UCognitiveAnimationEditor::StopPlayback()
{
    bIsPlaying = false;
    bIsPaused = false;
    ActiveClipIndex = -1;
    PlaybackTime = 0.0f;
}

void UCognitiveAnimationEditor::ScrubToTime(float Time)
{
    PlaybackTime = FMath::Max(0.0f, Time);
    if (ActiveClipIndex >= 0 && ActiveClipIndex < Clips.Num() && ConnectedStudio)
    {
        ApplyClipStateToStudio(Clips[ActiveClipIndex], PlaybackTime);
    }
}

void UCognitiveAnimationEditor::ApplyClipStateToStudio(const FCognitiveAnimationClip& Clip, float Time)
{
    if (!ConnectedStudio) return;

    FFACSActionUnitState FACS;
    FHormoneBusState Hormones;
    FSuperHotGirlAesthetics Aesthetics;

    Clip.EvaluateAtTime(Time, FACS, Hormones, Aesthetics);

    // Apply FACS to studio
    for (const auto& Pair : FACS.ActionUnits)
    {
        ConnectedStudio->SetActionUnit(Pair.Key, Pair.Value);
    }

    // Apply hormones to studio
    for (int32 i = 0; i < 16; ++i)
    {
        ConnectedStudio->SetHormoneConcentration(i, Hormones.GetChannel(i));
    }
}

// ========================================================================
// Preset Generators
// ========================================================================

int32 UCognitiveAnimationEditor::GenerateExpressionTransition(const FString& FromExpression,
    const FString& ToExpression, float TransitionDuration, EAnimBlendMode BlendMode)
{
    FString Name = FString::Printf(TEXT("%s_to_%s"), *FromExpression, *ToExpression);
    int32 ClipIdx = CreateClip(Name);

    // Get the named expressions from a temporary design studio
    // For now, use standard DTE expression AU configurations
    TMap<int32, float> FromAUs;
    TMap<int32, float> ToAUs;

    // Standard DTE expressions (from live2d-dtecho skill)
    if (FromExpression == TEXT("JOY_01_BroadSmile")) { FromAUs.Add(6, 0.8f); FromAUs.Add(12, 0.8f); FromAUs.Add(25, 0.6f); }
    else if (FromExpression == TEXT("WONDER_02_CuriousGaze")) { FromAUs.Add(1, 0.3f); FromAUs.Add(2, 0.3f); FromAUs.Add(5, 0.6f); }
    else if (FromExpression == TEXT("PHOTO_Awe")) { FromAUs.Add(1, 0.6f); FromAUs.Add(2, 0.6f); FromAUs.Add(5, 0.8f); FromAUs.Add(26, 0.6f); }

    if (ToExpression == TEXT("JOY_01_BroadSmile")) { ToAUs.Add(6, 0.8f); ToAUs.Add(12, 0.8f); ToAUs.Add(25, 0.6f); }
    else if (ToExpression == TEXT("WONDER_02_CuriousGaze")) { ToAUs.Add(1, 0.3f); ToAUs.Add(2, 0.3f); ToAUs.Add(5, 0.6f); }
    else if (ToExpression == TEXT("PHOTO_Awe")) { ToAUs.Add(1, 0.6f); ToAUs.Add(2, 0.6f); ToAUs.Add(5, 0.8f); ToAUs.Add(26, 0.6f); }

    // Collect all AU numbers
    TSet<int32> AllAUs;
    for (const auto& P : FromAUs) AllAUs.Add(P.Key);
    for (const auto& P : ToAUs) AllAUs.Add(P.Key);

    // Create tracks for each AU
    for (int32 AU : AllAUs)
    {
        float FromVal = FromAUs.Contains(AU) ? FromAUs[AU] : 0.0f;
        float ToVal = ToAUs.Contains(AU) ? ToAUs[AU] : 0.0f;

        AddFACSKey(ClipIdx, AU, 0.0f, FromVal);
        AddFACSKey(ClipIdx, AU, TransitionDuration, ToVal);

        // Set blend mode
        FString TrackName = FString::Printf(TEXT("AU%d"), AU);
        SetTrackBlendMode(ClipIdx, TrackName, BlendMode);
    }

    return ClipIdx;
}

int32 UCognitiveAnimationEditor::GenerateEndocrineResponse(const FString& EventType, float Intensity, float ResponseDuration)
{
    FString Name = FString::Printf(TEXT("Endocrine_%s"), *EventType);
    int32 ClipIdx = CreateClip(Name);

    float PeakTime = ResponseDuration * 0.3f; // Peak at 30% of duration

    if (EventType == TEXT("REWARD_RECEIVED"))
    {
        // Dopamine phasic spike
        AddHormoneKey(ClipIdx, 4, 0.0f, 0.0f);
        AddHormoneKey(ClipIdx, 4, PeakTime, Intensity * 0.7f);
        AddHormoneKey(ClipIdx, 4, ResponseDuration, 0.0f);

        // Serotonin gentle rise
        AddHormoneKey(ClipIdx, 5, 0.0f, 0.4f);
        AddHormoneKey(ClipIdx, 5, PeakTime * 1.5f, 0.4f + Intensity * 0.15f);
        AddHormoneKey(ClipIdx, 5, ResponseDuration, 0.4f);

        // Expression: smile builds and fades
        AddFACSKey(ClipIdx, 12, 0.0f, 0.0f);
        AddFACSKey(ClipIdx, 12, PeakTime, Intensity * 0.9f);
        AddFACSKey(ClipIdx, 12, ResponseDuration, 0.1f);
        AddFACSKey(ClipIdx, 6, 0.0f, 0.0f);
        AddFACSKey(ClipIdx, 6, PeakTime, Intensity * 0.7f);
        AddFACSKey(ClipIdx, 6, ResponseDuration, 0.05f);
    }
    else if (EventType == TEXT("THREAT_DETECTED"))
    {
        // Cortisol cascade
        AddHormoneKey(ClipIdx, 0, 0.0f, 0.05f); // CRH
        AddHormoneKey(ClipIdx, 0, PeakTime * 0.5f, Intensity * 0.4f);
        AddHormoneKey(ClipIdx, 0, ResponseDuration, 0.05f);

        AddHormoneKey(ClipIdx, 2, 0.0f, 0.15f); // Cortisol
        AddHormoneKey(ClipIdx, 2, PeakTime, 0.15f + Intensity * 0.5f);
        AddHormoneKey(ClipIdx, 2, ResponseDuration, 0.15f);

        // NE spike
        AddHormoneKey(ClipIdx, 6, 0.0f, 0.1f);
        AddHormoneKey(ClipIdx, 6, PeakTime * 0.7f, Intensity * 0.6f);
        AddHormoneKey(ClipIdx, 6, ResponseDuration, 0.1f);

        // Expression: brow furrow + wide eyes
        AddFACSKey(ClipIdx, 4, 0.0f, 0.0f);
        AddFACSKey(ClipIdx, 4, PeakTime, Intensity * 0.7f);
        AddFACSKey(ClipIdx, 4, ResponseDuration, 0.0f);
        AddFACSKey(ClipIdx, 5, 0.0f, 0.0f);
        AddFACSKey(ClipIdx, 5, PeakTime, Intensity * 0.6f);
        AddFACSKey(ClipIdx, 5, ResponseDuration, 0.0f);
    }
    else if (EventType == TEXT("NOVELTY_ENCOUNTERED"))
    {
        // NE + phasic dopamine
        AddHormoneKey(ClipIdx, 6, 0.0f, 0.1f);
        AddHormoneKey(ClipIdx, 6, PeakTime, Intensity * 0.5f);
        AddHormoneKey(ClipIdx, 6, ResponseDuration, 0.1f);

        AddHormoneKey(ClipIdx, 4, 0.0f, 0.0f);
        AddHormoneKey(ClipIdx, 4, PeakTime * 0.8f, Intensity * 0.3f);
        AddHormoneKey(ClipIdx, 4, ResponseDuration, 0.0f);

        // Expression: raised brows + wide eyes (curiosity)
        AddFACSKey(ClipIdx, 1, 0.0f, 0.0f);
        AddFACSKey(ClipIdx, 1, PeakTime, Intensity * 0.5f);
        AddFACSKey(ClipIdx, 1, ResponseDuration, 0.0f);
        AddFACSKey(ClipIdx, 2, 0.0f, 0.0f);
        AddFACSKey(ClipIdx, 2, PeakTime, Intensity * 0.5f);
        AddFACSKey(ClipIdx, 2, ResponseDuration, 0.0f);
        AddFACSKey(ClipIdx, 5, 0.0f, 0.0f);
        AddFACSKey(ClipIdx, 5, PeakTime, Intensity * 0.6f);
        AddFACSKey(ClipIdx, 5, ResponseDuration, 0.0f);
    }

    return ClipIdx;
}

int32 UCognitiveAnimationEditor::GenerateCognitiveCycleClip(float CycleDuration)
{
    int32 ClipIdx = CreateClip(TEXT("CognitiveCycle_9Step"));
    float StepDuration = CycleDuration / 9.0f;

    // 9 steps of the Echobeats cognitive cycle, each with characteristic expression
    // Step 1: SENSE - neutral, alert
    AddFACSKey(ClipIdx, 5, 0.0f, 0.3f); // Slight eye opening
    AddHormoneKey(ClipIdx, 6, 0.0f, 0.2f); // NE baseline

    // Step 2: ATTEND - focused
    AddFACSKey(ClipIdx, 7, StepDuration, 0.4f); // Lid tightener
    AddHormoneKey(ClipIdx, 6, StepDuration, 0.35f); // NE rises

    // Step 3: REMEMBER - contemplative
    AddFACSKey(ClipIdx, 4, StepDuration * 2, 0.2f); // Slight brow furrow
    AddFACSKey(ClipIdx, 63, StepDuration * 2, 0.3f); // Eyes up (recall)

    // Step 4: PREDICT - anticipatory
    AddFACSKey(ClipIdx, 1, StepDuration * 3, 0.3f); // Inner brow raise
    AddHormoneKey(ClipIdx, 3, StepDuration * 3, 0.4f); // Tonic dopamine

    // Step 5: COMPARE - evaluative
    AddFACSKey(ClipIdx, 4, StepDuration * 4, 0.3f); // Brow lowerer
    AddFACSKey(ClipIdx, 7, StepDuration * 4, 0.3f); // Lid tightener

    // Step 6: LEARN - processing
    AddHormoneKey(ClipIdx, 8, StepDuration * 5, 0.7f); // T3/T4 up (processing)
    AddFACSKey(ClipIdx, 4, StepDuration * 5, 0.15f); // Slight concentration

    // Step 7: DECIDE - resolute
    AddFACSKey(ClipIdx, 12, StepDuration * 6, 0.3f); // Slight smile (confidence)
    AddHormoneKey(ClipIdx, 5, StepDuration * 6, 0.5f); // Serotonin (certainty)

    // Step 8: ACT - engaged
    AddFACSKey(ClipIdx, 12, StepDuration * 7, 0.5f); // Smile
    AddFACSKey(ClipIdx, 6, StepDuration * 7, 0.4f); // Cheek raise
    AddHormoneKey(ClipIdx, 4, StepDuration * 7, 0.3f); // Phasic dopamine

    // Step 9: REFLECT - serene
    AddFACSKey(ClipIdx, 12, StepDuration * 8, 0.2f); // Gentle smile
    AddFACSKey(ClipIdx, 6, StepDuration * 8, 0.2f);
    AddHormoneKey(ClipIdx, 5, StepDuration * 8, 0.55f); // High serotonin
    AddHormoneKey(ClipIdx, 13, StepDuration * 8, 0.2f); // Anandamide (calm)

    // Return to neutral
    AddFACSKey(ClipIdx, 5, CycleDuration, 0.1f);
    AddFACSKey(ClipIdx, 12, CycleDuration, 0.1f);

    return ClipIdx;
}

int32 UCognitiveAnimationEditor::GenerateChaoticIdleClip(float Duration, float ChaosIntensity)
{
    int32 ClipIdx = CreateClip(TEXT("ChaoticIdle"));
    Clips[ClipIdx].bLooping = true;

    // Generate Lorenz-derived keyframes for natural idle variation
    FLorenzAttractorState Lorenz;
    Lorenz.ChaosIntensity = ChaosIntensity;

    int32 NumKeys = FMath::Max(10, FMath::CeilToInt(Duration * 5.0f)); // 5 keys per second
    float TimeStep = Duration / static_cast<float>(NumKeys);

    for (int32 i = 0; i <= NumKeys; ++i)
    {
        float Time = i * TimeStep;
        Lorenz.Step(TimeStep);
        FVector Norm = Lorenz.GetNormalizedOutput();

        // Subtle eye movements
        AddFACSKey(ClipIdx, 5, Time, 0.1f + FMath::Abs(Norm.X) * ChaosIntensity * 0.2f);
        // Subtle brow movements
        AddFACSKey(ClipIdx, 1, Time, FMath::Max(0.0f, Norm.Y * ChaosIntensity * 0.15f));
        // Subtle smile variation
        AddFACSKey(ClipIdx, 12, Time, 0.1f + FMath::Max(0.0f, Norm.Z * ChaosIntensity * 0.2f));
        // Subtle blink tendency
        float BlinkChance = FMath::Abs(Norm.X * Norm.Y);
        if (BlinkChance > 0.7f)
        {
            AddFACSKey(ClipIdx, 43, Time, 0.8f);
            AddFACSKey(ClipIdx, 43, Time + 0.15f, 0.0f); // Quick blink
        }
    }

    return ClipIdx;
}

// ========================================================================
// Export / Import
// ========================================================================

FString UCognitiveAnimationEditor::ExportClipToJSON(int32 ClipIndex) const
{
    if (ClipIndex < 0 || ClipIndex >= Clips.Num()) return TEXT("{}");

    const FCognitiveAnimationClip& Clip = Clips[ClipIndex];
    FString JSON = TEXT("{");
    JSON += FString::Printf(TEXT("\"name\":\"%s\","), *Clip.ClipName);
    JSON += FString::Printf(TEXT("\"duration\":%.4f,"), Clip.Duration);
    JSON += FString::Printf(TEXT("\"looping\":%s,"), Clip.bLooping ? TEXT("true") : TEXT("false"));

    // FACS tracks
    JSON += TEXT("\"facs_tracks\":{");
    bool bFirst = true;
    for (const auto& Pair : Clip.FACSAUTracks)
    {
        if (!bFirst) JSON += TEXT(",");
        JSON += FString::Printf(TEXT("\"AU%d\":{\"times\":["), Pair.Key);
        for (int32 i = 0; i < Pair.Value.KeyTimes.Num(); ++i)
        {
            if (i > 0) JSON += TEXT(",");
            JSON += FString::Printf(TEXT("%.4f"), Pair.Value.KeyTimes[i]);
        }
        JSON += TEXT("],\"values\":[");
        for (int32 i = 0; i < Pair.Value.KeyValues.Num(); ++i)
        {
            if (i > 0) JSON += TEXT(",");
            JSON += FString::Printf(TEXT("%.4f"), Pair.Value.KeyValues[i]);
        }
        JSON += FString::Printf(TEXT("],\"blend\":%d}"), static_cast<int32>(Pair.Value.BlendMode));
        bFirst = false;
    }
    JSON += TEXT("},");

    // Hormone tracks
    JSON += TEXT("\"hormone_tracks\":{");
    bFirst = true;
    for (const auto& Pair : Clip.HormoneTracks)
    {
        if (!bFirst) JSON += TEXT(",");
        JSON += FString::Printf(TEXT("\"%d\":{\"name\":\"%s\",\"times\":["), Pair.Key, *Pair.Value.TrackName);
        for (int32 i = 0; i < Pair.Value.KeyTimes.Num(); ++i)
        {
            if (i > 0) JSON += TEXT(",");
            JSON += FString::Printf(TEXT("%.4f"), Pair.Value.KeyTimes[i]);
        }
        JSON += TEXT("],\"values\":[");
        for (int32 i = 0; i < Pair.Value.KeyValues.Num(); ++i)
        {
            if (i > 0) JSON += TEXT(",");
            JSON += FString::Printf(TEXT("%.4f"), Pair.Value.KeyValues[i]);
        }
        JSON += TEXT("]}");
        bFirst = false;
    }
    JSON += TEXT("}");

    JSON += TEXT("}");
    return JSON;
}

int32 UCognitiveAnimationEditor::ImportClipFromJSON(const FString& JSONString)
{
    // Basic import - create clip with name from JSON
    FCognitiveAnimationClip NewClip;
    int32 NameStart = JSONString.Find(TEXT("\"name\":\"")) + 8;
    int32 NameEnd = JSONString.Find(TEXT("\""), ESearchCase::IgnoreCase, ESearchDir::FromStart, NameStart);
    if (NameStart > 7 && NameEnd > NameStart)
    {
        NewClip.ClipName = JSONString.Mid(NameStart, NameEnd - NameStart);
    }
    else
    {
        NewClip.ClipName = TEXT("Imported Clip");
    }

    Clips.Add(NewClip);
    return Clips.Num() - 1;
}

bool UCognitiveAnimationEditor::ExportToAnimSequence(int32 ClipIndex, const FString& AssetPath)
{
    if (ClipIndex < 0 || ClipIndex >= Clips.Num()) return false;

    // In full UE5 context, this would create a UAnimSequence asset
    // For standalone compilation, we log the export path
    UE_LOG(LogTemp, Log, TEXT("CognitiveAnimationEditor: Exporting clip '%s' to %s"),
           *Clips[ClipIndex].ClipName, *AssetPath);
    return true;
}
