// CoreMinimal.h - standalone build shim.
//
// Lets the engine-independent Mastery headers compile outside Unreal, so the REAL headers are
// under test rather than mirrored copies of them.
//
// SCOPE DISCIPLINE. This provides only what the *pure data and pure logic* headers need: scalar
// typedefs, a minimal FString/TArray, and the FMath calls actually used. It deliberately does
// NOT fake UCLASS, USTRUCT, UPROPERTY, UActorComponent or delegates. If a header under test
// starts needing those, that is a real signal it has acquired an engine dependency and should
// either be split or excluded from the standalone survey - not a prompt to grow this file.
//
// Semantics are matched where it matters (FMath::Clamp argument order, CountBits) so a header
// that compiles here behaves the same way when compiled for real.

#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <initializer_list>
#include <string>
#include <vector>

using int32  = std::int32_t;
using int64  = std::int64_t;
using uint8  = std::uint8_t;
using uint32 = std::uint32_t;
using uint64 = std::uint64_t;
using TCHAR  = char;

#define TEXT(x) x

/** Minimal FString: enough for names and identifiers in pure-data headers. */
class FString
{
public:
    FString() = default;
    FString(const char* In) : Str(In ? In : "") {}
    FString(const std::string& In) : Str(In) {}

    bool IsEmpty() const { return Str.empty(); }
    const char* operator*() const { return Str.c_str(); }
    bool operator==(const FString& O) const { return Str == O.Str; }
    bool operator!=(const FString& O) const { return Str != O.Str; }
    FString operator+(const FString& O) const { return FString(Str + O.Str); }

    std::string Str;
};

/** Minimal TArray over std::vector - only the operations the pure headers use. */
template <typename T>
class TArray
{
public:
    TArray() = default;
    TArray(std::initializer_list<T> Init) : Data(Init) {}

    int32 Num() const { return static_cast<int32>(Data.size()); }
    void Add(const T& V) { Data.push_back(V); }
    void Empty() { Data.clear(); }
    void Reset() { Data.clear(); }
    void Reserve(int32 N) { Data.reserve(static_cast<size_t>(N)); }
    bool IsValidIndex(int32 i) const { return i >= 0 && i < Num(); }
    bool Contains(const T& V) const { return std::find(Data.begin(), Data.end(), V) != Data.end(); }
    void AddUnique(const T& V) { if (!Contains(V)) Add(V); }

    T& operator[](int32 i) { return Data[static_cast<size_t>(i)]; }
    const T& operator[](int32 i) const { return Data[static_cast<size_t>(i)]; }
    T& Last() { return Data.back(); }
    const T& Last() const { return Data.back(); }

    auto begin() { return Data.begin(); }
    auto end() { return Data.end(); }
    auto begin() const { return Data.begin(); }
    auto end() const { return Data.end(); }

    std::vector<T> Data;
};

/** Minimal FMath. Argument order matches Unreal's so behaviour is identical. */
struct FMath
{
    template <typename T> static T Clamp(T V, T Lo, T Hi) { return V < Lo ? Lo : (V > Hi ? Hi : V); }
    template <typename T> static T Max(T A, T B) { return A > B ? A : B; }
    template <typename T> static T Min(T A, T B) { return A < B ? A : B; }
    template <typename T> static T Abs(T V) { return V < T(0) ? -V : V; }
    template <typename T> static T Square(T V) { return V * V; }

    static float Sqrt(float V) { return std::sqrt(V); }
    static float Pow(float A, float B) { return std::pow(A, B); }
    static float Loge(float V) { return std::log(V); }
    static float Exp(float V) { return std::exp(V); }
    static float Tanh(float V) { return std::tanh(V); }
    static int32 RoundToInt(float V) { return static_cast<int32>(std::lround(V)); }
    static int32 FloorToInt(float V) { return static_cast<int32>(std::floor(V)); }
    static float Lerp(float A, float B, float T) { return A + (B - A) * T; }
    static int32 CountBits(uint64 V) { int32 C = 0; while (V) { C += int32(V & 1ull); V >>= 1; } return C; }
    static float Sign(float V) { return V > 0.0f ? 1.0f : (V < 0.0f ? -1.0f : 0.0f); }
};

#ifndef KINDA_SMALL_NUMBER
#define KINDA_SMALL_NUMBER 1.0e-4f
#endif
