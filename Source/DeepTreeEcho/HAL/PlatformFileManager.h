// Stub for standalone compilation. Production uses the real UE header.
#pragma once
#include "CoreMinimal.h"
class FPlatformFileManager {
public:
    static FPlatformFileManager& Get() { static FPlatformFileManager M; return M; }
};
