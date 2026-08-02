// CoreMinimal.h - standalone build shim.
//
// Lets the REAL MasterySignal.h compile outside Unreal, so the integration test exercises the
// actual contract header rather than a mirrored copy of it. Deliberately minimal: if a header
// under test starts needing more than this, that is a signal it has grown an engine dependency
// it should not have.
#pragma once
#include <cstdint>
#include <cmath>
using int32 = std::int32_t;
using uint8 = std::uint8_t;
