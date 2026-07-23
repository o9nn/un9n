// Stub for standalone compilation. Production uses the real UE header.
#pragma once
#include "CoreMinimal.h"
#include "Sound/SoundBase.h"
class UAudioComponent : public UActorComponent {
public:
    void SetSound(USoundBase*) {}
    void Play(float StartTime = 0.0f) { (void)StartTime; }
    void Stop() {}
    void SetVolumeMultiplier(float) {}
    void SetPitchMultiplier(float) {}
    bool IsPlaying() const { return false; }
};
