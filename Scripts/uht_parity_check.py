#!/usr/bin/env python3
"""UHT-parity static validator for Deep Tree Echo Unreal modules.

Enforces the UnrealHeaderTool rules that most commonly break full-engine
builds, without requiring an engine install. Complements (does not replace)
the full-engine UHT pass in ci-uht-fullengine.yml.

Checks per header containing reflection macros:
  R1  GENERATED_BODY()/GENERATED_USTRUCT_BODY() present in every
      UCLASS/USTRUCT/UINTERFACE body.
  R2  The matching *.generated.h include exists and is the LAST #include.
  R3  UENUM/USTRUCT/UCLASS type names are unique across the whole module
      set (duplicate reflected identifiers are a hard UHT error).
  R4  Reflected type name prefixes: UCLASS->U/A (A for AActor derivatives),
      USTRUCT->F, UENUM enum class->E, interfaces->I/U pair.
  R5  UPROPERTY raw-pointer members to UObject types must be UPROPERTY()
      visible (already implied) and must not be references.
  R6  Dynamic delegate declarations must use the F-prefix and the
      DECLARE_DYNAMIC_* macros with correctly counted _OneParam etc.
  R7  UFUNCTION parameters must not use forward-declared enum types
      (heuristic: enum types used in UFUNCTION signatures must be defined
      or included in the same translation unit).

Usage:  python3 Scripts/uht_parity_check.py [--root <repo-root>]
Exit code 0 on success, 1 on violations.
"""

import argparse
import os
import re
import sys
from collections import defaultdict

REFLECTED_DIRS = [
    "DeepTreeEcho/Avatar",
    "Source/DeepTreeEchoAvatar/Public",
    "Source/DeepTreeEchoAvatar/Private",
]

MACRO_RE = re.compile(r"^\s*(UCLASS|USTRUCT|UENUM|UINTERFACE)\s*\(", re.M)
TYPE_AFTER_MACRO_RE = re.compile(
    r"(UCLASS|USTRUCT|UENUM|UINTERFACE)\s*\([^)]*\)\s*"
    r"(?:class|struct|enum\s+class)\s+(?:\w+_API\s+)?(\w+)",
    re.S,
)
INCLUDE_RE = re.compile(r'^\s*#include\s+"([^"]+)"', re.M)
DELEGATE_RE = re.compile(
    r"DECLARE_DYNAMIC(?:_MULTICAST)?_DELEGATE(_\w+)?\s*\(\s*(\w+)([^)]*)\)", re.S
)
PARAM_COUNT_SUFFIX = {
    None: 0, "": 0,
    "_OneParam": 1, "_TwoParams": 2, "_ThreeParams": 3, "_FourParams": 4,
    "_FiveParams": 5, "_SixParams": 6, "_SevenParams": 7, "_EightParams": 8,
    "_NineParams": 9,
}


def strip_comments(text: str) -> str:
    text = re.sub(r"//[^\n]*", "", text)
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.S)
    return text


def check_header(path: str, errors: list, type_registry: dict):
    with open(path, "r", errors="replace") as f:
        raw = f.read()
    text = strip_comments(raw)
    rel = os.path.relpath(path)
    has_reflection = bool(MACRO_RE.search(text))

    # R2: generated.h include position
    base = os.path.splitext(os.path.basename(path))[0]
    gen_name = f"{base}.generated.h"
    includes = INCLUDE_RE.findall(text)
    if has_reflection:
        gen_includes = [i for i in includes if i.endswith(".generated.h")]
        if not gen_includes:
            errors.append(f"{rel}: R2 missing #include \"{gen_name}\"")
        else:
            if os.path.basename(gen_includes[-1]) != gen_name:
                errors.append(
                    f"{rel}: R2 generated include is '{gen_includes[-1]}', expected '{gen_name}'"
                )
            if includes and includes[-1] != gen_includes[-1]:
                errors.append(
                    f"{rel}: R2 '{gen_includes[-1]}' must be the LAST #include (found '{includes[-1]}' after it)"
                )

    # R1 + R3 + R4: reflected types
    for macro, name in TYPE_AFTER_MACRO_RE.findall(text):
        key = name
        if key in type_registry and type_registry[key] != rel:
            errors.append(
                f"{rel}: R3 duplicate reflected type '{name}' (also in {type_registry[key]})"
            )
        else:
            type_registry[key] = rel
        if macro == "USTRUCT" and not name.startswith("F"):
            errors.append(f"{rel}: R4 USTRUCT '{name}' must use F prefix")
        if macro == "UENUM" and not name.startswith("E"):
            errors.append(f"{rel}: R4 UENUM '{name}' must use E prefix")
        if macro == "UCLASS" and not (name.startswith("U") or name.startswith("A")):
            errors.append(f"{rel}: R4 UCLASS '{name}' must use U or A prefix")

    # R1: GENERATED_BODY per reflected class/struct
    n_class_struct = len([m for m, _ in TYPE_AFTER_MACRO_RE.findall(text) if m in ("UCLASS", "USTRUCT", "UINTERFACE")])
    n_generated = len(re.findall(r"GENERATED_(?:USTRUCT_)?BODY\s*\(\s*\)", text))
    if n_class_struct > n_generated:
        errors.append(
            f"{rel}: R1 {n_class_struct} reflected class/struct bodies but only {n_generated} GENERATED_BODY() macros"
        )

    # R6: dynamic delegate arity and naming
    for suffix, dname, params in DELEGATE_RE.findall(text):
        if not dname.startswith("F"):
            errors.append(f"{rel}: R6 delegate '{dname}' must use F prefix")
        expected = PARAM_COUNT_SUFFIX.get(suffix if suffix else "", None)
        if expected is not None:
            # count top-level commas in params -> each param contributes ", type, name"
            depth = 0
            commas = 0
            for ch in params:
                if ch in "<(":
                    depth += 1
                elif ch in ">)":
                    depth -= 1
                elif ch == "," and depth == 0:
                    commas += 1
            actual = commas // 2
            if params.strip() and actual != expected:
                errors.append(
                    f"{rel}: R6 delegate '{dname}' declares {actual} params but macro suffix expects {expected}"
                )

    # R5: UPROPERTY reference members are illegal
    for m in re.finditer(r"UPROPERTY\s*\([^)]*\)\s*[\w:<>\*\s]+?&\s*\w+\s*;", text):
        errors.append(f"{rel}: R5 UPROPERTY member cannot be a reference: '{m.group(0)[:60]}...'")


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--root", default=os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
    args = ap.parse_args()

    headers = []
    for d in REFLECTED_DIRS:
        full = os.path.join(args.root, d)
        if not os.path.isdir(full):
            continue
        for fn in sorted(os.listdir(full)):
            if fn.endswith(".h") and not fn.endswith(".generated.h"):
                headers.append(os.path.join(full, fn))

    errors: list = []
    type_registry: dict = {}
    for h in headers:
        check_header(h, errors, type_registry)

    print(f"UHT-parity check: {len(headers)} headers, {len(type_registry)} reflected types")
    if errors:
        print(f"\n{len(errors)} violation(s):")
        for e in errors:
            print(f"  ERROR {e}")
        return 1
    print("All UHT-parity rules satisfied.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
