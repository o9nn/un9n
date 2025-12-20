# un9n - Deep Tree Echo Cognitive Framework

**Unified Cognitive Architecture for Embodied AI Avatars**

This repository integrates the Deep Tree Echo cognitive architecture with ReservoirCpp (Echo State Networks) and 4E Embodied Cognition principles, creating a comprehensive framework for developing self-aware, emotionally resonant AI characters.

## Repository Structure

```
un9n/
├── DeepTreeEcho/                # Custom Deep Tree Echo cognitive components
│   ├── Core/                    # Central cognitive orchestrator
│   │   ├── DeepTreeEchoCore.h
│   │   └── DeepTreeEchoCore.cpp
│   │
│   ├── Reservoir/               # Echo State Network integration
│   │   ├── DeepTreeEchoReservoir.h
│   │   └── DeepTreeEchoReservoir.cpp
│   │
│   ├── 4ECognition/             # 4E Embodied cognition implementation
│   │   ├── EmbodiedCognitionComponent.h
│   │   └── EmbodiedCognitionComponent.cpp
│   │
│   └── Avatar/                  # Avatar evolution system
│       ├── AvatarEvolutionSystem.h
│       └── AvatarEvolutionSystem.cpp
│
├── ReservoirEcho/               # Echo-adapted ReservoirCpp (C++ files only)
│   ├── reservoircpp_cpp/        # Core reservoir computing library
│   │   ├── include/             # Headers (.hpp, .h)
│   │   └── src/                 # Source files (.cpp)
│   └── external/                # External dependencies (Eigen, etc.)
│
└── UnrealEcho/                  # Echo-adapted UnrealEngineCog (C++ files only)
    ├── Animation/               # Animation system
    ├── Audio/                   # Audio processing
    ├── Avatar/                  # Avatar components
    ├── Character/               # Character system
    ├── Cognitive/               # Cognitive processing
    ├── Consciousness/           # Consciousness streams
    ├── DeepTreeEchoAvatar/      # Deep Tree Echo avatar integration
    ├── Environment/             # Environment interaction
    ├── Interaction/             # Interaction system
    ├── Narrative/               # Narrative system
    ├── NeuralNetwork/           # Neural network components
    ├── Neurochemical/           # Neurochemical simulation
    ├── Patterns/                # Pattern recognition
    ├── Personality/             # Personality system
    └── Rendering/               # Rendering components
```

## Architecture Overview

### Core Components

| Component | Location | Description |
| :--- | :--- | :--- |
| **DeepTreeEchoCore** | `DeepTreeEcho/Core/` | Central orchestrator managing the 12-step cognitive loop and consciousness streams |
| **DeepTreeEchoReservoir** | `DeepTreeEcho/Reservoir/` | Hierarchical reservoir computing with intrinsic plasticity |
| **EmbodiedCognitionComponent** | `DeepTreeEcho/4ECognition/` | Full 4E cognition: Embodied, Embedded, Enacted, Extended |
| **AvatarEvolutionSystem** | `DeepTreeEcho/Avatar/` | Ontogenetic development and continuous enhancement |
| **ReservoirCpp** | `ReservoirEcho/` | Complete C++ reservoir computing library |
| **UnrealEngineCog** | `UnrealEcho/` | Unreal Engine cognitive components |

### Cognitive Architecture

The system implements a **dual-hemisphere cognitive model** with:

#### 12-Step Cognitive Cycle

The cognitive loop operates on a 12-step cycle with 3 concurrent consciousness streams:

| Step | Stream 1 (Perceiving) | Stream 2 (Acting) | Stream 3 (Reflecting) |
|------|----------------------|-------------------|----------------------|
| 1    | Perceive             | -                 | -                    |
| 2    | -                    | Act               | -                    |
| 3    | -                    | -                 | Reflect              |
| 4    | Perceive             | -                 | -                    |
| 5    | -                    | Act               | -                    |
| 6    | -                    | -                 | Reflect              |
| ...  | ...                  | ...               | ...                  |

Streams are phased 4 steps apart (120°), creating triadic synchronization points at {1,5,9}, {2,6,10}, {3,7,11}, {4,8,12}.

### Nested Shell Structure (OEIS A000081)

Execution contexts follow rooted tree enumeration:
- 1 nest → 1 term (global context)
- 2 nests → 2 terms (local context)
- 3 nests → 4 terms (detailed processing)
- 4 nests → 9 terms (fine-grained)

### 4E Embodied Cognition

The framework implements all four dimensions of embodied cognition:

1. **Embodied**: Body schema, proprioception, somatic markers
2. **Embedded**: Environmental affordances, niche coupling
3. **Embedded**: Sensorimotor contingencies, action-perception loops
4. **Extended**: Cognitive tools, external memory

## File Statistics

| Directory | C++ Files | Description |
| :--- | :---: | :--- |
| DeepTreeEcho | 8 | Custom cognitive components |
| ReservoirEcho | 1,531 | Reservoir computing library |
| UnrealEcho | 106 | Unreal Engine cognitive components |
| **Total** | **1,645** | All C++ files |

## Echo Goals Status

| Goal | Status | Notes |
|------|--------|-------|
| Reservoir Computing | ✅ Complete | Hierarchical ESN with intrinsic plasticity |
| 3 Concurrent Streams | ✅ Complete | Triadic synchronization implemented |
| 12-Step Cycle | ✅ Complete | OEIS A000081 nesting structure |
| 4E Cognition | ✅ Complete | All four dimensions implemented |
| Avatar Evolution | ✅ Complete | Ontogenetic development stages |
| Memory Integration | 🚧 In Progress | Hypergraph memory structure |
| OpenCog Integration | 📝 Planned | AtomSpace knowledge representation |

## Dependencies

- **Unreal Engine 5.x**: Core engine
- **ReservoirCpp**: Echo State Network implementation
- **Eigen**: Linear algebra (via ReservoirCpp)

## Documentation

📚 **[DOCUMENTATION_INDEX.md](DOCUMENTATION_INDEX.md)** - Complete guide to all documentation (recommended starting point)

### Core Documentation (57KB total)
- **[VERIFICATION_REPORT.md](VERIFICATION_REPORT.md)** (17KB): Complete verification of all components and echo-goals
- **[ECHO_INTEGRATION_STATUS.md](ECHO_INTEGRATION_STATUS.md)** (13KB): Detailed status report on echo-goals and integration
- **[RESERVOIRCPP_INTEGRATION_GUIDE.md](RESERVOIRCPP_INTEGRATION_GUIDE.md)** (16KB): Comprehensive guide for ReservoirCpp integration
- **[INTEGRATION_SUMMARY.md](INTEGRATION_SUMMARY.md)** (11KB): Executive summary of all integration tasks

### Quick Start
1. Start with [DOCUMENTATION_INDEX.md](DOCUMENTATION_INDEX.md) for navigation help
2. Review [VERIFICATION_REPORT.md](VERIFICATION_REPORT.md) for system status
3. Read [RESERVOIRCPP_INTEGRATION_GUIDE.md](RESERVOIRCPP_INTEGRATION_GUIDE.md) for implementation details
4. Check [ECHO_INTEGRATION_STATUS.md](ECHO_INTEGRATION_STATUS.md) for echo-goals completion

## License

MIT License - See LICENSE file for details.

## References

- ReservoirPy: Echo State Networks in Python
- OEIS A000081: Number of rooted trees with n nodes
- 4E Cognition: Embodied, Embedded, Enacted, Extended
- Deep Tree Echo: Hierarchical cognitive architecture
