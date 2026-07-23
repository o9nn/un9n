// DeepTreeEchoCognitiveTypes.h
// Deep Tree Echo - Canonical reflected cognitive state types
//
// These USTRUCTs are the UE-reflected counterparts of the neural cognitive
// state exchanged between the "neural" Deep Tree Echo core and the
// "symbolic" Unreal Engine systems (Cosmic Order, Avatar, Echobeats).
//
// They were previously defined only in the standalone compatibility shim
// (CoreMinimal.h), which made them invisible to UnrealHeaderTool. Any
// UFUNCTION that returns or accepts these types requires this header.
//
// Copyright (c) 2026 Deep Tree Echo Project

#pragma once

#ifndef DTE_SHIM_COREMINIMAL_ACTIVE
#include "CoreMinimal.h"
#endif
#define DTE_COGNITIVE_TYPES_REFLECTED 1
#include "DeepTreeEchoCognitiveTypes.generated.h"

/**
 * Core cognitive state of the Deep Tree Echo neural system.
 * Dimensional (PAD-style) affect plus attention/focus and cultivated wisdom.
 */
USTRUCT(BlueprintType)
struct FDeepTreeEchoCognitiveState
{
    GENERATED_BODY()

    /** Physiological/cognitive arousal (0 calm .. 1 highly activated) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cognitive State")
    float Arousal = 0.5f;

    /** Hedonic valence (-1 negative .. +1 positive; 0.5 default mid) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cognitive State")
    float Valence = 0.5f;

    /** Sense of agency/control (0 submissive .. 1 dominant) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cognitive State")
    float Dominance = 0.5f;

    /** Global attention level (0 .. 1) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cognitive State")
    float Attention = 0.5f;

    /** Focal concentration (0 diffuse .. 1 locked-on) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cognitive State")
    float Focus = 0.5f;

    /** Aggregated wisdom score from the wisdom-cultivation subsystem */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cognitive State")
    float WisdomScore = 0.0f;

    /** Named auxiliary state values (reservoir readouts, custom channels) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cognitive State")
    TMap<FString, float> StateValues;
};

/**
 * Discrete-emotion state (Plutchik primaries) plus dimensional summary.
 * Produced by the Cosmic Order autonomic triad and consumed by the avatar
 * expression pipeline.
 */
USTRUCT(BlueprintType)
struct FEmotionalState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion")
    float Joy = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion")
    float Sadness = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion")
    float Anger = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion")
    float Fear = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion")
    float Surprise = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion")
    float Disgust = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion")
    float Trust = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion")
    float Anticipation = 0.0f;

    /** Dimensional summary: arousal (0 .. 1) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion")
    float Arousal = 0.5f;

    /** Dimensional summary: valence (-1 .. +1) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion")
    float Valence = 0.5f;
};

/**
 * Executive/cognitive processing state derived from the cerebral and somatic
 * triads. Consumed by avatar behavior and gaming-mastery systems.
 */
USTRUCT(BlueprintType)
struct FCognitiveState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cognition")
    float Focus = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cognition")
    float Creativity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cognition")
    float ExecutiveFunction = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cognition")
    float MotorReadiness = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cognition")
    float SensoryIntegration = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cognition")
    float ProcessingLoad = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cognition")
    float Attention = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cognition")
    float Arousal = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cognition")
    float Valence = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cognition")
    float Dominance = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cognition")
    float WisdomScore = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cognition")
    float EmotionalResonance = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cognition")
    float ProprioceptiveAwareness = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cognition")
    TMap<FString, float> StateValues;
};
