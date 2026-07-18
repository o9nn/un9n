// Stub for standalone compilation. Production uses the real UE header.
#pragma once
#include "CoreMinimal.h"
struct UGameplayStatics {
    static class APawn* GetPlayerPawn(const UObject*, int32) { return nullptr; }
    static class AActor* GetPlayerCharacter(const UObject*, int32) { return nullptr; }
    static float GetWorldDeltaSeconds(const UObject*) { return 0.016f; }
    static float GetTimeSeconds(const UObject*) { return 0.0f; }
    static void GetAllActorsInRadius(class UWorld*, const FVector&, float, TArray<class AActor*>& OutActors) { OutActors.Empty(); }
};
