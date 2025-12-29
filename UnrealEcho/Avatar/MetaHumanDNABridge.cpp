// MetaHumanDNABridge.cpp
// Deep Tree Echo - MetaHuman DNA Integration Implementation
// Copyright (c) 2025 Deep Tree Echo Project

#include "MetaHumanDNABridge.h"
#include "PythonDNACalibWrapper.h"
#include "../Neurochemical/NeurochemicalSimulationComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Animation/Skeleton.h"

DEFINE_LOG_CATEGORY_STATIC(LogMetaHumanDNA, Log, All);

// ========================================
// Constructor / Destructor
// ========================================

UMetaHumanDNABridge::UMetaHumanDNABridge()
    : bDNALoaded(false)
    , DNAVersion(EDNAVersion::Unknown)
{
    UE_LOG(LogMetaHumanDNA, Log, TEXT("MetaHumanDNABridge initialized"));
}

UMetaHumanDNABridge::~UMetaHumanDNABridge()
{
    if (PythonWrapper.IsValid())
    {
        ShutdownPythonWrapper();
    }
}

// ========================================
// DNA File Operations
// ========================================

bool UMetaHumanDNABridge::LoadDNAFile(const FString& DNAPath)
{
    if (!FPaths::FileExists(DNAPath))
    {
        UE_LOG(LogMetaHumanDNA, Error, TEXT("DNA file not found: %s"), *DNAPath);
        return false;
    }

    // Initialize Python wrapper if not already done
    if (!PythonWrapper.IsValid())
    {
        FString DNACalibPath = FPaths::ProjectDir() / TEXT("MetaHuman-DNA-Calibration");
        if (!InitializePythonWrapper(DNACalibPath))
        {
            UE_LOG(LogMetaHumanDNA, Error, TEXT("Failed to initialize Python wrapper"));
            return false;
        }
    }

    // Load DNA using Python wrapper
    if (!PythonWrapper->LoadDNA(DNAPath))
    {
        UE_LOG(LogMetaHumanDNA, Error, TEXT("Failed to load DNA file: %s"), *DNAPath);
        return false;
    }

    LoadedDNAPath = DNAPath;
    bDNALoaded = true;

    // Parse DNA version
    if (!ParseDNAVersion())
    {
        UE_LOG(LogMetaHumanDNA, Warning, TEXT("Could not determine DNA version, assuming DHI"));
        DNAVersion = EDNAVersion::DHI;
    }

    // Load DNA data into cache
    if (!LoadDNAData())
    {
        UE_LOG(LogMetaHumanDNA, Error, TEXT("Failed to load DNA data"));
        bDNALoaded = false;
        return false;
    }

    // Build cache maps
    BuildCacheMaps();

    // Initialize blend shape weights
    InitializeBlendShapeWeights();

    UE_LOG(LogMetaHumanDNA, Log, TEXT("Successfully loaded DNA file: %s (Version: %s)"), 
        *DNAPath, *GetDNAVersionString());

    return true;
}

bool UMetaHumanDNABridge::SaveDNAFile(const FString& DNAPath)
{
    if (!bDNALoaded || !PythonWrapper.IsValid())
    {
        UE_LOG(LogMetaHumanDNA, Error, TEXT("No DNA loaded or Python wrapper not initialized"));
        return false;
    }

    if (!PythonWrapper->SaveDNA(DNAPath))
    {
        UE_LOG(LogMetaHumanDNA, Error, TEXT("Failed to save DNA file: %s"), *DNAPath);
        return false;
    }

    UE_LOG(LogMetaHumanDNA, Log, TEXT("Successfully saved DNA file: %s"), *DNAPath);
    return true;
}

FString UMetaHumanDNABridge::GetDNAVersionString() const
{
    switch (DNAVersion)
    {
    case EDNAVersion::DHI:
        return TEXT("DHI");
    case EDNAVersion::MH4:
        return TEXT("MH.4");
    default:
        return TEXT("Unknown");
    }
}

// ========================================
// Rig Structure Access
// ========================================

TArray<FString> UMetaHumanDNABridge::GetJointNames() const
{
    if (!bDNALoaded || !PythonWrapper.IsValid())
    {
        return TArray<FString>();
    }

    return PythonWrapper->GetJointNames();
}

TArray<FString> UMetaHumanDNABridge::GetMeshNames() const
{
    TArray<FString> MeshNames;
    for (const FDNAMeshInfo& MeshInfo : MeshInfoCache)
    {
        MeshNames.Add(MeshInfo.MeshName);
    }
    return MeshNames;
}

TArray<FString> UMetaHumanDNABridge::GetBlendShapeNames() const
{
    if (!bDNALoaded || !PythonWrapper.IsValid())
    {
        return TArray<FString>();
    }

    return PythonWrapper->GetBlendShapeNames();
}

bool UMetaHumanDNABridge::GetJointInfo(const FString& JointName, FDNAJointInfo& OutJointInfo) const
{
    const int32* IndexPtr = JointNameToIndexMap.Find(JointName);
    if (!IndexPtr || !JointInfoCache.IsValidIndex(*IndexPtr))
    {
        return false;
    }

    OutJointInfo = JointInfoCache[*IndexPtr];
    return true;
}

bool UMetaHumanDNABridge::GetBlendShapeInfo(const FString& BlendShapeName, FDNABlendShapeInfo& OutBlendShapeInfo) const
{
    const int32* IndexPtr = BlendShapeNameToIndexMap.Find(BlendShapeName);
    if (!IndexPtr || !BlendShapeInfoCache.IsValidIndex(*IndexPtr))
    {
        return false;
    }

    OutBlendShapeInfo = BlendShapeInfoCache[*IndexPtr];
    return true;
}

bool UMetaHumanDNABridge::GetMeshInfo(const FString& MeshName, FDNAMeshInfo& OutMeshInfo) const
{
    const int32* IndexPtr = MeshNameToIndexMap.Find(MeshName);
    if (!IndexPtr || !MeshInfoCache.IsValidIndex(*IndexPtr))
    {
        return false;
    }

    OutMeshInfo = MeshInfoCache[*IndexPtr];
    return true;
}

FTransform UMetaHumanDNABridge::GetNeutralJointTransform(const FString& JointName) const
{
    FDNAJointInfo JointInfo;
    if (GetJointInfo(JointName, JointInfo))
    {
        return JointInfo.NeutralTransform;
    }
    return FTransform::Identity;
}

// ========================================
// DNA Manipulation
// ========================================

bool UMetaHumanDNABridge::RenameJoint(const FString& OldName, const FString& NewName)
{
    if (!bDNALoaded || !PythonWrapper.IsValid())
    {
        UE_LOG(LogMetaHumanDNA, Error, TEXT("No DNA loaded"));
        return false;
    }

    // Check for critical joints that should not be renamed
    if (OldName == TEXT("neck_01") || OldName == TEXT("neck_02") || OldName == TEXT("FACIAL_C_FacialRoot"))
    {
        UE_LOG(LogMetaHumanDNA, Error, TEXT("Cannot rename critical joint: %s"), *OldName);
        return false;
    }

    if (!PythonWrapper->RenameJoint(OldName, NewName))
    {
        UE_LOG(LogMetaHumanDNA, Error, TEXT("Failed to rename joint: %s -> %s"), *OldName, *NewName);
        return false;
    }

    // Update cache
    LoadDNAData();
    BuildCacheMaps();

    UE_LOG(LogMetaHumanDNA, Log, TEXT("Successfully renamed joint: %s -> %s"), *OldName, *NewName);
    return true;
}

bool UMetaHumanDNABridge::RemoveJoint(const FString& JointName)
{
    if (!bDNALoaded || !PythonWrapper.IsValid())
    {
        UE_LOG(LogMetaHumanDNA, Error, TEXT("No DNA loaded"));
        return false;
    }

    // Check for critical joints that should not be removed
    if (JointName == TEXT("neck_01") || JointName == TEXT("neck_02") || JointName == TEXT("FACIAL_C_FacialRoot"))
    {
        UE_LOG(LogMetaHumanDNA, Error, TEXT("Cannot remove critical joint: %s"), *JointName);
        return false;
    }

    if (!PythonWrapper->RemoveJoint(JointName))
    {
        UE_LOG(LogMetaHumanDNA, Error, TEXT("Failed to remove joint: %s"), *JointName);
        return false;
    }

    // Update cache
    LoadDNAData();
    BuildCacheMaps();

    UE_LOG(LogMetaHumanDNA, Log, TEXT("Successfully removed joint: %s"), *JointName);
    return true;
}

bool UMetaHumanDNABridge::RenameBlendShape(const FString& OldName, const FString& NewName)
{
    if (!bDNALoaded || !PythonWrapper.IsValid())
    {
        UE_LOG(LogMetaHumanDNA, Error, TEXT("No DNA loaded"));
        return false;
    }

    // Rename in Python wrapper (implementation needed in wrapper)
    // For now, log warning
    UE_LOG(LogMetaHumanDNA, Warning, TEXT("RenameBlendShape not yet implemented in Python wrapper"));
    return false;
}

bool UMetaHumanDNABridge::RemoveBlendShape(const FString& BlendShapeName)
{
    if (!bDNALoaded || !PythonWrapper.IsValid())
    {
        UE_LOG(LogMetaHumanDNA, Error, TEXT("No DNA loaded"));
        return false;
    }

    // Remove in Python wrapper (implementation needed in wrapper)
    // For now, log warning
    UE_LOG(LogMetaHumanDNA, Warning, TEXT("RemoveBlendShape not yet implemented in Python wrapper"));
    return false;
}

bool UMetaHumanDNABridge::ModifyBlendShapeDeltas(const FString& BlendShapeName, const TArray<FVector>& Deltas)
{
    if (!bDNALoaded || !PythonWrapper.IsValid())
    {
        UE_LOG(LogMetaHumanDNA, Error, TEXT("No DNA loaded"));
        return false;
    }

    // Modify in Python wrapper (implementation needed in wrapper)
    // For now, log warning
    UE_LOG(LogMetaHumanDNA, Warning, TEXT("ModifyBlendShapeDeltas not yet implemented in Python wrapper"));
    return false;
}

bool UMetaHumanDNABridge::ClearAllBlendShapes()
{
    if (!bDNALoaded || !PythonWrapper.IsValid())
    {
        UE_LOG(LogMetaHumanDNA, Error, TEXT("No DNA loaded"));
        return false;
    }

    // Clear in Python wrapper (implementation needed in wrapper)
    // For now, log warning
    UE_LOG(LogMetaHumanDNA, Warning, TEXT("ClearAllBlendShapes not yet implemented in Python wrapper"));
    return false;
}

bool UMetaHumanDNABridge::RemoveLOD(int32 LODIndex)
{
    if (!bDNALoaded || !PythonWrapper.IsValid())
    {
        UE_LOG(LogMetaHumanDNA, Error, TEXT("No DNA loaded"));
        return false;
    }

    // Remove LOD in Python wrapper (implementation needed in wrapper)
    // For now, log warning
    UE_LOG(LogMetaHumanDNA, Warning, TEXT("RemoveLOD not yet implemented in Python wrapper"));
    return false;
}

// ========================================
// Integration with Avatar System
// ========================================

bool UMetaHumanDNABridge::ApplyDNAToSkeleton(USkeletalMeshComponent* SkeletalMesh)
{
    if (!bDNALoaded || !SkeletalMesh)
    {
        UE_LOG(LogMetaHumanDNA, Error, TEXT("No DNA loaded or invalid skeletal mesh"));
        return false;
    }

    USkeletalMesh* Mesh = SkeletalMesh->SkeletalMesh;
    if (!Mesh)
    {
        UE_LOG(LogMetaHumanDNA, Error, TEXT("Skeletal mesh component has no mesh"));
        return false;
    }

    USkeleton* Skeleton = Mesh->GetSkeleton();
    if (!Skeleton)
    {
        UE_LOG(LogMetaHumanDNA, Error, TEXT("Skeletal mesh has no skeleton"));
        return false;
    }

    // Map DNA joints to Unreal skeleton
    TArray<FString> DNAJoints = GetJointNames();
    const FReferenceSkeleton& RefSkeleton = Skeleton->GetReferenceSkeleton();

    int32 MappedJoints = 0;
    for (const FString& DNAJointName : DNAJoints)
    {
        FName BoneName(*DNAJointName);
        int32 BoneIndex = RefSkeleton.FindBoneIndex(BoneName);
        
        if (BoneIndex != INDEX_NONE)
        {
            MappedJoints++;
            // Successfully mapped joint
            // Could store mapping for future use
        }
    }

    UE_LOG(LogMetaHumanDNA, Log, TEXT("Mapped %d/%d DNA joints to skeleton"), MappedJoints, DNAJoints.Num());

    return MappedJoints > 0;
}

void UMetaHumanDNABridge::SyncNeurochemicalStateToBlendShapes(const FNeurochemicalState& State, TMap<FString, float>& OutBlendShapeWeights)
{
    MapNeurochemicalToBlendShapes(State, OutBlendShapeWeights);
}

void UMetaHumanDNABridge::SyncEmotionalStateToFacialRig(const FEmotionalState& Emotion, TMap<FString, float>& OutBlendShapeWeights)
{
    MapEmotionalStateToBlendShapes(Emotion, OutBlendShapeWeights);
}

float UMetaHumanDNABridge::GetBlendShapeWeight(const FString& BlendShapeName) const
{
    const float* WeightPtr = CurrentBlendShapeWeights.Find(BlendShapeName);
    return WeightPtr ? *WeightPtr : 0.0f;
}

void UMetaHumanDNABridge::SetBlendShapeWeight(const FString& BlendShapeName, float Weight)
{
    CurrentBlendShapeWeights.Add(BlendShapeName, FMath::Clamp(Weight, 0.0f, 1.0f));
}

TMap<FString, float> UMetaHumanDNABridge::GetAllBlendShapeWeights() const
{
    return CurrentBlendShapeWeights;
}

void UMetaHumanDNABridge::SetAllBlendShapeWeights(const TMap<FString, float>& Weights)
{
    CurrentBlendShapeWeights = Weights;
    
    // Clamp all weights to 0-1
    for (auto& Pair : CurrentBlendShapeWeights)
    {
        Pair.Value = FMath::Clamp(Pair.Value, 0.0f, 1.0f);
    }
}

// ========================================
// Advanced Features
// ========================================

bool UMetaHumanDNABridge::InitializePythonWrapper(const FString& DNACalibPath)
{
    if (PythonWrapper.IsValid())
    {
        UE_LOG(LogMetaHumanDNA, Warning, TEXT("Python wrapper already initialized"));
        return true;
    }

    PythonWrapper = MakeShared<FPythonDNACalibWrapper>();
    
    if (!PythonWrapper->Initialize(DNACalibPath))
    {
        UE_LOG(LogMetaHumanDNA, Error, TEXT("Failed to initialize Python wrapper with path: %s"), *DNACalibPath);
        PythonWrapper.Reset();
        return false;
    }

    UE_LOG(LogMetaHumanDNA, Log, TEXT("Python wrapper initialized successfully"));
    return true;
}

void UMetaHumanDNABridge::ShutdownPythonWrapper()
{
    if (PythonWrapper.IsValid())
    {
        PythonWrapper->Shutdown();
        PythonWrapper.Reset();
        UE_LOG(LogMetaHumanDNA, Log, TEXT("Python wrapper shut down"));
    }
}

void UMetaHumanDNABridge::GetDNAStatistics(int32& OutJointCount, int32& OutMeshCount, int32& OutBlendShapeCount) const
{
    OutJointCount = JointInfoCache.Num();
    OutMeshCount = MeshInfoCache.Num();
    OutBlendShapeCount = BlendShapeInfoCache.Num();
}

bool UMetaHumanDNABridge::ValidateDNAIntegrity() const
{
    if (!bDNALoaded)
    {
        return false;
    }

    // Check for critical joints
    TArray<FString> CriticalJoints = { TEXT("neck_01"), TEXT("neck_02"), TEXT("FACIAL_C_FacialRoot") };
    TArray<FString> JointNames = GetJointNames();

    for (const FString& CriticalJoint : CriticalJoints)
    {
        if (!JointNames.Contains(CriticalJoint))
        {
            UE_LOG(LogMetaHumanDNA, Error, TEXT("Critical joint missing: %s"), *CriticalJoint);
            return false;
        }
    }

    return true;
}

// ========================================
// Helper Functions
// ========================================

void UMetaHumanDNABridge::BuildCacheMaps()
{
    JointNameToIndexMap.Empty();
    BlendShapeNameToIndexMap.Empty();
    MeshNameToIndexMap.Empty();

    for (int32 i = 0; i < JointInfoCache.Num(); ++i)
    {
        JointNameToIndexMap.Add(JointInfoCache[i].JointName, i);
    }

    for (int32 i = 0; i < BlendShapeInfoCache.Num(); ++i)
    {
        BlendShapeNameToIndexMap.Add(BlendShapeInfoCache[i].BlendShapeName, i);
    }

    for (int32 i = 0; i < MeshInfoCache.Num(); ++i)
    {
        MeshNameToIndexMap.Add(MeshInfoCache[i].MeshName, i);
    }
}

void UMetaHumanDNABridge::ClearCache()
{
    JointInfoCache.Empty();
    BlendShapeInfoCache.Empty();
    MeshInfoCache.Empty();
    JointNameToIndexMap.Empty();
    BlendShapeNameToIndexMap.Empty();
    MeshNameToIndexMap.Empty();
    CurrentBlendShapeWeights.Empty();
}

bool UMetaHumanDNABridge::LoadDNAData()
{
    if (!PythonWrapper.IsValid())
    {
        return false;
    }

    ClearCache();

    // Load joint data
    TArray<FString> JointNames = PythonWrapper->GetJointNames();
    for (int32 i = 0; i < JointNames.Num(); ++i)
    {
        FDNAJointInfo JointInfo;
        JointInfo.JointName = JointNames[i];
        JointInfo.JointIndex = i;
        // TODO: Get neutral transform from Python wrapper
        JointInfo.NeutralTransform = FTransform::Identity;
        JointInfo.ParentIndex = -1; // TODO: Get parent index from Python wrapper
        
        JointInfoCache.Add(JointInfo);
    }

    // Load blend shape data
    TArray<FString> BlendShapeNames = PythonWrapper->GetBlendShapeNames();
    for (int32 i = 0; i < BlendShapeNames.Num(); ++i)
    {
        FDNABlendShapeInfo BlendShapeInfo;
        BlendShapeInfo.BlendShapeName = BlendShapeNames[i];
        BlendShapeInfo.BlendShapeIndex = i;
        // TODO: Get target mesh and vertex count from Python wrapper
        BlendShapeInfo.TargetMeshName = TEXT("");
        BlendShapeInfo.VertexCount = 0;
        
        BlendShapeInfoCache.Add(BlendShapeInfo);
    }

    // TODO: Load mesh data from Python wrapper

    return true;
}

bool UMetaHumanDNABridge::ParseDNAVersion()
{
    if (!PythonWrapper.IsValid())
    {
        return false;
    }

    // TODO: Get DNA version from Python wrapper
    // For now, assume DHI
    DNAVersion = EDNAVersion::DHI;
    return true;
}

void UMetaHumanDNABridge::InitializeBlendShapeWeights()
{
    CurrentBlendShapeWeights.Empty();

    // Initialize all blend shapes to 0
    for (const FDNABlendShapeInfo& BlendShapeInfo : BlendShapeInfoCache)
    {
        CurrentBlendShapeWeights.Add(BlendShapeInfo.BlendShapeName, 0.0f);
    }
}

void UMetaHumanDNABridge::MapNeurochemicalToBlendShapes(const FNeurochemicalState& State, TMap<FString, float>& OutWeights)
{
    // Example mapping: Dopamine -> Smile intensity
    // This is a simplified example; real mapping would be more complex
    
    OutWeights.Empty();

    // Map dopamine to smile/happiness blend shapes
    float SmileIntensity = State.Dopamine * 0.8f + State.Serotonin * 0.5f;
    OutWeights.Add(TEXT("Smile_L"), SmileIntensity);
    OutWeights.Add(TEXT("Smile_R"), SmileIntensity);

    // Map cortisol to stress/tension blend shapes
    float StressIntensity = State.Cortisol * 0.7f;
    OutWeights.Add(TEXT("BrowTension_L"), StressIntensity);
    OutWeights.Add(TEXT("BrowTension_R"), StressIntensity);

    // Map oxytocin to warmth/openness blend shapes
    float WarmthIntensity = State.Oxytocin * 0.6f;
    OutWeights.Add(TEXT("EyeWarmth_L"), WarmthIntensity);
    OutWeights.Add(TEXT("EyeWarmth_R"), WarmthIntensity);

    // Map norepinephrine to alertness blend shapes
    float AlertnessIntensity = State.Norepinephrine * 0.5f;
    OutWeights.Add(TEXT("EyeOpen_L"), AlertnessIntensity);
    OutWeights.Add(TEXT("EyeOpen_R"), AlertnessIntensity);

    // TODO: Add more sophisticated mappings based on DNA version
}

void UMetaHumanDNABridge::MapEmotionalStateToBlendShapes(const FEmotionalState& Emotion, TMap<FString, float>& OutWeights)
{
    // Example mapping: Emotional state to facial expressions
    // This is a simplified example; real mapping would be more complex
    
    OutWeights.Empty();

    // Map valence to smile/frown
    if (Emotion.Valence > 0.0f)
    {
        OutWeights.Add(TEXT("Smile_L"), Emotion.Valence * Emotion.Intensity);
        OutWeights.Add(TEXT("Smile_R"), Emotion.Valence * Emotion.Intensity);
    }
    else
    {
        OutWeights.Add(TEXT("Frown_L"), -Emotion.Valence * Emotion.Intensity);
        OutWeights.Add(TEXT("Frown_R"), -Emotion.Valence * Emotion.Intensity);
    }

    // Map arousal to eye openness and brow raise
    OutWeights.Add(TEXT("EyeOpen_L"), Emotion.Arousal * 0.5f);
    OutWeights.Add(TEXT("EyeOpen_R"), Emotion.Arousal * 0.5f);
    OutWeights.Add(TEXT("BrowRaise_L"), Emotion.Arousal * 0.3f);
    OutWeights.Add(TEXT("BrowRaise_R"), Emotion.Arousal * 0.3f);

    // TODO: Add more sophisticated mappings based on DNA version and emotional categories
}

TMap<FString, FString> UMetaHumanDNABridge::GetDHIBlendShapeNames() const
{
    // DHI blend shape name mappings
    TMap<FString, FString> Mappings;
    
    // TODO: Add comprehensive DHI blend shape mappings
    Mappings.Add(TEXT("Smile_L"), TEXT("CTRL_L_mouth_smile"));
    Mappings.Add(TEXT("Smile_R"), TEXT("CTRL_R_mouth_smile"));
    Mappings.Add(TEXT("Frown_L"), TEXT("CTRL_L_mouth_frown"));
    Mappings.Add(TEXT("Frown_R"), TEXT("CTRL_R_mouth_frown"));
    
    return Mappings;
}

TMap<FString, FString> UMetaHumanDNABridge::GetMH4BlendShapeNames() const
{
    // MH.4 blend shape name mappings
    TMap<FString, FString> Mappings;
    
    // TODO: Add comprehensive MH.4 blend shape mappings
    Mappings.Add(TEXT("Smile_L"), TEXT("CTRL_L_mouth_smile"));
    Mappings.Add(TEXT("Smile_R"), TEXT("CTRL_R_mouth_smile"));
    Mappings.Add(TEXT("Frown_L"), TEXT("CTRL_L_mouth_frown"));
    Mappings.Add(TEXT("Frown_R"), TEXT("CTRL_R_mouth_frown"));
    
    return Mappings;
}
