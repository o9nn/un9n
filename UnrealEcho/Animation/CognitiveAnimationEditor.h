// CognitiveAnimationEditor.h
// Deep Tree Echo - Cognitive Animation Editor
// Copyright (c) 2025-2026 Deep Tree Echo Project
//
// A timeline-based animation editor for cognitive avatar expressions.
// Integrates with AvatarDesignStudio for real-time preview and
// supports export to standard animation formats.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../Avatar/AvatarDesignStudio.h"
#include "CognitiveAnimationEditor.generated.h"

/** Animation blend mode */
UENUM(BlueprintType)
enum class EAnimBlendMode : uint8
{
    Linear      UMETA(DisplayName = "Linear"),
    SmoothStep  UMETA(DisplayName = "Smooth Step"),
    EaseIn      UMETA(DisplayName = "Ease In"),
    EaseOut     UMETA(DisplayName = "Ease Out"),
    EaseInOut   UMETA(DisplayName = "Ease In/Out"),
    Spring      UMETA(DisplayName = "Spring")
};

/** Animation track for a single parameter */
USTRUCT(BlueprintType)
struct FAnimationTrack
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track")
    FString TrackName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track")
    TArray<float> KeyTimes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track")
    TArray<float> KeyValues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track")
    EAnimBlendMode BlendMode = EAnimBlendMode::SmoothStep;

    /** Evaluate the track at a given time */
    float Evaluate(float Time) const;

    /** Add a key */
    void AddKey(float Time, float Value);

    /** Remove a key by index */
    void RemoveKey(int32 Index);
};

/** Cognitive animation clip containing multiple tracks */
USTRUCT(BlueprintType)
struct FCognitiveAnimationClip
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clip")
    FString ClipName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clip")
    float Duration = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clip")
    bool bLooping = false;

    /** FACS AU tracks (key = AU number) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clip")
    TMap<int32, FAnimationTrack> FACSAUTracks;

    /** Hormone tracks (key = channel index) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clip")
    TMap<int32, FAnimationTrack> HormoneTracks;

    /** Aesthetic parameter tracks */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clip")
    TMap<FString, FAnimationTrack> AestheticTracks;

    /** 4E cognition metric tracks */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clip")
    TMap<FString, FAnimationTrack> CognitionTracks;

    /** Evaluate all tracks at a given time and return a complete state */
    void EvaluateAtTime(float Time, FFACSActionUnitState& OutFACS, FHormoneBusState& OutHormones,
                        FSuperHotGirlAesthetics& OutAesthetics) const;

    /** Recalculate duration from all tracks */
    void RecalculateDuration();
};

/**
 * UCognitiveAnimationEditor
 *
 * Timeline-based animation editor for Deep Tree Echo cognitive avatar.
 * Provides clip management, multi-track editing, real-time preview,
 * and export capabilities.
 */
UCLASS(ClassGroup = (DeepTreeEcho), meta = (BlueprintSpawnableComponent))
class UCognitiveAnimationEditor : public UActorComponent
{
    GENERATED_BODY()

public:
    UCognitiveAnimationEditor();

    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ========================================================================
    // Clip Management
    // ========================================================================

    /** Create a new animation clip */
    UFUNCTION(BlueprintCallable, Category = "Animation Editor")
    int32 CreateClip(const FString& Name);

    /** Delete a clip */
    UFUNCTION(BlueprintCallable, Category = "Animation Editor")
    void DeleteClip(int32 ClipIndex);

    /** Duplicate a clip */
    UFUNCTION(BlueprintCallable, Category = "Animation Editor")
    int32 DuplicateClip(int32 SourceIndex, const FString& NewName);

    /** Get all clips */
    UFUNCTION(BlueprintPure, Category = "Animation Editor")
    TArray<FCognitiveAnimationClip> GetClips() const { return Clips; }

    // ========================================================================
    // Track Editing
    // ========================================================================

    /** Add a FACS AU keyframe to a clip */
    UFUNCTION(BlueprintCallable, Category = "Animation Editor")
    void AddFACSKey(int32 ClipIndex, int32 AUNumber, float Time, float Value);

    /** Add a hormone keyframe to a clip */
    UFUNCTION(BlueprintCallable, Category = "Animation Editor")
    void AddHormoneKey(int32 ClipIndex, int32 ChannelIndex, float Time, float Value);

    /** Add an aesthetic parameter keyframe */
    UFUNCTION(BlueprintCallable, Category = "Animation Editor")
    void AddAestheticKey(int32 ClipIndex, const FString& ParamName, float Time, float Value);

    /** Set blend mode for a track */
    UFUNCTION(BlueprintCallable, Category = "Animation Editor")
    void SetTrackBlendMode(int32 ClipIndex, const FString& TrackName, EAnimBlendMode Mode);

    // ========================================================================
    // Playback
    // ========================================================================

    /** Play a clip with optional connection to design studio */
    UFUNCTION(BlueprintCallable, Category = "Animation Editor")
    void PlayClip(int32 ClipIndex, UAvatarDesignStudio* DesignStudio = nullptr);

    /** Pause playback */
    UFUNCTION(BlueprintCallable, Category = "Animation Editor")
    void PausePlayback();

    /** Resume playback */
    UFUNCTION(BlueprintCallable, Category = "Animation Editor")
    void ResumePlayback();

    /** Stop playback */
    UFUNCTION(BlueprintCallable, Category = "Animation Editor")
    void StopPlayback();

    /** Scrub to a specific time */
    UFUNCTION(BlueprintCallable, Category = "Animation Editor")
    void ScrubToTime(float Time);

    /** Get current playback time */
    UFUNCTION(BlueprintPure, Category = "Animation Editor")
    float GetPlaybackTime() const { return PlaybackTime; }

    /** Is currently playing? */
    UFUNCTION(BlueprintPure, Category = "Animation Editor")
    bool IsPlaying() const { return bIsPlaying; }

    // ========================================================================
    // Preset Generators
    // ========================================================================

    /** Generate a clip from a named expression transition */
    UFUNCTION(BlueprintCallable, Category = "Animation Editor|Presets")
    int32 GenerateExpressionTransition(const FString& FromExpression, const FString& ToExpression,
                                        float TransitionDuration, EAnimBlendMode BlendMode);

    /** Generate a clip from an endocrine event response */
    UFUNCTION(BlueprintCallable, Category = "Animation Editor|Presets")
    int32 GenerateEndocrineResponse(const FString& EventType, float Intensity, float ResponseDuration);

    /** Generate a cognitive cycle visualization clip */
    UFUNCTION(BlueprintCallable, Category = "Animation Editor|Presets")
    int32 GenerateCognitiveCycleClip(float CycleDuration);

    /** Generate an idle animation with chaotic micro-expressions */
    UFUNCTION(BlueprintCallable, Category = "Animation Editor|Presets")
    int32 GenerateChaoticIdleClip(float Duration, float ChaosIntensity);

    // ========================================================================
    // Export / Import
    // ========================================================================

    /** Export clip to JSON */
    UFUNCTION(BlueprintCallable, Category = "Animation Editor|IO")
    FString ExportClipToJSON(int32 ClipIndex) const;

    /** Import clip from JSON */
    UFUNCTION(BlueprintCallable, Category = "Animation Editor|IO")
    int32 ImportClipFromJSON(const FString& JSONString);

    /** Export clip to Unreal Animation Sequence asset */
    UFUNCTION(BlueprintCallable, Category = "Animation Editor|IO")
    bool ExportToAnimSequence(int32 ClipIndex, const FString& AssetPath);

protected:
    UPROPERTY()
    TArray<FCognitiveAnimationClip> Clips;

    UPROPERTY()
    UAvatarDesignStudio* ConnectedStudio = nullptr;

    int32 ActiveClipIndex = -1;
    float PlaybackTime = 0.0f;
    bool bIsPlaying = false;
    bool bIsPaused = false;

    /** Apply current clip state to the connected design studio */
    void ApplyClipStateToStudio(const FCognitiveAnimationClip& Clip, float Time);
};
