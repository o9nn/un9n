#!/usr/bin/env bash
# =============================================================================
# verify-avatar-module.sh
# Standalone syntax verification for the DeepTreeEcho Avatar module.
#
# Compiles every Avatar .cpp against the UE-compat shim headers in
# Source/DeepTreeEcho, catching type errors, duplicate UENUM/USTRUCT
# definitions, and API drift without requiring an Unreal Engine install.
#
# Usage: ./Scripts/verify-avatar-module.sh
# Exit:  0 if all files pass, 1 otherwise.
# =============================================================================
set -uo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

CXX="${CXX:-g++}"
FLAGS=(-std=c++17 -fsyntax-only -I Source/DeepTreeEcho -I DeepTreeEcho -I DeepTreeEcho/Avatar)

PASS=0
FAIL=0
FAILED_FILES=()

for f in DeepTreeEcho/Avatar/*.cpp; do
    if "$CXX" "${FLAGS[@]}" "$f" 2>/tmp/avatar_verify_err.log; then
        printf 'OK   %s\n' "$(basename "$f")"
        PASS=$((PASS+1))
    else
        printf 'FAIL %s\n' "$(basename "$f")"
        head -5 /tmp/avatar_verify_err.log | sed 's/^/     /'
        FAIL=$((FAIL+1))
        FAILED_FILES+=("$f")
    fi
done

echo "----------------------------------------"
echo "Avatar module verification: $PASS passed, $FAIL failed"
if [ "$FAIL" -gt 0 ]; then
    printf 'Failed: %s\n' "${FAILED_FILES[@]}"
    exit 1
fi
exit 0
