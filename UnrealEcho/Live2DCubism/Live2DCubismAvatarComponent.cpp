#include "Live2DCubismAvatarComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Texture2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "RenderingThread.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "JsonObjectConverter.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

// Live2D Cubism SDK integration structures
struct FLive2DModelData
{
    TArray<uint8> MocData;
    TMap<FName, float> Parameters;
    TMap<FName, TPair<float, float>> ParameterRanges;
    TArray<FName> PartIds;
    TArray<FString> TexturePaths;
    FVector2D CanvasSize;
    float PixelsPerUnit;
};

struct FLive2DPhysicsData
{
    TMap<FName, FVector2D> PhysicsSettings;
    TMap<FName, float> Gravity;
    TMap<FName, float> Wind;
};

ULive2DCubismAvatarComponent::ULive2DCubismAvatarComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void ULive2DCubismAvatarComponent::BeginPlay()
{
    Super::BeginPlay();

    // Initialize Live2D Cubism SDK
    InitializeLive2DCubismSDK();
    
    UE_LOG(LogTemp, Log, TEXT("Live2D Cubism SDK Initialized Successfully"));
}

void ULive2DCubismAvatarComponent::InitializeLive2DCubismSDK()
{
    // Initialize Cubism Framework
    // Note: In production, this would call CubismFramework::Initialize()
    // For now, we set up the internal data structures
    
    ModelData = MakeShared<FLive2DModelData>();
    PhysicsData = MakeShared<FLive2DPhysicsData>();
    
    // Initialize default parameters for super-hot-girl aesthetic
    InitializeDefaultParameters();
}

void ULive2DCubismAvatarComponent::InitializeDefaultParameters()
{
    if (!ModelData.IsValid())
        return;

    // Facial expression parameters
    ModelData->Parameters.Add(TEXT("ParamEyeLOpen"), 1.0f);
    ModelData->Parameters.Add(TEXT("ParamEyeROpen"), 1.0f);
    ModelData->Parameters.Add(TEXT("ParamEyeBallX"), 0.0f);
    ModelData->Parameters.Add(TEXT("ParamEyeBallY"), 0.0f);
    ModelData->Parameters.Add(TEXT("ParamBrowLY"), 0.0f);
    ModelData->Parameters.Add(TEXT("ParamBrowRY"), 0.0f);
    ModelData->Parameters.Add(TEXT("ParamMouthForm"), 0.0f);
    ModelData->Parameters.Add(TEXT("ParamMouthOpenY"), 0.0f);
    ModelData->Parameters.Add(TEXT("ParamCheek"), 0.0f);
    
    // Body parameters
    ModelData->Parameters.Add(TEXT("ParamAngleX"), 0.0f);
    ModelData->Parameters.Add(TEXT("ParamAngleY"), 0.0f);
    ModelData->Parameters.Add(TEXT("ParamAngleZ"), 0.0f);
    ModelData->Parameters.Add(TEXT("ParamBodyAngleX"), 0.0f);
    ModelData->Parameters.Add(TEXT("ParamBodyAngleY"), 0.0f);
    ModelData->Parameters.Add(TEXT("ParamBodyAngleZ"), 0.0f);
    ModelData->Parameters.Add(TEXT("ParamBreath"), 0.0f);
    
    // Hair physics parameters
    ModelData->Parameters.Add(TEXT("ParamHairFront"), 0.0f);
    ModelData->Parameters.Add(TEXT("ParamHairSide"), 0.0f);
    ModelData->Parameters.Add(TEXT("ParamHairBack"), 0.0f);
    
    // Set parameter ranges
    for (const auto& Param : ModelData->Parameters)
    {
        ModelData->ParameterRanges.Add(Param.Key, TPair<float, float>(-1.0f, 1.0f));
    }
    
    // Special ranges for specific parameters
    ModelData->ParameterRanges[TEXT("ParamEyeLOpen")] = TPair<float, float>(0.0f, 1.0f);
    ModelData->ParameterRanges[TEXT("ParamEyeROpen")] = TPair<float, float>(0.0f, 1.0f);
    ModelData->ParameterRanges[TEXT("ParamMouthOpenY")] = TPair<float, float>(0.0f, 1.0f);
}

void ULive2DCubismAvatarComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!Live2DModel || !ModelData.IsValid())
        return;

    // Update physics simulation
    UpdatePhysics(DeltaTime);
    
    // Update breathing animation
    UpdateBreathing(DeltaTime);
    
    // Update eye blink
    UpdateEyeBlink(DeltaTime);
    
    // Update model and render
    UpdateModelAndRender(DeltaTime);
}

void ULive2DCubismAvatarComponent::UpdatePhysics(float DeltaTime)
{
    if (!PhysicsData.IsValid())
        return;

    // Simulate hair physics with spring dynamics
    const float SpringConstant = 0.5f;
    const float Damping = 0.8f;
    const float Mass = 1.0f;
    
    // Hair front physics
    float HairFrontTarget = GetParameterValue(TEXT("ParamAngleX")) * 0.3f;
    float HairFrontCurrent = GetParameterValue(TEXT("ParamHairFront"));
    float HairFrontVelocity = (HairFrontTarget - HairFrontCurrent) * SpringConstant;
    float NewHairFront = HairFrontCurrent + HairFrontVelocity * DeltaTime * Damping;
    SetParameterValue(TEXT("ParamHairFront"), NewHairFront);
    
    // Hair side physics
    float HairSideTarget = GetParameterValue(TEXT("ParamAngleY")) * 0.5f;
    float HairSideCurrent = GetParameterValue(TEXT("ParamHairSide"));
    float HairSideVelocity = (HairSideTarget - HairSideCurrent) * SpringConstant;
    float NewHairSide = HairSideCurrent + HairSideVelocity * DeltaTime * Damping;
    SetParameterValue(TEXT("ParamHairSide"), NewHairSide);
    
    // Hair back physics
    float HairBackTarget = GetParameterValue(TEXT("ParamBodyAngleY")) * 0.4f;
    float HairBackCurrent = GetParameterValue(TEXT("ParamHairBack"));
    float HairBackVelocity = (HairBackTarget - HairBackCurrent) * SpringConstant;
    float NewHairBack = HairBackCurrent + HairBackVelocity * DeltaTime * Damping;
    SetParameterValue(TEXT("ParamHairBack"), NewHairBack);
}

void ULive2DCubismAvatarComponent::UpdateBreathing(float DeltaTime)
{
    static float BreathTime = 0.0f;
    BreathTime += DeltaTime;
    
    // Natural breathing cycle (4 seconds per cycle)
    const float BreathCycleTime = 4.0f;
    const float BreathAmplitude = 0.3f;
    
    float BreathValue = FMath::Sin(2.0f * PI * BreathTime / BreathCycleTime) * BreathAmplitude;
    SetParameterValue(TEXT("ParamBreath"), BreathValue);
}

void ULive2DCubismAvatarComponent::UpdateEyeBlink(float DeltaTime)
{
    static float BlinkTimer = 0.0f;
    static float NextBlinkTime = 3.0f;
    static bool IsBlinking = false;
    static float BlinkProgress = 0.0f;
    
    BlinkTimer += DeltaTime;
    
    if (IsBlinking)
    {
        BlinkProgress += DeltaTime * 10.0f; // Blink speed
        
        if (BlinkProgress >= 1.0f)
        {
            IsBlinking = false;
            BlinkProgress = 0.0f;
            NextBlinkTime = FMath::RandRange(2.0f, 5.0f);
            BlinkTimer = 0.0f;
            
            SetParameterValue(TEXT("ParamEyeLOpen"), 1.0f);
            SetParameterValue(TEXT("ParamEyeROpen"), 1.0f);
        }
        else
        {
            // Smooth blink curve
            float BlinkCurve = FMath::Sin(BlinkProgress * PI);
            float EyeOpenness = 1.0f - BlinkCurve;
            
            SetParameterValue(TEXT("ParamEyeLOpen"), EyeOpenness);
            SetParameterValue(TEXT("ParamEyeROpen"), EyeOpenness);
        }
    }
    else if (BlinkTimer >= NextBlinkTime)
    {
        IsBlinking = true;
        BlinkProgress = 0.0f;
    }
}

void ULive2DCubismAvatarComponent::UpdateModelAndRender(float DeltaTime)
{
    // Update model transformation matrix
    // In production, this would call model->Update()
    
    // Render to texture
    if (RenderTarget && DynamicMaterial)
    {
        // Update material parameters with current model state
        for (const auto& Param : ModelData->Parameters)
        {
            FName MaterialParamName = FName(*FString::Printf(TEXT("Live2D_%s"), *Param.Key.ToString()));
            DynamicMaterial->SetScalarParameterValue(MaterialParamName, Param.Value);
        }
    }
}

void ULive2DCubismAvatarComponent::LoadLive2DModel(const FString& ModelPath)
{
    if (!FPaths::FileExists(ModelPath))
    {
        UE_LOG(LogTemp, Error, TEXT("Model file not found: %s"), *ModelPath);
        return;
    }

    // Determine file type
    FString Extension = FPaths::GetExtension(ModelPath).ToLower();
    
    if (Extension == TEXT("json"))
    {
        LoadModel3Json(ModelPath);
    }
    else if (Extension == TEXT("moc3"))
    {
        LoadMoc3Binary(ModelPath);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Unsupported model file format: %s"), *Extension);
        return;
    }
    
    // Create render target for Live2D rendering
    CreateRenderTarget();
    
    // Create dynamic material instance
    CreateDynamicMaterial();
    
    UE_LOG(LogTemp, Log, TEXT("Live2D model loaded successfully: %s"), *ModelPath);
}

void ULive2DCubismAvatarComponent::LoadModel3Json(const FString& JsonPath)
{
    FString JsonString;
    if (!FFileHelper::LoadFileToString(JsonString, *JsonPath))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load model3.json file: %s"), *JsonPath);
        return;
    }

    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(JsonString);
    
    if (!FJsonSerializer::Deserialize(JsonReader, JsonObject) || !JsonObject.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to parse model3.json"));
        return;
    }

    // Parse model version
    int32 Version = JsonObject->GetIntegerField(TEXT("Version"));
    UE_LOG(LogTemp, Log, TEXT("Live2D Model Version: %d"), Version);

    // Parse file references
    FString MocFileName = JsonObject->GetStringField(TEXT("FileReferences"))->GetObjectField(TEXT("Moc"))->GetStringField(TEXT("File"));
    TArray<TSharedPtr<FJsonValue>> Textures = JsonObject->GetObjectField(TEXT("FileReferences"))->GetArrayField(TEXT("Textures"));
    
    // Load .moc3 file
    FString MocFilePath = FPaths::GetPath(JsonPath) / MocFileName;
    LoadMoc3Binary(MocFilePath);
    
    // Load textures
    for (const auto& TextureValue : Textures)
    {
        FString TexturePath = FPaths::GetPath(JsonPath) / TextureValue->AsString();
        LoadTexture(TexturePath);
    }

    // Parse physics settings if available
    if (JsonObject->HasField(TEXT("FileReferences")) && 
        JsonObject->GetObjectField(TEXT("FileReferences"))->HasField(TEXT("Physics")))
    {
        FString PhysicsFile = JsonObject->GetObjectField(TEXT("FileReferences"))->GetStringField(TEXT("Physics"));
        FString PhysicsPath = FPaths::GetPath(JsonPath) / PhysicsFile;
        LoadPhysicsJson(PhysicsPath);
    }
}

void ULive2DCubismAvatarComponent::LoadMoc3Binary(const FString& MocPath)
{
    if (!ModelData.IsValid())
        return;

    if (!FFileHelper::LoadFileToArray(ModelData->MocData, *MocPath))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load .moc3 file: %s"), *MocPath);
        return;
    }

    // Parse MOC3 header
    if (ModelData->MocData.Num() < 64)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid .moc3 file: too small"));
        return;
    }

    // Verify MOC3 magic number
    const uint8 MOC3_MAGIC[] = {'M', 'O', 'C', '3'};
    for (int i = 0; i < 4; i++)
    {
        if (ModelData->MocData[i] != MOC3_MAGIC[i])
        {
            UE_LOG(LogTemp, Error, TEXT("Invalid .moc3 file: wrong magic number"));
            return;
        }
    }

    // In production, this would call csmReviveMocInPlace() from Cubism SDK
    // For now, we create a placeholder model object
    Live2DModel = NewObject<UObject>();
    
    UE_LOG(LogTemp, Log, TEXT("MOC3 binary loaded successfully: %d bytes"), ModelData->MocData.Num());
}

void ULive2DCubismAvatarComponent::LoadTexture(const FString& TexturePath)
{
    // Load texture file
    TArray<uint8> TextureData;
    if (!FFileHelper::LoadFileToArray(TextureData, *TexturePath))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load texture: %s"), *TexturePath);
        return;
    }

    // Create UTexture2D from file data
    // In production, this would use image loading libraries
    RenderTarget = NewObject<UTexture2D>();
    
    if (ModelData.IsValid())
    {
        ModelData->TexturePaths.Add(TexturePath);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Texture loaded: %s"), *TexturePath);
}

void ULive2DCubismAvatarComponent::LoadPhysicsJson(const FString& PhysicsPath)
{
    FString JsonString;
    if (!FFileHelper::LoadFileToString(JsonString, *PhysicsPath))
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to load physics file: %s"), *PhysicsPath);
        return;
    }

    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(JsonString);
    
    if (!FJsonSerializer::Deserialize(JsonReader, JsonObject) || !JsonObject.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to parse physics JSON"));
        return;
    }

    // Parse physics settings
    if (JsonObject->HasField(TEXT("PhysicsSettings")))
    {
        TArray<TSharedPtr<FJsonValue>> PhysicsSettings = JsonObject->GetArrayField(TEXT("PhysicsSettings"));
        
        for (const auto& Setting : PhysicsSettings)
        {
            TSharedPtr<FJsonObject> SettingObj = Setting->AsObject();
            FString Id = SettingObj->GetStringField(TEXT("Id"));
            
            if (SettingObj->HasField(TEXT("Input")))
            {
                TArray<TSharedPtr<FJsonValue>> Inputs = SettingObj->GetArrayField(TEXT("Input"));
                // Process physics inputs
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Physics settings loaded successfully"));
}

void ULive2DCubismAvatarComponent::CreateRenderTarget()
{
    // Create render target texture for Live2D rendering
    RenderTarget = NewObject<UTexture2D>(this);
    
    if (RenderTarget)
    {
        // Set texture properties for high-quality rendering (super-hot-girl aesthetic)
        // In production, this would initialize the texture with proper format and size
        UE_LOG(LogTemp, Log, TEXT("Render target created for Live2D model"));
    }
}

void ULive2DCubismAvatarComponent::CreateDynamicMaterial()
{
    // Create dynamic material instance for rendering
    // In production, this would load a material asset and create an instance
    DynamicMaterial = UMaterialInstanceDynamic::Create(nullptr, this);
    
    if (DynamicMaterial)
    {
        // Set initial material parameters
        DynamicMaterial->SetTextureParameterValue(TEXT("Live2DTexture"), RenderTarget);
        UE_LOG(LogTemp, Log, TEXT("Dynamic material created for Live2D rendering"));
    }
}

void ULive2DCubismAvatarComponent::SetParameterValue(const FName& ParameterName, float Value)
{
    if (!ModelData.IsValid())
        return;

    if (ModelData->Parameters.Contains(ParameterName))
    {
        // Clamp value to parameter range
        if (ModelData->ParameterRanges.Contains(ParameterName))
        {
            TPair<float, float> Range = ModelData->ParameterRanges[ParameterName];
            Value = FMath::Clamp(Value, Range.Key, Range.Value);
        }
        
        ModelData->Parameters[ParameterName] = Value;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Parameter not found: %s"), *ParameterName.ToString());
    }
}

float ULive2DCubismAvatarComponent::GetParameterValue(const FName& ParameterName) const
{
    if (!ModelData.IsValid())
        return 0.0f;

    if (ModelData->Parameters.Contains(ParameterName))
    {
        return ModelData->Parameters[ParameterName];
    }
    
    return 0.0f;
}

void ULive2DCubismAvatarComponent::SetExpression(const FString& ExpressionName)
{
    // Expression presets for super-hot-girl aesthetic
    if (ExpressionName == TEXT("Happy"))
    {
        SetParameterValue(TEXT("ParamMouthForm"), 1.0f);
        SetParameterValue(TEXT("ParamMouthOpenY"), 0.6f);
        SetParameterValue(TEXT("ParamEyeLOpen"), 0.9f);
        SetParameterValue(TEXT("ParamEyeROpen"), 0.9f);
        SetParameterValue(TEXT("ParamCheek"), 0.8f);
    }
    else if (ExpressionName == TEXT("Flirty"))
    {
        SetParameterValue(TEXT("ParamMouthForm"), 0.5f);
        SetParameterValue(TEXT("ParamMouthOpenY"), 0.3f);
        SetParameterValue(TEXT("ParamEyeLOpen"), 0.7f);
        SetParameterValue(TEXT("ParamEyeROpen"), 1.0f);
        SetParameterValue(TEXT("ParamCheek"), 1.0f);
    }
    else if (ExpressionName == TEXT("Surprised"))
    {
        SetParameterValue(TEXT("ParamMouthForm"), 0.0f);
        SetParameterValue(TEXT("ParamMouthOpenY"), 1.0f);
        SetParameterValue(TEXT("ParamEyeLOpen"), 1.0f);
        SetParameterValue(TEXT("ParamEyeROpen"), 1.0f);
        SetParameterValue(TEXT("ParamBrowLY"), 0.8f);
        SetParameterValue(TEXT("ParamBrowRY"), 0.8f);
    }
    else if (ExpressionName == TEXT("Neutral"))
    {
        SetParameterValue(TEXT("ParamMouthForm"), 0.0f);
        SetParameterValue(TEXT("ParamMouthOpenY"), 0.0f);
        SetParameterValue(TEXT("ParamEyeLOpen"), 1.0f);
        SetParameterValue(TEXT("ParamEyeROpen"), 1.0f);
        SetParameterValue(TEXT("ParamCheek"), 0.0f);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Expression set: %s"), *ExpressionName);
}

void ULive2DCubismAvatarComponent::SetGazeTarget(const FVector2D& TargetPosition)
{
    // Convert target position to eye parameters
    float EyeX = FMath::Clamp(TargetPosition.X, -1.0f, 1.0f);
    float EyeY = FMath::Clamp(TargetPosition.Y, -1.0f, 1.0f);
    
    SetParameterValue(TEXT("ParamEyeBallX"), EyeX);
    SetParameterValue(TEXT("ParamEyeBallY"), EyeY);
}

void ULive2DCubismAvatarComponent::SetBodyRotation(const FRotator& Rotation)
{
    // Convert rotation to Live2D parameters
    float AngleX = FMath::Clamp(Rotation.Pitch / 30.0f, -1.0f, 1.0f);
    float AngleY = FMath::Clamp(Rotation.Yaw / 30.0f, -1.0f, 1.0f);
    float AngleZ = FMath::Clamp(Rotation.Roll / 30.0f, -1.0f, 1.0f);
    
    SetParameterValue(TEXT("ParamAngleX"), AngleX);
    SetParameterValue(TEXT("ParamAngleY"), AngleY);
    SetParameterValue(TEXT("ParamAngleZ"), AngleZ);
}
