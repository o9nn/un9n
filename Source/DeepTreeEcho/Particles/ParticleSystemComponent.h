#pragma once
// UE-compat shim (standalone builds)
#include "CoreMinimal.h"

class UParticleSystem : public UObject {
public:
};

class UParticleSystemComponent : public USceneComponent {
public:
    void SetTemplate(UParticleSystem* InTemplate) {}
    void SetFloatParameter(const FName& Name, float Value) {}
    void SetVectorParameter(const FName& Name, const FVector& Value) {}
    void SetColorParameter(const FName& Name, const FLinearColor& Value) {}
    void Activate(bool bReset = false) {}
    void Deactivate() {}
    void ActivateSystem(bool bFlagAsJustAttached = false) {}
    void DeactivateSystem() {}
    bool IsActive() const { return true; }
};
