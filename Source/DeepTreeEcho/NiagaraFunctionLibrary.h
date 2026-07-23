// NiagaraFunctionLibrary.h - Stub for standalone compilation testing
// In production, the actual Unreal Engine Niagara plugin header will be used.
#pragma once
#include "CoreMinimal.h"
#include "NiagaraComponent.h"

struct UNiagaraFunctionLibrary {
    static UNiagaraComponent* SpawnSystemAttached(
        UNiagaraSystem* SystemTemplate, USceneComponent* AttachToComponent,
        FName AttachPointName, FVector Location, FRotator Rotation,
        int LocationType, bool bAutoDestroy) {
        (void)SystemTemplate; (void)AttachToComponent; (void)AttachPointName;
        (void)Location; (void)Rotation; (void)LocationType; (void)bAutoDestroy;
        static UNiagaraComponent Component;
        return &Component;
    }
};
