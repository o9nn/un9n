// Animation/AnimInstance.h - Stub for standalone compilation testing
#pragma once
#include "CoreMinimal.h"

class UAnimInstance : public UObject {
public:
    virtual void NativeInitializeAnimation() {}
    virtual void NativeUpdateAnimation(float DeltaSeconds) {}
    
    APawn* TryGetPawnOwner() const { return nullptr; }
    
    float GetInstanceAssetPlayerLength(int32 AssetPlayerIndex) { return 0.0f; }
    float GetInstanceAssetPlayerTime(int32 AssetPlayerIndex) { return 0.0f; }
    
    void Montage_Play(class UAnimMontage* MontageToPlay, float InPlayRate = 1.0f) {}
    void Montage_Stop(float InBlendOutTime, class UAnimMontage* Montage = nullptr) {}
    bool Montage_IsPlaying(class UAnimMontage* Montage) const { return false; }
    void SetRootMotionMode(int Mode) { (void)Mode; }
    void Montage_SetPlayRate(class UAnimMontage*, float) {}
    float Montage_GetPosition(class UAnimMontage*) const { return 0.0f; }
    void Montage_SetPosition(class UAnimMontage*, float) {}
};

class UAnimMontage : public UObject {
public:
    float GetPlayLength() const { return 0.0f; }
};

namespace EBoneSpaces {
    enum Type { WorldSpace, ComponentSpace };
}

class USkeletalMeshComponent : public USceneComponent {
public:
    UAnimInstance* GetAnimInstance() const { return nullptr; }
    void SetAnimInstanceClass(UClass* NewClass) {}
    void PlayAnimation(class UAnimationAsset* NewAnimToPlay, bool bLooping) {}

    // Bone / socket query surface (used by 4E body schema + MetaHuman DNA bridge)
    int32 GetNumBones() const { return BoneNames.Num(); }
    FName GetBoneName(int32 BoneIndex) const {
        return BoneNames.IsValidIndex(BoneIndex) ? BoneNames[BoneIndex] : FName();
    }
    int32 GetBoneIndex(const FName& BoneName) const {
        for (int32 i = 0; i < BoneNames.Num(); ++i)
            if (BoneNames[i] == BoneName) return i;
        return -1;
    }
    FVector GetBoneLocation(const FName&) const { return FVector(); }
    FTransform GetBoneTransform(int32) const { return FTransform(); }
    FTransform GetSocketTransform(const FName&) const { return FTransform(); }
    bool DoesSocketExist(const FName&) const { return false; }
    void GetBoneNames(TArray<FName>& OutNames) const { OutNames = BoneNames; }
    void SetBoneTransformByName(const FName&, const FTransform&, int /*EBoneSpaces::Type*/) {}
    FVector GetBoneLocation(const FName&, int /*EBoneSpaces::Type*/) const { return FVector(); }
    int32 GetNumMaterials() const { return 0; }
    class UMaterialInterface* GetMaterial(int32) const { return nullptr; }
    void SetMaterial(int32, class UMaterialInterface*) {}
    class UMaterialInstanceDynamic* CreateDynamicMaterialInstance(int32, class UMaterialInterface* = nullptr) { return nullptr; }

    // Morph target surface (used by FACS / Rig Logic expression pipeline)
    void SetMorphTarget(const FName& MorphTargetName, float Value) {
        MorphTargets.FindOrAdd(MorphTargetName) = Value;
    }
    float GetMorphTarget(const FName& MorphTargetName) const {
        const float* V = MorphTargets.Find(MorphTargetName);
        return V ? *V : 0.0f;
    }
    void ClearMorphTargets() { MorphTargets.Empty(); }

private:
    TArray<FName> BoneNames;
    TMap<FName, float> MorphTargets;
};
