# Modern OpenCog

A reimagined implementation of OpenCog's cognitive architecture using modern C++23 techniques.

## Overview

This project takes the core concepts from the original OpenCog project (circa 2012-2017) and reimplements them using contemporary C++ best practices:

- **Data-Oriented Design**: Structure of Arrays (SoA) for cache efficiency
- **Lock-Free Concurrency**: Atomic operations and wait-free algorithms
- **C++20/23 Features**: Concepts, coroutines, ranges, modules
- **SIMD Optimization**: Vectorized PLN formula computation
- **Compact Memory Layout**: Cache-line aligned structures

## Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    Modern OpenCog Core                       │
├─────────────────────────────────────────────────────────────┤
│  Core Types        │  Compact 8-byte TruthValue, AtomId     │
│  Memory            │  Pool allocators, SIMD-aligned vectors │
├─────────────────────────────────────────────────────────────┤
│  AtomSpace         │  SoA atom storage, O(1) hash lookup    │
│  Index             │  Type index, target-type index         │
├─────────────────────────────────────────────────────────────┤
│  Attention (ECAN)  │  Lock-free spreading activation        │
├─────────────────────────────────────────────────────────────┤
│  Pattern Matcher   │  Coroutine-based lazy evaluation       │
├─────────────────────────────────────────────────────────────┤
│  PLN               │  SIMD batch formulas, inference engine │
├─────────────────────────────────────────────────────────────┤
│  URE               │  Forward/backward chaining             │
└─────────────────────────────────────────────────────────────┘
```

## Building

```bash
# Configure
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build

# Run tests
cd build && ctest

# Run benchmarks
./build/benchmarks/benchmarks
```

### Requirements

- C++23 compliant compiler (GCC 13+, Clang 16+, MSVC 2022 17.6+)
- CMake 3.20+

### Optional Dependencies

- mimalloc: High-performance allocator (`-DOPENCOG_USE_MIMALLOC=ON`)

## Key Improvements Over Original

| Component | Original (~2014) | Modern (2024) |
|-----------|------------------|---------------|
| Atom size | ~200+ bytes | 64 bytes |
| TruthValue | 24+ bytes | 8 bytes |
| Concurrency | Mutex per atom | Lock-free atomics |
| Pattern match | Eager, blocking | Lazy coroutines |
| PLN compute | Sequential | SIMD batch |
| Lookup | std::unordered_map | flat_map |

## Usage Example

```cpp
#include <opencog/atomspace/atomspace.hpp>
#include <opencog/pln/inference.hpp>

using namespace opencog;

int main() {
    AtomSpace space;

    // Create knowledge
    auto cat = space.add_node(AtomType::CONCEPT_NODE, "Cat");
    auto animal = space.add_node(AtomType::CONCEPT_NODE, "Animal");
    auto mammal = space.add_node(AtomType::CONCEPT_NODE, "Mammal");

    space.add_link(AtomType::INHERITANCE_LINK, {cat, mammal},
                   TruthValue{0.95f, 0.9f});
    space.add_link(AtomType::INHERITANCE_LINK, {mammal, animal},
                   TruthValue{0.99f, 0.95f});

    // Query with pattern matching
    auto results = Query(space)
        .variable("X")
        .match(AtomType::INHERITANCE_LINK, {var("X"), ground(animal.id())})
        .collect();

    // Run PLN inference
    pln::PLNEngine engine(space);
    engine.add_rules(pln::rules::get_standard_rules());
    auto conclusions = engine.forward_chain(cat);

    return 0;
}
```

## Components

### Core (`include/opencog/core/`)
- `types.hpp`: AtomId, AtomType, TruthValue, AttentionValue
- `memory.hpp`: Pool allocators, arena allocator, SIMD vectors

### AtomSpace (`include/opencog/atomspace/`)
- `atom_table.hpp`: SoA atom storage
- `index.hpp`: Type and target-type indices
- `atomspace.hpp`: High-level AtomSpace API

### Attention (`include/opencog/attention/`)
- `attention_bank.hpp`: ECAN implementation

### Pattern Matching (`include/opencog/pattern/`)
- `pattern.hpp`: Pattern definitions
- `generator.hpp`: C++20 coroutine generator
- `matcher.hpp`: Pattern matcher

### PLN (`include/opencog/pln/`)
- `formulas.hpp`: PLN formulas with SIMD
- `inference.hpp`: Forward/backward chaining

### URE (`include/opencog/ure/`)
- `rule.hpp`: Rule definitions
- `engine.hpp`: Unified Rule Engine

## License

MIT License

## Acknowledgments

Based on concepts from the [OpenCog](https://github.com/opencog) project.
