// Stub for standalone compilation. Production uses the real UE header.
#pragma once
#include "CoreMinimal.h"
class UMaterialInstanceDynamic : public UObject {
public:
    void SetScalarParameterValue(const FName&, float) {}
    void SetVectorParameterValue(const FName&, const FLinearColor&) {}
    static UMaterialInstanceDynamic* Create(UObject*, UObject*) { return new UMaterialInstanceDynamic(); }
};
