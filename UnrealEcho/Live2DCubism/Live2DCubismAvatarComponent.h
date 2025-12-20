#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Live2DCubismAvatarComponent.generated.h"

// Forward declarations
class UTexture2D;
class UMaterialInstanceDynamic;
struct FLive2DModelData;
struct FLive2DPhysicsData;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class DEEPTREECHO_API ULive2DCubismAvatarComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    ULive2DCubismAvatarComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    /** Load a Live2D model from a .model3.json file */
    UFUNCTION(BlueprintCallable, Category = "Live2D")
    void LoadLive2DModel(const FString& ModelPath);

    /** Update Live2D model parameters */
    UFUNCTION(BlueprintCallable, Category = "Live2D")
    void SetParameterValue(const FName& ParameterName, float Value);

    /** Get Live2D model parameter value */
    UFUNCTION(BlueprintCallable, Category = "Live2D")
    float GetParameterValue(const FName& ParameterName) const;

    /** Set facial expression by name */
    UFUNCTION(BlueprintCallable, Category = "Live2D|Expression")
    void SetExpression(const FString& ExpressionName);

    /** Set gaze target position */
    UFUNCTION(BlueprintCallable, Category = "Live2D|Expression")
    void SetGazeTarget(const FVector2D& TargetPosition);

    /** Set body rotation */
    UFUNCTION(BlueprintCallable, Category = "Live2D|Body")
    void SetBodyRotation(const FRotator& Rotation);

private:
    /** Initialize Live2D Cubism SDK */
    void InitializeLive2DCubismSDK();

    /** Initialize default parameters */
    void InitializeDefaultParameters();

    /** Load model3.json configuration file */
    void LoadModel3Json(const FString& JsonPath);

    /** Load .moc3 binary file */
    void LoadMoc3Binary(const FString& MocPath);

    /** Load texture file */
    void LoadTexture(const FString& TexturePath);

    /** Load physics configuration */
    void LoadPhysicsJson(const FString& PhysicsPath);

    /** Create render target for Live2D rendering */
    void CreateRenderTarget();

    /** Create dynamic material instance */
    void CreateDynamicMaterial();

    /** Update physics simulation */
    void UpdatePhysics(float DeltaTime);

    /** Update breathing animation */
    void UpdateBreathing(float DeltaTime);

    /** Update eye blink */
    void UpdateEyeBlink(float DeltaTime);

    /** Update model and render to texture */
    void UpdateModelAndRender(float DeltaTime);

    /** The Live2D model asset */
    UPROPERTY(Transient)
    UObject* Live2DModel;

    /** The texture for rendering the Live2D model */
    UPROPERTY(Transient)
    UTexture2D* RenderTarget;

    /** The dynamic material instance for the Live2D model */
    UPROPERTY(Transient)
    UMaterialInstanceDynamic* DynamicMaterial;

    /** Model data structure */
    TSharedPtr<FLive2DModelData> ModelData;

    /** Physics data structure */
    TSharedPtr<FLive2DPhysicsData> PhysicsData;
};
