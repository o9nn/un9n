#pragma once

#include "CoreMinimal.h"
#include "CubismSDKIntegration.generated.h"

/**
 * Live2D Cubism SDK Integration Layer
 * Provides proper SDK integration for Deep Tree Echo avatar system
 * Maintains super-hot-girl aesthetic and hyper-chaotic behavior properties
 */

// Forward declarations for Cubism SDK types
struct CubismModel;
struct CubismRenderer;
struct CubismMotionManager;
struct CubismPhysics;
struct CubismExpressionMotion;

USTRUCT(BlueprintType)
struct FCubismModelInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Live2D")
    FString ModelName;

    UPROPERTY(BlueprintReadOnly, Category = "Live2D")
    FVector2D CanvasSize;

    UPROPERTY(BlueprintReadOnly, Category = "Live2D")
    float PixelsPerUnit;

    UPROPERTY(BlueprintReadOnly, Category = "Live2D")
    int32 ParameterCount;

    UPROPERTY(BlueprintReadOnly, Category = "Live2D")
    int32 PartCount;

    UPROPERTY(BlueprintReadOnly, Category = "Live2D")
    int32 DrawableCount;

    FCubismModelInfo()
        : CanvasSize(FVector2D::ZeroVector)
        , PixelsPerUnit(1.0f)
        , ParameterCount(0)
        , PartCount(0)
        , DrawableCount(0)
    {}
};

USTRUCT(BlueprintType)
struct FCubismParameter
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Live2D")
    FName ParameterId;

    UPROPERTY(BlueprintReadWrite, Category = "Live2D")
    float Value;

    UPROPERTY(BlueprintReadWrite, Category = "Live2D")
    float MinValue;

    UPROPERTY(BlueprintReadWrite, Category = "Live2D")
    float MaxValue;

    UPROPERTY(BlueprintReadWrite, Category = "Live2D")
    float DefaultValue;

    FCubismParameter()
        : Value(0.0f)
        , MinValue(-1.0f)
        , MaxValue(1.0f)
        , DefaultValue(0.0f)
    {}
};

USTRUCT(BlueprintType)
struct FCubismDrawable
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Live2D")
    FName DrawableId;

    UPROPERTY(BlueprintReadOnly, Category = "Live2D")
    int32 TextureIndex;

    UPROPERTY(BlueprintReadOnly, Category = "Live2D")
    int32 VertexCount;

    UPROPERTY(BlueprintReadOnly, Category = "Live2D")
    int32 IndexCount;

    UPROPERTY(BlueprintReadOnly, Category = "Live2D")
    float Opacity;

    UPROPERTY(BlueprintReadOnly, Category = "Live2D")
    bool bIsVisible;

    FCubismDrawable()
        : TextureIndex(0)
        , VertexCount(0)
        , IndexCount(0)
        , Opacity(1.0f)
        , bIsVisible(true)
    {}
};

/**
 * Cubism SDK Integration Class
 * Handles actual SDK initialization and model management
 */
UCLASS()
class DEEPTREECHO_API UCubismSDKIntegration : public UObject
{
    GENERATED_BODY()

public:
    UCubismSDKIntegration();
    virtual ~UCubismSDKIntegration();

    /** Initialize Cubism Framework - must be called once at startup */
    UFUNCTION(BlueprintCallable, Category = "Live2D|SDK")
    static bool InitializeCubismFramework();

    /** Shutdown Cubism Framework - must be called once at shutdown */
    UFUNCTION(BlueprintCallable, Category = "Live2D|SDK")
    static void ShutdownCubismFramework();

    /** Check if Cubism Framework is initialized */
    UFUNCTION(BlueprintPure, Category = "Live2D|SDK")
    static bool IsCubismFrameworkInitialized();

    /** Load a Cubism model from MOC3 data */
    UFUNCTION(BlueprintCallable, Category = "Live2D|SDK")
    bool LoadModelFromMoc3(const TArray<uint8>& Moc3Data);

    /** Get model information */
    UFUNCTION(BlueprintPure, Category = "Live2D|SDK")
    FCubismModelInfo GetModelInfo() const;

    /** Get all parameters */
    UFUNCTION(BlueprintPure, Category = "Live2D|SDK")
    TArray<FCubismParameter> GetAllParameters() const;

    /** Get parameter by ID */
    UFUNCTION(BlueprintPure, Category = "Live2D|SDK")
    FCubismParameter GetParameter(const FName& ParameterId) const;

    /** Set parameter value */
    UFUNCTION(BlueprintCallable, Category = "Live2D|SDK")
    void SetParameterValue(const FName& ParameterId, float Value);

    /** Get all drawables */
    UFUNCTION(BlueprintPure, Category = "Live2D|SDK")
    TArray<FCubismDrawable> GetAllDrawables() const;

    /** Update model - must be called before rendering */
    UFUNCTION(BlueprintCallable, Category = "Live2D|SDK")
    void UpdateModel(float DeltaTime);

    /** Initialize renderer for this model */
    UFUNCTION(BlueprintCallable, Category = "Live2D|SDK")
    bool InitializeRenderer();

    /** Load and apply motion */
    UFUNCTION(BlueprintCallable, Category = "Live2D|SDK")
    bool LoadMotion(const TArray<uint8>& MotionData, int32 Priority = 0);

    /** Load and apply expression */
    UFUNCTION(BlueprintCallable, Category = "Live2D|SDK")
    bool LoadExpression(const TArray<uint8>& ExpressionData);

    /** Initialize physics simulation */
    UFUNCTION(BlueprintCallable, Category = "Live2D|SDK")
    bool InitializePhysics(const TArray<uint8>& PhysicsData);

    /** Update physics simulation */
    UFUNCTION(BlueprintCallable, Category = "Live2D|SDK")
    void UpdatePhysics(float DeltaTime);

    /** Get vertex positions for a drawable */
    UFUNCTION(BlueprintPure, Category = "Live2D|SDK")
    TArray<FVector2D> GetDrawableVertexPositions(const FName& DrawableId) const;

    /** Get UV coordinates for a drawable */
    UFUNCTION(BlueprintPure, Category = "Live2D|SDK")
    TArray<FVector2D> GetDrawableUVs(const FName& DrawableId) const;

    /** Get indices for a drawable */
    UFUNCTION(BlueprintPure, Category = "Live2D|SDK")
    TArray<int32> GetDrawableIndices(const FName& DrawableId) const;

private:
    /** Internal Cubism model pointer */
    void* CubismModelPtr;

    /** Internal Cubism renderer pointer */
    void* CubismRendererPtr;

    /** Internal motion manager pointer */
    void* MotionManagerPtr;

    /** Internal physics pointer */
    void* PhysicsPtr;

    /** Model information cache */
    FCubismModelInfo CachedModelInfo;

    /** Parameter cache */
    TMap<FName, FCubismParameter> ParameterCache;

    /** Drawable cache */
    TMap<FName, FCubismDrawable> DrawableCache;

    /** Static flag for framework initialization */
    static bool bIsFrameworkInitialized;

    /** Helper: Update parameter cache from model */
    void UpdateParameterCache();

    /** Helper: Update drawable cache from model */
    void UpdateDrawableCache();

    /** Helper: Find parameter index by ID */
    int32 FindParameterIndex(const FName& ParameterId) const;

    /** Helper: Find drawable index by ID */
    int32 FindDrawableIndex(const FName& DrawableId) const;
};

/**
 * Enhanced Live2D Avatar Features
 * Implements super-hot-girl aesthetic and hyper-chaotic behaviors
 */
UCLASS()
class DEEPTREECHO_API UCubismEnhancedFeatures : public UObject
{
    GENERATED_BODY()

public:
    /** Apply super-hot-girl aesthetic enhancements */
    UFUNCTION(BlueprintCallable, Category = "Live2D|Enhancement")
    static void ApplySuperHotGirlAesthetic(UCubismSDKIntegration* Model);

    /** Apply hyper-chaotic behavior patterns */
    UFUNCTION(BlueprintCallable, Category = "Live2D|Enhancement")
    static void ApplyHyperChaoticBehavior(UCubismSDKIntegration* Model, float ChaosFactor);

    /** Generate sparkle effect parameters for eyes */
    UFUNCTION(BlueprintCallable, Category = "Live2D|Enhancement")
    static void GenerateEyeSparkle(UCubismSDKIntegration* Model, float Intensity);

    /** Apply emotional blush effect */
    UFUNCTION(BlueprintCallable, Category = "Live2D|Enhancement")
    static void ApplyEmotionalBlush(UCubismSDKIntegration* Model, float Intensity, const FLinearColor& Color);

    /** Generate chaotic micro-expressions */
    UFUNCTION(BlueprintCallable, Category = "Live2D|Enhancement")
    static void GenerateChaoticMicroExpression(UCubismSDKIntegration* Model);

    /** Apply echo resonance visual effect */
    UFUNCTION(BlueprintCallable, Category = "Live2D|Enhancement")
    static void ApplyEchoResonanceEffect(UCubismSDKIntegration* Model, float Resonance);

    /** Generate glitch effect during cognitive overload */
    UFUNCTION(BlueprintCallable, Category = "Live2D|Enhancement")
    static void GenerateGlitchEffect(UCubismSDKIntegration* Model, float Severity);

    /** Apply hair shimmer and flow animation */
    UFUNCTION(BlueprintCallable, Category = "Live2D|Enhancement")
    static void ApplyHairShimmer(UCubismSDKIntegration* Model, float Time, float Intensity);

    /** Generate lip-sync with emotional modulation */
    UFUNCTION(BlueprintCallable, Category = "Live2D|Enhancement")
    static void GenerateEmotionalLipSync(UCubismSDKIntegration* Model, const TArray<float>& AudioData, float EmotionalIntensity);

    /** Apply gaze following with personality traits */
    UFUNCTION(BlueprintCallable, Category = "Live2D|Enhancement")
    static void ApplyPersonalizedGaze(UCubismSDKIntegration* Model, const FVector2D& Target, float Confidence, float Flirtiness);
};
