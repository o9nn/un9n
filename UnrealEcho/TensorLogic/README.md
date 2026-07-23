# Tensor Logic Module

A complete implementation of **Tensor Logic** for Unreal Engine, providing unified neural-symbolic AI reasoning.

## Quick Start

### 1. Add Component to Actor

```cpp
// In your actor header
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cognitive")
UTensorLogicEngine* TensorLogicEngine;

// In constructor
TensorLogicEngine = CreateDefaultSubobject<UTensorLogicEngine>(TEXT("TensorLogicEngine"));
```

### 2. Add Facts

```cpp
FTensorLogicAtom Fact;
Fact.Predicate = TEXT("human");
Fact.Arguments.Add(FTensorLogicTerm::MakeConstant(TEXT("socrates"), 1.0f));
Fact.TruthValue = 1.0f;
TensorLogicEngine->AddFact(Fact);
```

### 3. Add Rules

```cpp
// Rule: mortal(X) :- human(X)
FTensorLogicRule Rule;
Rule.Head.Predicate = TEXT("mortal");
Rule.Head.Arguments.Add(FTensorLogicTerm::MakeVariable(TEXT("X")));

FTensorLogicAtom BodyAtom;
BodyAtom.Predicate = TEXT("human");
BodyAtom.Arguments.Add(FTensorLogicTerm::MakeVariable(TEXT("X")));
Rule.Body.Add(BodyAtom);

TensorLogicEngine->AddRule(Rule);
```

### 4. Query

```cpp
FTensorLogicAtom Query;
Query.Predicate = TEXT("mortal");
Query.Arguments.Add(FTensorLogicTerm::MakeConstant(TEXT("socrates"), 1.0f));

FTensorLogicQueryResult Result = TensorLogicEngine->Query(Query, ETensorLogicInferenceMode::BackwardChaining);

if (Result.bSuccess)
{
    // Socrates is mortal!
}
```

## Example Actor

Use `ATensorLogicExampleActor` to see working examples:

1. Place the actor in your level
2. Call `RunAllExamples()` in Blueprint or code
3. Check output log for detailed results

## Features

### Symbolic Reasoning
- ✅ Forward chaining (data-driven)
- ✅ Backward chaining (goal-driven)
- ✅ Proof tracing
- ✅ Unification
- ✅ Variable substitution

### Neural Learning
- ✅ Entity embeddings
- ✅ Relation embeddings
- ✅ Gradient descent
- ✅ RESCAL decomposition
- ✅ Predicate invention

### Hybrid AI
- ✅ Combined symbolic + neural
- ✅ Integration with hypergraph memory
- ✅ Neural-guided inference
- ✅ Symbolic-constrained learning

## Documentation

See `Documentation/TensorLogic_Implementation.md` for complete documentation.

## Testing

```
Session Frontend → Automation → TensorLogic.*
```

25 comprehensive unit tests covering all functionality.

## References

- https://tensor-logic.org/
- https://bengoertzel.substack.com/p/tensor-logic-for-bridging-neural
- arXiv:2510.12269 - "Tensor Logic: The Language of AI"
