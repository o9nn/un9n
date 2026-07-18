// CoreMinimal.h - Stub header for standalone compilation testing
// This file provides minimal type definitions to allow syntax checking
// outside of the Unreal Engine environment.
//
// IMPORTANT: This file is ONLY for development validation.
// In production, this file should NOT be included - the actual
// Unreal Engine CoreMinimal.h will be used instead.

#pragma once

#ifndef COREMINIMAL_STUB_H
#define COREMINIMAL_STUB_H

// Standard library includes
#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <map>
#include <set>
#include <algorithm>
#include <cmath>
#include <chrono>
#include <mutex>
#include <atomic>
#include <thread>
#include <condition_variable>
#include <cfloat>

// =============================================================================
// UNREAL ENGINE TYPE STUBS
// =============================================================================

// Basic types
using int8 = int8_t;
using int16 = int16_t;
using int32 = int32_t;
using int64 = int64_t;
using uint8 = uint8_t;
using uint16 = uint16_t;
using uint32 = uint32_t;
using uint64 = uint64_t;

// Character type
using TCHAR = char;

// String types
class FString : public std::string {
public:
    FString() = default;
    FString(const char* s) : std::string(s) {}
    FString(const std::string& s) : std::string(s) {}
    
    const TCHAR* operator*() const { return c_str(); }
    bool IsEmpty() const { return empty(); }
    int32 Len() const { return static_cast<int32>(length()); }
    
    static FString Printf(const TCHAR* Format, ...) { return FString(); }
    static FString FromInt(int32 Val) { return FString(std::to_string(Val)); }
    
    FString& Append(const FString& Other) { append(Other); return *this; }
    
    bool Contains(const TCHAR* SubStr) const { return find(SubStr) != std::string::npos; }
    bool Contains(const FString& SubStr) const { return find(SubStr) != std::string::npos; }
    
    bool StartsWith(const TCHAR* Prefix) const { return rfind(Prefix, 0) == 0; }
    bool StartsWith(const FString& Prefix) const { return rfind(Prefix, 0) == 0; }
    bool EndsWith(const TCHAR* Suffix) const {
        const std::string Suf(Suffix);
        return length() >= Suf.length() && compare(length() - Suf.length(), Suf.length(), Suf) == 0;
    }
    bool EndsWith(const FString& Suffix) const {
        return length() >= Suffix.length() && compare(length() - Suffix.length(), Suffix.length(), Suffix) == 0;
    }
    
    int32 Find(const TCHAR* SubStr) const {
        size_t pos = find(SubStr);
        return pos != std::string::npos ? static_cast<int32>(pos) : -1;
    }
    
    FString Left(int32 Count) const { return FString(substr(0, Count)); }
    FString Right(int32 Count) const { return FString(substr(length() - Count)); }
    FString Mid(int32 Start, int32 Count = INT32_MAX) const { return FString(substr(Start, Count)); }
    
    void ToLowerInline() { std::transform(begin(), end(), begin(), ::tolower); }
    void ToUpperInline() { std::transform(begin(), end(), begin(), ::toupper); }
    FString ToLower() const { FString Result = *this; Result.ToLowerInline(); return Result; }
    FString ToUpper() const { FString Result = *this; Result.ToUpperInline(); return Result; }
};

class FName {
public:
    FName() = default;
    FName(const char* s) : Name(s) {}
    FName(const FString& s) : Name(s) {}
    
    bool IsNone() const { return Name.empty(); }
    FString ToString() const { return Name; }
    
    bool operator==(const FName& Other) const { return Name == Other.Name; }
    bool operator!=(const FName& Other) const { return Name != Other.Name; }
    bool operator<(const FName& Other) const { return static_cast<const std::string&>(Name) < static_cast<const std::string&>(Other.Name); }
    
private:
    FString Name;
};

static const FName NAME_None;


// Globally-unique identifier stub
struct FGuid {
    static bool Parse(const FString& GuidString, FGuid& OutGuid);

    uint32 A = 0, B = 0, C = 0, D = 0;
    FGuid() = default;
    FGuid(uint32 InA, uint32 InB, uint32 InC, uint32 InD) : A(InA), B(InB), C(InC), D(InD) {}
    static FGuid NewGuid() {
        static std::atomic<uint32> Counter{1};
        uint32 N = Counter++;
        return FGuid(N, N * 2654435761u, N ^ 0x9E3779B9u, ~N);
    }
    bool IsValid() const { return A | B | C | D; }
    void Invalidate() { A = B = C = D = 0; }
    FString ToString() const {
        char Buf[40];
        std::snprintf(Buf, sizeof(Buf), "%08X%08X%08X%08X", A, B, C, D);
        return FString(Buf);
    }
    bool operator==(const FGuid& O) const { return A == O.A && B == O.B && C == O.C && D == O.D; }
    bool operator!=(const FGuid& O) const { return !(*this == O); }
    bool operator<(const FGuid& O) const {
        if (A != O.A) return A < O.A;
        if (B != O.B) return B < O.B;
        if (C != O.C) return C < O.C;
        return D < O.D;
    }
};
inline bool FGuid::Parse(const FString& GuidString, FGuid& OutGuid) {
    (void)GuidString; OutGuid = FGuid(); return true;
}


// Sentinel index for "not found" (UE parity)
constexpr int32 INDEX_NONE = -1;

// Root motion mode (UE parity)
namespace ERootMotionMode { enum Type { NoRootMotionExtraction, IgnoreRootMotion, RootMotionFromEverything, RootMotionFromMontagesOnly }; }

// End-play reason enum used by EndPlay overrides
namespace EEndPlayReason { enum Type { Destroyed, LevelTransition, EndPlayInEditor, RemovedFromWorld, Quit }; }

// Timer handle stub
struct FTimerHandle {
    int32 Handle = 0;
    bool IsValid() const { return Handle != 0; }
    void Invalidate() { Handle = 0; }
};

class FText {
public:
    FText() = default;
    static FText FromString(const FString& s) { return FText(); }
    FString ToString() const { return FString(); }
    bool IsEmpty() const { return true; }
};



// =============================================================================
// CONTAINER TYPES
// =============================================================================

template<typename T>
class TArray : public std::vector<T> {
public:
    using std::vector<T>::vector;
    
    int32 Num() const { return static_cast<int32>(this->size()); }
    bool IsEmpty() const { return this->empty(); }
    void Add(const T& Item) { this->push_back(Item); }
    void AddUnique(const T& Item) {
        if (std::find(this->begin(), this->end(), Item) == this->end()) {
            this->push_back(Item);
        }
    }
    void Empty() { this->clear(); }
    void Reset() { this->clear(); }
    template<typename Predicate>
    bool ContainsByPredicate(Predicate Pred) const {
        for (const T& Item : *this) { if (Pred(Item)) return true; }
        return false;
    }
    bool Contains(const T& Item) const {
        return std::find(this->begin(), this->end(), Item) != this->end();
    }
    int32 Find(const T& Item) const {
        auto it = std::find(this->begin(), this->end(), Item);
        return it != this->end() ? static_cast<int32>(it - this->begin()) : -1;
    }
    void RemoveAt(int32 Index) {
        if (Index >= 0 && Index < static_cast<int32>(this->size())) {
            this->erase(this->begin() + Index);
        }
    }
    void RemoveAt(int32 Index, int32 Count) {
        if (Index >= 0 && Count > 0 && Index + Count <= static_cast<int32>(this->size())) {
            this->erase(this->begin() + Index, this->begin() + Index + Count);
        }
    }
    T& Last() { return this->back(); }
    const T& Last() const { return this->back(); }
    void SetNum(int32 NewNum) { this->resize(NewNum); }
    void Reserve(int32 Number) { this->reserve(Number); }
    bool IsValidIndex(int32 Index) const { return Index >= 0 && Index < static_cast<int32>(this->size()); }
    T& operator[](int32 Index) { return std::vector<T>::operator[](Index); }
    const T& operator[](int32 Index) const { return std::vector<T>::operator[](Index); }
    
    template<typename Predicate>
    void Sort(Predicate Pred) { std::sort(this->begin(), this->end(), Pred); }
    void Sort() { std::sort(this->begin(), this->end()); }
    
    T* GetData() { return this->data(); }
    const T* GetData() const { return this->data(); }
    
    bool Remove(const T& Item) {
        auto it = std::find(this->begin(), this->end(), Item);
        if (it != this->end()) {
            this->erase(it);
            return true;
        }
        return false;
    }
    
    int32 RemoveAll(const T& Item) {
        auto newEnd = std::remove(this->begin(), this->end(), Item);
        int32 Count = static_cast<int32>(this->end() - newEnd);
        this->erase(newEnd, this->end());
        return Count;
    }
    
    // Predicate-based RemoveAll (UE parity)
    template<typename Predicate,
             typename = decltype(std::declval<Predicate>()(std::declval<const T&>()))>
    int32 RemoveAll(Predicate Pred) {
        auto newEnd = std::remove_if(this->begin(), this->end(), Pred);
        int32 Count = static_cast<int32>(this->end() - newEnd);
        this->erase(newEnd, this->end());
        return Count;
    }
    
    T Pop(bool /*bAllowShrinking*/ = true) {
        T Value = this->back();
        this->pop_back();
        return Value;
    }
    void Push(const T& Item) { this->push_back(Item); }
    void Insert(const T& Item, int32 Index) { this->insert(this->begin() + Index, Item); }
    
    void Append(const TArray<T>& Other) {
        this->insert(this->end(), Other.begin(), Other.end());
    }
    
    void Append(TArray<T>&& Other) {
        this->insert(this->end(), std::make_move_iterator(Other.begin()), std::make_move_iterator(Other.end()));
    }
};

// UE-style pair wrapper for TMap iteration
template<typename KeyType, typename ValueType>
struct TMapPair {
    const KeyType& Key;
    ValueType& Value;
    TMapPair(const KeyType& K, ValueType& V) : Key(K), Value(V) {}
};

template<typename KeyType, typename ValueType>
class TMap {
private:
    std::map<KeyType, ValueType> InternalMap;
public:
    TMap() = default;
    TMap(std::initializer_list<std::pair<const KeyType, ValueType>> InitList) : InternalMap(InitList) {}
    
    int32 Num() const { return static_cast<int32>(InternalMap.size()); }
    void Add(const KeyType& Key, const ValueType& Value) { InternalMap[Key] = Value; }
    bool Contains(const KeyType& Key) const { return InternalMap.find(Key) != InternalMap.end(); }
    ValueType* Find(const KeyType& Key) {
        auto it = InternalMap.find(Key);
        return it != InternalMap.end() ? &it->second : nullptr;
    }
    const ValueType* Find(const KeyType& Key) const {
        auto it = InternalMap.find(Key);
        return it != InternalMap.end() ? &it->second : nullptr;
    }
    void Empty() { InternalMap.clear(); }
    void Reset() { InternalMap.clear(); }
    bool Remove(const KeyType& Key) { return InternalMap.erase(Key) > 0; }
    void Reserve(int32) { /* std::map does not pre-allocate */ }
    
    ValueType& FindOrAdd(const KeyType& Key) { return InternalMap[Key]; }
    ValueType& FindOrAdd(const KeyType& Key, const ValueType& Default) {
        auto it = InternalMap.find(Key);
        if (it == InternalMap.end()) it = InternalMap.emplace(Key, Default).first;
        return it->second;
    }
    ValueType FindRef(const KeyType& Key) const {
        auto it = InternalMap.find(Key);
        return it != InternalMap.end() ? it->second : ValueType();
    }
    template<typename... Args>
    ValueType& Emplace(const KeyType& Key, Args&&... args) {
        return InternalMap[Key] = ValueType(std::forward<Args>(args)...);
    }
    void GetKeys(TArray<KeyType>& OutKeys) const {
        OutKeys.Empty();
        for (const auto& P : InternalMap) OutKeys.Add(P.first);
    }
    TArray<KeyType> GetKeysArray() const {
        TArray<KeyType> Keys;
        GetKeys(Keys);
        return Keys;
    }
    void GenerateValueArray(TArray<ValueType>& OutValues) const {
        OutValues.Empty();
        for (const auto& P : InternalMap) OutValues.Add(P.second);
    }
    
    ValueType& operator[](const KeyType& Key) { return InternalMap[Key]; }
    const ValueType& operator[](const KeyType& Key) const { return InternalMap.at(Key); }
    
    // Iterator support with Key/Value access
    class Iterator {
    public:
        using MapIterator = typename std::map<KeyType, ValueType>::iterator;
        MapIterator It;
        // Reference-semantics pair: `for (auto& Pair : Map)` binds to this cached
        // proxy, and writes to Pair.Value flow through into the map (UE parity).
        struct Pair {
            const KeyType& Key;
            ValueType& Value;
            Pair(const KeyType& K, ValueType& V) : Key(K), Value(V) {}
        };
        mutable std::unique_ptr<Pair> Current;
        Iterator(MapIterator it) : It(it) {}
        Iterator(const Iterator& Other) : It(Other.It) {}
        Iterator& operator=(const Iterator& Other) { It = Other.It; Current.reset(); return *this; }
        bool operator!=(const Iterator& Other) const { return It != Other.It; }
        Iterator& operator++() { ++It; Current.reset(); return *this; }
        Pair& operator*() const {
            Current = std::make_unique<Pair>(It->first, It->second);
            return *Current;
        }
    };
    
    class ConstIterator {
    public:
        using MapIterator = typename std::map<KeyType, ValueType>::const_iterator;
        MapIterator It;
        ConstIterator(MapIterator it) : It(it) {}
        bool operator!=(const ConstIterator& Other) const { return It != Other.It; }
        ConstIterator& operator++() { ++It; return *this; }
        struct Pair {
            const KeyType& Key;
            const ValueType& Value;
            Pair(const KeyType& K, const ValueType& V) : Key(K), Value(V) {}
        };
        Pair operator*() const { return Pair(It->first, It->second); }
    };
    
    Iterator begin() { return Iterator(InternalMap.begin()); }
    Iterator end() { return Iterator(InternalMap.end()); }
    ConstIterator begin() const { return ConstIterator(InternalMap.begin()); }
    ConstIterator end() const { return ConstIterator(InternalMap.end()); }
};

template<typename T>
class TSet : public std::set<T> {
public:
    using std::set<T>::set;
    
    int32 Num() const { return static_cast<int32>(this->size()); }
    void Add(const T& Item) { this->insert(Item); }
    bool Contains(const T& Item) const { return this->find(Item) != this->end(); }
    void Empty() { this->clear(); }
    bool Remove(const T& Item) { return this->erase(Item) > 0; }
};

// =============================================================================
// SMART POINTERS
// =============================================================================

// TSharedPtr — thin subclass of std::shared_ptr adding UE-parity members
// (ToSharedRef, IsValid). Constructible from shared_ptr for interop.
template<typename T>
class TSharedPtr : public std::shared_ptr<T> {
public:
    using std::shared_ptr<T>::shared_ptr;
    TSharedPtr() = default;
    TSharedPtr(const std::shared_ptr<T>& In) : std::shared_ptr<T>(In) {}
    TSharedPtr(std::shared_ptr<T>&& In) : std::shared_ptr<T>(std::move(In)) {}
    bool IsValid() const { return this->get() != nullptr; }
    std::shared_ptr<T> ToSharedRef() const { return *this; }
};

template<typename T>
using TWeakPtr = std::weak_ptr<T>;

template<typename T>
using TUniquePtr = std::unique_ptr<T>;

template<typename T, typename... Args>
TSharedPtr<T> MakeShared(Args&&... args) {
    return std::make_shared<T>(std::forward<Args>(args)...);
}

// TSharedRef — non-null shared pointer; approximated by shared_ptr in the shim
template<typename T>
using TSharedRef = std::shared_ptr<T>;

template<typename T>
TSharedPtr<T> MakeShareable(T* Ptr) {
    return TSharedPtr<T>(Ptr);
}

template<typename T, typename... Args>
TUniquePtr<T> MakeUnique(Args&&... args) {
    return std::make_unique<T>(std::forward<Args>(args)...);
}

// TFunction - UE wrapper for std::function
template<typename Signature>
using TFunction = std::function<Signature>;

// Cast function
template<typename To, typename From>
To* Cast(From* Src) {
    return dynamic_cast<To*>(Src);
}

template<typename To, typename From>
const To* Cast(const From* Src) {
    return dynamic_cast<const To*>(Src);
}

// =============================================================================
// MATH TYPES
// =============================================================================

struct FVector {
    float X = 0.0f, Y = 0.0f, Z = 0.0f;
    
    FVector() = default;
    FVector(float InX, float InY, float InZ) : X(InX), Y(InY), Z(InZ) {}
    
    FVector operator+(const FVector& V) const { return FVector(X + V.X, Y + V.Y, Z + V.Z); }
    FVector operator-(const FVector& V) const { return FVector(X - V.X, Y - V.Y, Z - V.Z); }
    FVector operator*(float Scale) const { return FVector(X * Scale, Y * Scale, Z * Scale); }
    FVector operator/(float Scale) const { return FVector(X / Scale, Y / Scale, Z / Scale); }
    FVector& operator+=(const FVector& V) { X += V.X; Y += V.Y; Z += V.Z; return *this; }
    FVector& operator-=(const FVector& V) { X -= V.X; Y -= V.Y; Z -= V.Z; return *this; }
    FVector& operator*=(float Scale) { X *= Scale; Y *= Scale; Z *= Scale; return *this; }
    FVector& operator/=(float Scale) { X /= Scale; Y /= Scale; Z /= Scale; return *this; }
    bool operator==(const FVector& V) const { return X == V.X && Y == V.Y && Z == V.Z; }
    bool operator!=(const FVector& V) const { return !(*this == V); }
    
    friend FVector operator*(float Scale, const FVector& V) { return FVector(V.X * Scale, V.Y * Scale, V.Z * Scale); }
    
    float Size() const { return std::sqrt(X*X + Y*Y + Z*Z); }
    struct FRotator Rotation() const;
    float SizeSquared() const { return X*X + Y*Y + Z*Z; }
    FVector operator-() const { return FVector(-X, -Y, -Z); }
    FVector GetSafeNormal() const {
        float S = Size();
        return S > 0.0001f ? (*this) / S : FVector();
    }
    
    static float DotProduct(const FVector& A, const FVector& B) {
        return A.X * B.X + A.Y * B.Y + A.Z * B.Z;
    }
    static FVector CrossProduct(const FVector& A, const FVector& B) {
        return FVector(
            A.Y * B.Z - A.Z * B.Y,
            A.Z * B.X - A.X * B.Z,
            A.X * B.Y - A.Y * B.X
        );
    }
    
    static float Dist(const FVector& A, const FVector& B) {
        return (A - B).Size();
    }
    static float DistSquared(const FVector& A, const FVector& B) {
        return (A - B).SizeSquared();
    }
    
    bool Normalize(float Tolerance = 0.0001f) {
        float S = Size();
        if (S > Tolerance) {
            X /= S; Y /= S; Z /= S;
            return true;
        }
        return false;
    }
    
    static const FVector ZeroVector;
    static const FVector OneVector;
    static const FVector UpVector;
    static const FVector ForwardVector;
    static const FVector RightVector;
};

inline const FVector FVector::ZeroVector(0.0f, 0.0f, 0.0f);
inline const FVector FVector::OneVector(1.0f, 1.0f, 1.0f);
inline const FVector FVector::UpVector(0.0f, 0.0f, 1.0f);
inline const FVector FVector::ForwardVector(1.0f, 0.0f, 0.0f);
inline const FVector FVector::RightVector(0.0f, 1.0f, 0.0f);

struct FVector2D {
    float X = 0.0f, Y = 0.0f;
    
    FVector2D() = default;
    FVector2D(float InX, float InY) : X(InX), Y(InY) {}
    
    FVector2D operator+(const FVector2D& V) const { return FVector2D(X + V.X, Y + V.Y); }
    FVector2D operator-(const FVector2D& V) const { return FVector2D(X - V.X, Y - V.Y); }
    FVector2D operator*(float Scale) const { return FVector2D(X * Scale, Y * Scale); }
    
    float Size() const { return std::sqrt(X*X + Y*Y); }
};

struct FRotator {
    float Pitch = 0.0f, Yaw = 0.0f, Roll = 0.0f;
    // (extended below)
    
    FRotator() = default;
    FRotator(float InPitch, float InYaw, float InRoll) : Pitch(InPitch), Yaw(InYaw), Roll(InRoll) {}
    
    FVector Vector() const;
    FRotator GetNormalized() const { return *this; }
    
    static const FRotator ZeroRotator;
};

inline const FRotator FRotator::ZeroRotator(0.0f, 0.0f, 0.0f);


inline FRotator FVector::Rotation() const {
    const float YawRad = std::atan2(Y, X);
    const float PitchRad = std::atan2(Z, std::sqrt(X*X + Y*Y));
    return FRotator(PitchRad * (180.0f / 3.14159265358979323846f),
                    YawRad * (180.0f / 3.14159265358979323846f), 0.0f);
}

struct FQuat {
    float X = 0.0f, Y = 0.0f, Z = 0.0f, W = 1.0f;
    FQuat() = default;
    FQuat(float InX, float InY, float InZ, float InW) : X(InX), Y(InY), Z(InZ), W(InW) {}
    explicit FQuat(const FRotator& R); // defined out-of-line below
    FQuat operator*(const FQuat& Q) const;
    FVector RotateVector(const FVector& V) const { return V; }
    
    static const FQuat Identity;
};

inline const FQuat FQuat::Identity(0.0f, 0.0f, 0.0f, 1.0f);

inline FQuat::FQuat(const FRotator& R) {
    const float DEG2RAD = 3.14159265358979323846f / 180.0f;
    const float CP = std::cos(R.Pitch * DEG2RAD * 0.5f), SP = std::sin(R.Pitch * DEG2RAD * 0.5f);
    const float CY = std::cos(R.Yaw   * DEG2RAD * 0.5f), SY = std::sin(R.Yaw   * DEG2RAD * 0.5f);
    const float CR = std::cos(R.Roll  * DEG2RAD * 0.5f), SR = std::sin(R.Roll  * DEG2RAD * 0.5f);
    X = CR * SP * SY - SR * CP * CY;
    Y = -CR * SP * CY - SR * CP * SY;
    Z = CR * CP * SY - SR * SP * CY;
    W = CR * CP * CY + SR * SP * SY;
}

struct FTransform {
    FVector Translation;
    FQuat Rotation;
    FVector Scale3D = FVector::OneVector;
    
    FTransform() = default;
    FTransform(const FRotator& InRotation, const FVector& InTranslation, const FVector& InScale3D = FVector::OneVector)
        : Translation(InTranslation), Scale3D(InScale3D) {}
    FTransform(const FQuat& InRotation, const FVector& InTranslation, const FVector& InScale3D = FVector::OneVector)
        : Translation(InTranslation), Rotation(InRotation), Scale3D(InScale3D) {}
    
    FVector GetLocation() const { return Translation; }
    FVector TransformPosition(const FVector& V) const { return Translation + V; }
    FVector InverseTransformPosition(const FVector& V) const { return V - Translation; }
    FRotator GetRotation() const { return FRotator(); }
    FVector GetScale3D() const { return Scale3D; }
    
    void SetLocation(const FVector& NewLocation) { Translation = NewLocation; }
    void SetRotation(const FQuat& NewRotation) { Rotation = NewRotation; }
    
    static const FTransform Identity;
};

inline const FTransform FTransform::Identity;

struct FLinearColor {
    float R = 0.0f, G = 0.0f, B = 0.0f, A = 1.0f;
    
    FLinearColor() = default;
    FLinearColor(float InR, float InG, float InB, float InA = 1.0f) : R(InR), G(InG), B(InB), A(InA) {}
    
    FLinearColor& operator+=(const FLinearColor& C) { R += C.R; G += C.G; B += C.B; A += C.A; return *this; }
    FLinearColor& operator*=(float S) { R *= S; G *= S; B *= S; A *= S; return *this; }
    
    static const FLinearColor White;
    static const FLinearColor Black;
    static const FLinearColor Red;
    static const FLinearColor Green;
    static const FLinearColor Blue;
    static const FLinearColor Yellow;
    static const FLinearColor Cyan;
    static const FLinearColor Magenta;
    static const FLinearColor Orange;
    static const FLinearColor Gray;
    
    // HSV construction (H,S,V as bytes 0-255, UE parity)
    static FLinearColor MakeFromHSV8(uint8 H, uint8 S, uint8 V) {
        const float Hue = H / 255.0f * 360.0f;
        const float Sat = S / 255.0f;
        const float Val = V / 255.0f;
        const float C = Val * Sat;
        const float Hp = Hue / 60.0f;
        const float Xc = C * (1.0f - std::abs(std::fmod(Hp, 2.0f) - 1.0f));
        float R1 = 0, G1 = 0, B1 = 0;
        if (Hp < 1)      { R1 = C;  G1 = Xc; }
        else if (Hp < 2) { R1 = Xc; G1 = C; }
        else if (Hp < 3) { G1 = C;  B1 = Xc; }
        else if (Hp < 4) { G1 = Xc; B1 = C; }
        else if (Hp < 5) { R1 = Xc; B1 = C; }
        else             { R1 = C;  B1 = Xc; }
        const float M = Val - C;
        return FLinearColor(R1 + M, G1 + M, B1 + M, 1.0f);
    }
    
    static FLinearColor LerpUsingHSV(const FLinearColor& A, const FLinearColor& B, float Alpha) {
        return FLinearColor(A.R + Alpha * (B.R - A.R), A.G + Alpha * (B.G - A.G),
                            A.B + Alpha * (B.B - A.B), A.A + Alpha * (B.A - A.A));
    }
    
    FLinearColor operator*(float Scale) const { return FLinearColor(R * Scale, G * Scale, B * Scale, A * Scale); }
    FLinearColor operator+(const FLinearColor& O) const { return FLinearColor(R + O.R, G + O.G, B + O.B, A + O.A); }
};

inline const FLinearColor FLinearColor::White(1.0f, 1.0f, 1.0f, 1.0f);
inline const FLinearColor FLinearColor::Black(0.0f, 0.0f, 0.0f, 1.0f);
inline const FLinearColor FLinearColor::Red(1.0f, 0.0f, 0.0f, 1.0f);
inline const FLinearColor FLinearColor::Green(0.0f, 1.0f, 0.0f, 1.0f);
inline const FLinearColor FLinearColor::Blue(0.0f, 0.0f, 1.0f, 1.0f);
inline const FLinearColor FLinearColor::Yellow(1.0f, 1.0f, 0.0f, 1.0f);
inline const FLinearColor FLinearColor::Cyan(0.0f, 1.0f, 1.0f, 1.0f);
inline const FLinearColor FLinearColor::Magenta(1.0f, 0.0f, 1.0f, 1.0f);
inline const FLinearColor FLinearColor::Orange(1.0f, 0.5f, 0.0f, 1.0f);
inline const FLinearColor FLinearColor::Gray(0.5f, 0.5f, 0.5f, 1.0f);

struct FColor {
    uint8 R = 0, G = 0, B = 0, A = 255;
    
    FColor() = default;
    FColor(uint8 InR, uint8 InG, uint8 InB, uint8 InA = 255) : R(InR), G(InG), B(InB), A(InA) {}
    
    static const FColor White;
    static const FColor Black;
    static const FColor Red;
    static const FColor Green;
    static const FColor Blue;
    static const FColor Yellow;
    static const FColor Cyan;
    static const FColor Magenta;
    static const FColor Orange;
};

inline const FColor FColor::White(255, 255, 255, 255);
inline const FColor FColor::Black(0, 0, 0, 255);
inline const FColor FColor::Red(255, 0, 0, 255);
inline const FColor FColor::Green(0, 255, 0, 255);
inline const FColor FColor::Blue(0, 0, 255, 255);
inline const FColor FColor::Yellow(255, 255, 0, 255);
inline const FColor FColor::Cyan(0, 255, 255, 255);
inline const FColor FColor::Magenta(255, 0, 255, 255);
inline const FColor FColor::Orange(255, 165, 0, 255);


// =============================================================================
// MATH UTILITIES
// =============================================================================

struct FMath {
    static float Abs(float A) { return std::abs(A); }
    static float Sin(float A) { return std::sin(A); }
    static float Cos(float A) { return std::cos(A); }
    static float Tan(float A) { return std::tan(A); }
    static float Asin(float A) { return std::asin(A); }
    static float Acos(float A) { return std::acos(A); }
    static float Atan(float A) { return std::atan(A); }
    static float Atan2(float Y, float X) { return std::atan2(Y, X); }
    static float Sqrt(float A) { return std::sqrt(A); }
    static float Pow(float A, float B) { return std::pow(A, B); }
    static float Exp(float A) { return std::exp(A); }
    static float Log(float A) { return std::log(A); }
    static float Log2(float A) { return std::log2(A); }
    
    static float Clamp(float X, float Min, float Max) { return std::clamp(X, Min, Max); }
    static int32 Clamp(int32 X, int32 Min, int32 Max) { return std::clamp(X, Min, Max); }
    
    static float Min(float A, float B) { return std::min(A, B); }
    static float Max(float A, float B) { return std::max(A, B); }
    static int32 Min(int32 A, int32 B) { return std::min(A, B); }
    static int32 Max(int32 A, int32 B) { return std::max(A, B); }
    
    static float Max3(float A, float B, float C) { return Max(Max(A, B), C); }
    static int32 Max3(int32 A, int32 B, int32 C) { return Max(Max(A, B), C); }
    static float Min3(float A, float B, float C) { return Min(Min(A, B), C); }
    static int32 Min3(int32 A, int32 B, int32 C) { return Min(Min(A, B), C); }
    
    static float Lerp(float A, float B, float Alpha) { return A + Alpha * (B - A); }
    static FVector Lerp(const FVector& A, const FVector& B, float Alpha) {
        return FVector(A.X + Alpha * (B.X - A.X), A.Y + Alpha * (B.Y - A.Y), A.Z + Alpha * (B.Z - A.Z));
    }
    static FRotator Lerp(const FRotator& A, const FRotator& B, float Alpha) {
        return FRotator(A.Pitch + Alpha * (B.Pitch - A.Pitch), A.Yaw + Alpha * (B.Yaw - A.Yaw), A.Roll + Alpha * (B.Roll - A.Roll));
    }
    static FLinearColor Lerp(const FLinearColor& A, const FLinearColor& B, float Alpha) {
        return FLinearColor(A.R + Alpha * (B.R - A.R), A.G + Alpha * (B.G - A.G),
                            A.B + Alpha * (B.B - A.B), A.A + Alpha * (B.A - A.A));
    }
    static FQuat Lerp(const FQuat& A, const FQuat& B, float Alpha) {
        // Normalized lerp (nlerp) approximation of slerp
        FQuat Q(A.X + Alpha * (B.X - A.X), A.Y + Alpha * (B.Y - A.Y),
                A.Z + Alpha * (B.Z - A.Z), A.W + Alpha * (B.W - A.W));
        const float M = std::sqrt(Q.X*Q.X + Q.Y*Q.Y + Q.Z*Q.Z + Q.W*Q.W);
        if (M > 1e-8f) { Q.X /= M; Q.Y /= M; Q.Z /= M; Q.W /= M; }
        return Q;
    }
    static float FInterpTo(float Current, float Target, float DeltaTime, float InterpSpeed) {
        if (InterpSpeed <= 0.0f) return Target;
        float Dist = Target - Current;
        if (Dist * Dist < 0.00001f) return Target;
        return Current + Dist * Clamp(DeltaTime * InterpSpeed, 0.0f, 1.0f);
    }
    
    static int32 RandRange(int32 Min, int32 Max) { return Min + (rand() % (Max - Min + 1)); }
    static float FRandRange(float Min, float Max) { return Min + static_cast<float>(rand()) / RAND_MAX * (Max - Min); }
    static float FRand() { return static_cast<float>(rand()) / RAND_MAX; }
    
    static bool IsNearlyZero(float Value, float Tolerance = 0.0001f) { return Abs(Value) <= Tolerance; }
    static bool IsNearlyEqual(float A, float B, float Tolerance = 0.0001f) { return Abs(A - B) <= Tolerance; }
    
    static constexpr float PI = 3.14159265358979323846f;
    static constexpr float HALF_PI = PI / 2.0f;
    static constexpr float TWO_PI = PI * 2.0f;
    
    static float DegreesToRadians(float Degrees) { return Degrees * (PI / 180.0f); }
    static float RadiansToDegrees(float Radians) { return Radians * (180.0f / PI); }
    
    static int32 FloorToInt(float F) { return static_cast<int32>(std::floor(F)); }
    static int32 CeilToInt(float F) { return static_cast<int32>(std::ceil(F)); }
    static int32 RoundToInt(float F) { return static_cast<int32>(std::round(F)); }
    
    static float Tanh(float A) { return std::tanh(A); }
    static float Sinh(float A) { return std::sinh(A); }
    static float Cosh(float A) { return std::cosh(A); }
    
    static float GetMappedRangeValueClamped(const FVector2D& InputRange, const FVector2D& OutputRange, float Value) {
        const float Pct = GetRangePct(InputRange.X, InputRange.Y, Value);
        const float ClampedPct = Clamp(Pct, 0.0f, 1.0f);
        return OutputRange.X + ClampedPct * (OutputRange.Y - OutputRange.X);
    }
    static float Frac(float Value) { return Value - std::floor(Value); }
    static float Fmod(float X, float Y) { return std::fmod(X, Y); }
    static float GetRangePct(float MinValue, float MaxValue, float Value) {
        const float Divisor = MaxValue - MinValue;
        if (IsNearlyZero(Divisor)) return Value >= MaxValue ? 1.0f : 0.0f;
        return (Value - MinValue) / Divisor;
    }
    static float SmoothStep(float A, float B, float X) {
        if (X < A) return 0.0f;
        if (X >= B) return 1.0f;
        const float T = Clamp((X - A) / (B - A), 0.0f, 1.0f);
        return T * T * (3.0f - 2.0f * T);
    }
    
    static float Square(float A) { return A * A; }
    static int32 Square(int32 A) { return A * A; }
    
    static float Loge(float A) { return std::log(A); }
    static float Log10(float A) { return std::log10(A); }
};

// =============================================================================
// UNREAL OBJECT SYSTEM STUBS
// =============================================================================

// Forward declarations for tick types
enum ELevelTick : int;
struct FActorComponentTickFunction;

class UObject {
public:
    virtual ~UObject() = default;
    
    FName GetFName() const { return FName(); }
    FString GetName() const { return FString(); }
    class UClass* GetClass() const { return nullptr; }
    
    bool IsA(class UClass* SomeClass) const { return false; }
    
    template<typename T>
    bool IsA() const { return false; }
    
    // Lifecycle methods (for Super:: calls)
    virtual void BeginPlay() {}
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, struct FActorComponentTickFunction* ThisTickFunction) {}
};

class UClass : public UObject {
public:
    FName GetFName() const { return FName(); }
};

// NewObject / CreateDefaultSubobject stubs.
// NOTE (standalone shim): objects are heap-allocated and intentionally leaked —
// UE's garbage collector owns UObjects in production; the shim mirrors that
// ownership model so calling code needs no #ifdef changes.
template<typename T>
T* NewObject(UObject* /*Outer*/ = nullptr) { return new T(); }

template<typename T>
T* NewObject(UObject* /*Outer*/, UClass* /*Class*/) { return new T(); }

template<typename T>
T* CreateDefaultSubobject(const FName& /*SubobjectName*/) { return new T(); }

// Material interface stub
class UMaterialInterface : public UObject {
public:
    FString GetName() const { return FString(); }
};

// Collision/trace support stubs
class AActor;

struct FHitResult {
    bool bBlockingHit = false;
    FVector Location;
    FVector ImpactPoint;
    FVector ImpactNormal;
    FVector Normal;
    float Distance = 0.0f;
    AActor* HitActor = nullptr;
    AActor* GetActor() const { return HitActor; }
};

struct FCollisionQueryParams {
    bool bTraceComplex = false;
    FCollisionQueryParams() = default;
    FCollisionQueryParams(FName /*TraceTag*/, bool bInTraceComplex = false, const UObject* /*IgnoreActor*/ = nullptr)
        : bTraceComplex(bInTraceComplex) {}
    void AddIgnoredActor(const AActor*) {}
};

enum ECollisionChannel : int {
    ECC_WorldStatic, ECC_WorldDynamic, ECC_Pawn, ECC_Visibility,
    ECC_Camera, ECC_PhysicsBody, ECC_Vehicle, ECC_Destructible
};

class AActor : public UObject {
public:
    virtual void BeginPlay() {}
    virtual void Tick(float DeltaTime) {}
    virtual void EndPlay(EEndPlayReason::Type EndPlayReason) { (void)EndPlayReason; }
    
    FVector GetActorLocation() const { return FVector(); }
    FRotator GetActorRotation() const { return FRotator(); }
    FTransform GetActorTransform() const { return FTransform(); }
    FVector GetVelocity() const { return FVector(); }
    void SetActorLocation(const FVector& NewLocation) {}
    void SetActorRotation(const FRotator& NewRotation) {}
    
    FVector GetActorForwardVector() const { return FVector::ForwardVector; }
    FVector GetActorRightVector() const { return FVector::RightVector; }
    FVector GetActorUpVector() const { return FVector::UpVector; }
    
    class UWorld* GetWorld() const { return nullptr; }
    
    template<typename T>
    T* FindComponentByClass() const { return nullptr; }
    
    template<typename T>
    TArray<T*> GetComponentsByClass() const { return TArray<T*>(); }
};

// Ticking group constants (ETickingGroup)
enum ETickingGroup : int { TG_PrePhysics, TG_StartPhysics, TG_DuringPhysics, TG_EndPhysics, TG_PostPhysics, TG_PostUpdateWork, TG_LastDemotable, TG_NewlySpawned };

// Tick types definition
enum ELevelTick : int { LEVELTICK_TimeOnly, LEVELTICK_ViewportsOnly, LEVELTICK_All, LEVELTICK_PauseTick };
struct FActorComponentTickFunction { bool bCanEverTick = false; };

// Forward declarations for cognitive state types
struct FDeepTreeEchoCognitiveState {
    float Arousal = 0.5f;
    float Valence = 0.5f;
    float Dominance = 0.5f;
    float Attention = 0.5f;
    float Focus = 0.5f;
    float WisdomScore = 0.0f;
    TMap<FString, float> StateValues;
};

struct FEmotionalState {
    float Joy = 0.0f;
    float Sadness = 0.0f;
    float Anger = 0.0f;
    float Fear = 0.0f;
    float Surprise = 0.0f;
    float Disgust = 0.0f;
    float Trust = 0.0f;
    float Anticipation = 0.0f;
    float Arousal = 0.5f;
    float Valence = 0.5f;
};

struct FCognitiveState {
    float Attention = 0.5f;
    float Arousal = 0.5f;
    float Valence = 0.5f;
    float Focus = 0.5f;
    float Dominance = 0.5f;
    float Creativity = 0.5f;
    float WisdomScore = 0.0f;
    float ExecutiveFunction = 0.5f;
    float MotorReadiness = 0.5f;
    float EmotionalResonance = 0.5f;
    float SensoryIntegration = 0.5f;
    float ProprioceptiveAwareness = 0.5f;
    float ProcessingLoad = 0.0f;
    TMap<FString, float> StateValues;
};

// Note: Super is typically defined per-class in UE via GENERATED_BODY()
// For standalone compilation, we'll handle Super calls differently
// The actual source files need to use explicit parent class names

class UActorComponent : public UObject {
public:
    using Super = UActorComponent;  // For standalone compilation (self-reference for base)
    struct FTickFunction {
        bool bCanEverTick = false;
        bool bStartWithTickEnabled = true;
        float TickInterval = 0.0f;
        int TickGroup = 0; // ETickingGroup
    } PrimaryComponentTick;
    
    virtual void BeginPlay() {}
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) {}
    virtual void EndPlay(EEndPlayReason::Type EndPlayReason) { (void)EndPlayReason; }
    
    AActor* GetOwner() const { return nullptr; }
    void SetComponentTickEnabled(bool bEnabled) {}
    bool IsComponentTickEnabled() const { return PrimaryComponentTick.bCanEverTick; }
    UWorld* GetWorld() const { return nullptr; }
};

class USceneComponent : public UActorComponent {
public:
    FVector GetComponentLocation() const { return FVector(); }
    FRotator GetComponentRotation() const { return FRotator(); }
    void SetWorldLocation(const FVector& NewLocation) {}
    void SetWorldRotation(const FRotator& NewRotation) {}
};

class APawn : public AActor {
public:
    class AController* GetController() const { return nullptr; }
};

class UCharacterMovementComponent;

class ACharacter : public APawn {
public:
    UCharacterMovementComponent* GetCharacterMovement() const { return nullptr; }
    class USkeletalMeshComponent* GetMesh() const { return nullptr; }
    FVector GetVelocity() const { return FVector(); }
    void Jump() {}
    void StopJumping() {}
    bool CanJump() const { return true; }
};

class AController : public AActor {
public:
    APawn* GetPawn() const { return nullptr; }
};

class APlayerController : public AController {
public:
};

class FTimerManager {
public:
    template<typename... Args>
    void SetTimer(Args&&...) {}
    template<typename... Args>
    void ClearTimer(Args&&...) {}
    bool IsTimerActive(const struct FTimerHandle&) const { return false; }
};

class UWorld : public UObject {
public:
    FTimerManager& GetTimerManager() { static FTimerManager TM; return TM; }
    bool LineTraceSingleByChannel(struct FHitResult& OutHit, const FVector& Start, const FVector& End,
                                  int TraceChannel, const struct FCollisionQueryParams& Params = {}) {
        (void)OutHit; (void)Start; (void)End; (void)TraceChannel; (void)Params;
        return false;
    }
    float GetTimeSeconds() const { return 0.0f; }
    float GetDeltaSeconds() const { return 0.0f; }
};

// =============================================================================
// DELEGATES
// =============================================================================

template<typename... ParamTypes>
class TMulticastDelegate {
public:
    void Broadcast(ParamTypes... Params) {}
    void AddLambda(std::function<void(ParamTypes...)> Lambda) {}
    void AddUObject(UObject* Object, void (UObject::*Method)(ParamTypes...)) {}
    // AddDynamic/RemoveDynamic are UE macros resolving to __Internal_AddDynamic;
    // for the standalone shim we accept any object/method pair and discard it.
    template<typename UserClass, typename MethodType>
    void AddDynamic(UserClass* Object, MethodType Method) { (void)Object; (void)Method; }
    template<typename UserClass, typename MethodType>
    void RemoveDynamic(UserClass* Object, MethodType Method) { (void)Object; (void)Method; }
    template<typename UserClass, typename MethodType>
    void AddUniqueDynamic(UserClass* Object, MethodType Method) { (void)Object; (void)Method; }
    void RemoveAll(UObject* Object) {}
    void Clear() {}
    bool IsBound() const { return false; }
};

template<typename RetType, typename... ParamTypes>
class TDelegate {
public:
    RetType Execute(ParamTypes... Params) { return RetType(); }
    void BindLambda(std::function<RetType(ParamTypes...)> Lambda) {}
    bool IsBound() const { return false; }
};

#define DECLARE_MULTICAST_DELEGATE(DelegateName) \
    using DelegateName = TMulticastDelegate<>

#define DECLARE_MULTICAST_DELEGATE_OneParam(DelegateName, Param1Type) \
    using DelegateName = TMulticastDelegate<Param1Type>

#define DECLARE_MULTICAST_DELEGATE_TwoParams(DelegateName, Param1Type, Param2Type) \
    using DelegateName = TMulticastDelegate<Param1Type, Param2Type>

#define DECLARE_DELEGATE(DelegateName) \
    using DelegateName = TDelegate<void>

#define DECLARE_DELEGATE_RetVal(RetType, DelegateName) \
    using DelegateName = TDelegate<RetType>

// =============================================================================
// LOGGING
// =============================================================================

#define UE_LOG(Category, Verbosity, Format, ...) \
    do { /* stub */ } while(0)

#define DEFINE_LOG_CATEGORY(CategoryName) \
    struct CategoryName##_Stub {}

#define DEFINE_LOG_CATEGORY_STATIC(CategoryName, DefaultVerbosity, CompileTimeVerbosity) \
    struct CategoryName##_Stub {}

#define DECLARE_LOG_CATEGORY_EXTERN(CategoryName, DefaultVerbosity, CompileTimeVerbosity) \
    struct CategoryName##_Stub {}

// Log categories
struct LogTemp_Stub {};
#define LogTemp LogTemp_Stub()

// Verbosity levels
enum class ELogVerbosity { NoLogging, Fatal, Error, Warning, Display, Log, Verbose, VeryVerbose };

// =============================================================================
// ASSERTIONS
// =============================================================================

#define check(expr) do { (void)(expr); } while(0)
#define checkf(expr, format, ...) do { (void)(expr); } while(0)
#define ensure(expr) (expr)
#define ensureMsgf(expr, format, ...) (expr)
#define verify(expr) (expr)
#define verifyf(expr, format, ...) (expr)

// =============================================================================
// REFLECTION MACROS (Stubs)
// =============================================================================

#define GENERATED_BODY()
#define GENERATED_UCLASS_BODY()
#define GENERATED_USTRUCT_BODY()

#define UCLASS(...)
#define USTRUCT(...)
#define UENUM(...)
#define UPROPERTY(...)
#define UFUNCTION(...)
#define UMETA(...)

#define UINTERFACE(...)
#define GENERATED_IINTERFACE_BODY()

// =============================================================================
// MISCELLANEOUS
// =============================================================================

#define TEXT(x) x
#define TEXTVIEW(x) x

#define FORCEINLINE inline
#define FORCENOINLINE

#define WITH_EDITOR 0
#define WITH_EDITORONLY_DATA 0

// API export macros
#ifndef WITH_ENGINE_REFLECTION
#define WITH_ENGINE_REFLECTION 0
#endif
#define DEEPTREEECHO_API
#define UNREALECHO_API
#define CORE_API
#define ENGINE_API

// Pair type
template<typename KeyType, typename ValueType>
struct TPair {
    KeyType Key;
    ValueType Value;
    TPair() = default;
    TPair(const KeyType& InKey, const ValueType& InValue) : Key(InKey), Value(InValue) {}
};

template<typename KeyType, typename ValueType>
TPair<KeyType, ValueType> MakeTuple(const KeyType& Key, const ValueType& Value) {
    return TPair<KeyType, ValueType>(Key, Value);
}

// Dynamic delegate macros (stubs)
#define DECLARE_DYNAMIC_MULTICAST_DELEGATE(DelegateName) \
    using DelegateName = TMulticastDelegate<>

#define DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(DelegateName, Param1Type, Param1Name) \
    using DelegateName = TMulticastDelegate<Param1Type>

#define DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(DelegateName, Param1Type, Param1Name, Param2Type, Param2Name) \
    using DelegateName = TMulticastDelegate<Param1Type, Param2Type>

#define DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(DelegateName, P1T, P1N, P2T, P2N, P3T, P3N) \
    using DelegateName = TMulticastDelegate<P1T, P2T, P3T>

// GC functions
inline void CollectGarbage(int Flags) {}

// Time
struct FDateTime {
    static FDateTime Now() { return FDateTime(); }
    static FDateTime UtcNow() { return FDateTime(); }
    FString ToString() const { return FString(); }
};

struct FTimespan {
    static FTimespan FromSeconds(double Seconds) { return FTimespan(); }
    static FTimespan FromMilliseconds(double Milliseconds) { return FTimespan(); }
    double GetTotalSeconds() const { return 0.0; }
    double GetTotalMilliseconds() const { return 0.0; }
};

// Platform time
struct FPlatformTime {
    static double Seconds() { 
        return std::chrono::duration<double>(
            std::chrono::steady_clock::now().time_since_epoch()
        ).count();
    }
    static uint64 Cycles64() { return 0; }
};

// Async
namespace EAsyncExecution {
    enum Type { TaskGraph, Thread, ThreadPool };
}

template<typename ResultType>
class TFuture {
public:
    ResultType Get() { return ResultType(); }
    bool IsReady() const { return true; }
    void Wait() {}
};

template<typename ResultType, typename... Args>
TFuture<ResultType> Async(int ExecutionType, std::function<ResultType()> Function) {
    return TFuture<ResultType>();
}

// JSON
class FJsonObject {
public:
    /** Field map (UE parity: TMap<FString, TSharedPtr<FJsonValue>>) */
    TMap<FString, TSharedPtr<class FJsonValue>> Values;

    int32 GetIntegerField(const FString& FieldName) const { return 0; }
    bool TryGetObjectField(const FString& FieldName, const TSharedPtr<FJsonObject>*& OutObject) const { OutObject = nullptr; return false; }
    bool TryGetStringField(const FString& FieldName, FString& OutString) const { return false; }
    bool TryGetNumberField(const FString& FieldName, double& OutNumber) const { return false; }
    TSharedPtr<FJsonObject> GetObjectField(const FString& FieldName) const { return nullptr; }
    TArray<TSharedPtr<class FJsonValue>> GetArrayField(const FString& FieldName) const { return {}; }
    FString GetStringField(const FString& FieldName) const { return FString(); }
    double GetNumberField(const FString& FieldName) const { return 0.0; }
    bool GetBoolField(const FString& FieldName) const { return false; }
    bool HasField(const FString& FieldName) const { return false; }
    
    void SetObjectField(const FString& FieldName, TSharedPtr<FJsonObject> Value) {}
    void SetArrayField(const FString& FieldName, const TArray<TSharedPtr<class FJsonValue>>& Value) {}
    void SetStringField(const FString& FieldName, const FString& Value) {}
    void SetNumberField(const FString& FieldName, double Value) {}
    void SetBoolField(const FString& FieldName, bool Value) {}
};

class FJsonValue {
public:
    TSharedPtr<FJsonObject> AsObject() const { return nullptr; }
    TArray<TSharedPtr<FJsonValue>> AsArray() const { return {}; }
    FString AsString() const { return FString(); }
    double AsNumber() const { return 0.0; }
    bool AsBool() const { return false; }
};

// JSON serialization (parity stubs)
template<typename CharType = TCHAR>
class TJsonWriter {
public:
    void Close() {}
};

template<typename CharType = TCHAR>
class TJsonWriterFactory {
public:
    static TSharedRef<TJsonWriter<CharType>> Create(FString* OutString) {
        (void)OutString;
        return MakeShared<TJsonWriter<CharType>>();
    }
};

template<typename CharType = TCHAR>
class TJsonReader {
public:
};

template<typename CharType = TCHAR>
class TJsonReaderFactory {
public:
    static TSharedRef<TJsonReader<CharType>> Create(const FString& InString) {
        (void)InString;
        return MakeShared<TJsonReader<CharType>>();
    }
};

struct FJsonSerializer {
    template<typename CharType>
    static bool Serialize(TSharedRef<FJsonObject> Object, TSharedRef<TJsonWriter<CharType>> Writer) { return true; }
    template<typename CharType>
    static bool Deserialize(TSharedRef<TJsonReader<CharType>> Reader, TSharedPtr<FJsonObject>& OutObject) {
        OutObject = MakeShared<FJsonObject>();
        return true;
    }
};

// File utilities
struct FFileHelper {
    static bool SaveStringToFile(const FString& String, const TCHAR* Filename) { return true; }
    static bool LoadFileToString(FString& Result, const TCHAR* Filename) { return false; }
};

struct FPaths {
    static FString ProjectDir() { return FString(); }
    static FString ProjectContentDir() { return FString(); }
    static FString ProjectSavedDir() { return FString(); }
    static FString ProjectLogDir() { return FString(); }
    static FString Combine(const FString& A, const FString& B) { return A + "/" + B; }
};

// Module interface
class IModuleInterface {
public:
    virtual ~IModuleInterface() = default;
    virtual void StartupModule() {}
    virtual void ShutdownModule() {}
};

#define IMPLEMENT_MODULE(ModuleClass, ModuleName) \
    static ModuleClass ModuleName##_Instance;

#endif // COREMINIMAL_STUB_H
