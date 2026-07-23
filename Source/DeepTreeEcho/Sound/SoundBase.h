// Stub for standalone compilation. Production uses the real UE header.
#pragma once
#include "CoreMinimal.h"
class USoundBase : public UObject {
public:
    float GetDuration() const { return 0.0f; }
};
