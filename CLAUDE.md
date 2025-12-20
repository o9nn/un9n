# CLAUDE.md - Deep Tree Echo Cognitive Framework

## Project Overview

**un9n** is a unified cognitive architecture for embodied AI avatars that integrates:
- **Deep Tree Echo** - Hierarchical cognitive architecture with 12-step processing cycles
- **ReservoirCpp** - Echo State Networks for temporal pattern recognition
- **4E Embodied Cognition** - Embodied, Embedded, Enacted, Extended cognition principles
- **Unreal Engine Integration** - Real-time avatar and environment systems

## Repository Structure

```
un9n/
├── DeepTreeEcho/           # Core cognitive architecture (C++)
│   ├── Core/               # DeepTreeEchoCore - central orchestrator
│   ├── Reservoir/          # Echo State Network integration
│   ├── 4ECognition/        # 4E embodied cognition implementation
│   ├── Avatar/             # Avatar evolution system
│   ├── ActiveInference/    # Predictive processing
│   ├── Entelechy/          # Goal-directed behavior
│   ├── Metamodel/          # Meta-cognitive systems
│   ├── System5/            # Viable system model
│   └── Wisdom/             # Higher-order cognition
│
├── ReservoirEcho/          # Echo State Network library (~1,531 C++ files)
│   ├── reservoircpp_cpp/   # Core ESN implementation
│   │   ├── include/        # Headers
│   │   └── src/            # Implementation
│   └── external/           # Dependencies (Eigen 3.4.0)
│
└── UnrealEcho/             # Unreal Engine cognitive components (106 C++ files)
    ├── Animation/          # Animation systems
    ├── Audio/              # Audio processing
    ├── Avatar/             # Avatar components
    ├── Character/          # Character systems
    ├── Cognitive/          # Cognitive processing
    ├── Consciousness/      # Consciousness streams
    ├── DeepTreeEchoAvatar/ # DTE integration
    ├── Environment/        # Environmental interaction
    ├── Interaction/        # Interaction systems
    ├── Narrative/          # Narrative systems
    ├── NeuralNetwork/      # Neural network components
    ├── Neurochemical/      # Neurochemical simulation
    ├── Patterns/           # Pattern recognition
    ├── Personality/        # Personality modeling
    └── Rendering/          # Rendering pipeline
```

## Key Architecture Concepts

### 12-Step Cognitive Cycle
The system operates on a 12-step cognitive loop with 3 concurrent consciousness streams phased 120° apart:
- **Stream 1**: Perceiving (steps 1, 4, 7, 10)
- **Stream 2**: Acting (steps 2, 5, 8, 11)
- **Stream 3**: Reflecting (steps 3, 6, 9, 12)

Triadic synchronization points occur at {1,5,9}, {2,6,10}, {3,7,11}, {4,8,12}.

### Nested Shell Structure (OEIS A000081)
Execution contexts follow rooted tree enumeration:
- 1 nest → 1 term (global context)
- 2 nests → 2 terms (local context)
- 3 nests → 4 terms (detailed processing)
- 4 nests → 9 terms (fine-grained)

### 4E Embodied Cognition
1. **Embodied** - Body schema, proprioception, somatic markers
2. **Embedded** - Environmental affordances, niche coupling
3. **Enacted** - Sensorimotor contingencies, action-perception loops
4. **Extended** - Cognitive tools, external memory

### Reservoir Computing
- Hierarchical Echo State Networks with intrinsic plasticity
- Configurable spectral radius (default 0.9)
- Leak rate for temporal dynamics (default 0.3)
- Cross-reservoir coupling for echo propagation

## Core Components

| Component | Location | Purpose |
|-----------|----------|---------|
| `UDeepTreeEchoCore` | `DeepTreeEcho/Core/` | Central orchestrator for cognitive loop |
| `UDeepTreeEchoReservoir` | `DeepTreeEcho/Reservoir/` | ESN integration with hierarchical processing |
| `UEmbodiedCognitionComponent` | `DeepTreeEcho/4ECognition/` | 4E cognition state management |
| `UAvatarEvolutionSystem` | `DeepTreeEcho/Avatar/` | Ontogenetic development stages |

## Development Notes

### Language & Framework
- **C++** with Unreal Engine 5.x conventions
- Uses `UCLASS`, `USTRUCT`, `UPROPERTY`, `UFUNCTION` macros
- BlueprintCallable/BlueprintReadWrite for editor integration
- Generated headers (`.generated.h`) for reflection

### Key Patterns
- Actor components for modular cognitive subsystems
- State structs (`F*State`) for data containers
- Configuration via `EditAnywhere` properties
- Tick-based processing in `TickComponent`

### Dependencies
- **Unreal Engine 5.x** - Core engine
- **Eigen 3.4.0** - Linear algebra (in `ReservoirEcho/external/`)
- **ReservoirCpp** - Echo State Networks (integrated)

## Current Status

| Feature | Status |
|---------|--------|
| Reservoir Computing | Complete |
| 3 Concurrent Streams | Complete |
| 12-Step Cycle | Complete |
| 4E Cognition | Complete |
| Avatar Evolution | Complete |
| Memory Integration | In Progress |
| OpenCog Integration | Planned |

## Documentation

- `README.md` - Project overview and structure
- `DOCUMENTATION_INDEX.md` - Complete documentation guide
- `VERIFICATION_REPORT.md` - System verification report
- `ECHO_INTEGRATION_STATUS.md` - Echo-goals completion status
- `RESERVOIRCPP_INTEGRATION_GUIDE.md` - Technical integration guide
- `INTEGRATION_SUMMARY.md` - Executive summary

## Common Tasks

### Adding a New Cognitive Component
1. Create header in appropriate `DeepTreeEcho/` subdirectory
2. Inherit from `UActorComponent`
3. Define state struct with `USTRUCT(BlueprintType)`
4. Implement in corresponding `.cpp` file
5. Register with `DeepTreeEchoCore` for tick updates

### Working with Reservoirs
- Initialize via `UDeepTreeEchoReservoir::InitializeReservoirs()`
- Process input with `ProcessInput(data, streamID)`
- Detect patterns with `DetectTemporalPatterns()`
- Synchronize at triadic points with `SynchronizeStreams()`

### 4E Cognition Updates
- Update embodied state: proprioceptive/interoceptive data
- Update embedded state: affordances and environmental niche
- Update enacted state: sensorimotor contingencies
- Update extended state: external memory and tools
