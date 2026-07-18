// NiagaraComponent.h - Stub for standalone compilation testing
// In production, the actual Unreal Engine Niagara plugin header will be used.
#pragma once
#include "CoreMinimal.h"

class UNiagaraSystem : public UObject {
public:
    FString GetName() const { return FString(); }
};

class UNiagaraComponent : public USceneComponent {
public:
    void SetAsset(UNiagaraSystem*) {}
    void Activate(bool bReset = false) { (void)bReset; }
    void Deactivate() {}
    void SetVariableFloat(const FName&, float) {}
    void SetVariableVec3(const FName&, const FVector&) {}
    void SetVariableLinearColor(const FName&, const FLinearColor&) {}
    void SetFloatParameter(const FName&, float) {}
    void SetColorParameter(const FName&, const FLinearColor&) {}
    void SetVectorParameter(const FName&, const FVector&) {}
    void SetIntParameter(const FName&, int32) {}
    void SetBoolParameter(const FName&, bool) {}
    bool IsActive() const { return false; }
    void SetVisibility(bool, bool bPropagate = false) { (void)bPropagate; }
};
