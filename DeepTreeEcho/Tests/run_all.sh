#!/usr/bin/env bash
#
# run_all.sh - build and run every standalone verification harness.
#
# WHY THIS EXISTS
# These harnesses only ever passed because someone ran them by hand. During development a merge
# from main silently broke GameTraining (the header kept a bitmask refactor while the .cpp was
# rewritten against the old field names) and the module could not compile for hours - while all
# four harnesses kept reporting success, because they exercise mirrored logic rather than the
# real translation units. Running them automatically is the cheap half of closing that gap; the
# --syntax pass below is the other half, since it compiles the ACTUAL headers.
#
# USAGE
#   ./run_all.sh              build and run every harness
#   ./run_all.sh --syntax     also syntax-check the real module headers (catches merge breakage)
#   ./run_all.sh --keep       leave the built binaries in place for debugging
#
# Exit code is the number of failed harnesses, so CI fails loudly on any regression.
# Harnesses are DISCOVERED, not listed - a new Standalone*.cpp is picked up with no edit here.

set -uo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"          # DeepTreeEcho/
SHIM="${ROOT}/Mastery/Tests/StandaloneShim"
BUILD_DIR="${BUILD_DIR:-$(mktemp -d)}"
CXX="${CXX:-g++}"
CXXFLAGS="${CXXFLAGS:--std=c++17 -O2 -Wall}"

DO_SYNTAX=0
KEEP=0
for arg in "$@"; do
  case "$arg" in
    --syntax) DO_SYNTAX=1 ;;
    --keep)   KEEP=1 ;;
    -h|--help) sed -n '2,22p' "${BASH_SOURCE[0]}"; exit 0 ;;
    *) echo "unknown option: $arg" >&2; exit 2 ;;
  esac
done

if ! command -v "${CXX}" >/dev/null 2>&1; then
  echo "FATAL: compiler '${CXX}' not found" >&2
  exit 127
fi

cleanup() { [[ "${KEEP}" -eq 0 ]] && rm -rf "${BUILD_DIR}"; }
trap cleanup EXIT

# Colour only when attached to a terminal, so CI logs stay clean.
if [[ -t 1 ]]; then
  R=$'\e[31m'; G=$'\e[32m'; Y=$'\e[33m'; B=$'\e[1m'; N=$'\e[0m'
else
  R=''; G=''; Y=''; B=''; N=''
fi

echo "${B}=== DeepTreeEcho standalone verification ===${N}"
echo "compiler : $(${CXX} --version | head -1)"
echo "flags    : ${CXXFLAGS}"
echo "build dir: ${BUILD_DIR}"
echo

# Discover harnesses: any Standalone*.cpp under a Tests/ directory.
mapfile -t HARNESSES < <(find "${ROOT}" -type d -name Tests -exec find {} -maxdepth 1 -name 'Standalone*.cpp' \; 2>/dev/null | sort)

if [[ ${#HARNESSES[@]} -eq 0 ]]; then
  echo "${R}FATAL: no Standalone*.cpp harnesses found under ${ROOT}${N}" >&2
  exit 1
fi

FAILED=0
PASSED=0
declare -a RESULTS

for SRC in "${HARNESSES[@]}"; do
  NAME="$(basename "${SRC}" .cpp)"
  REL="${SRC#"${ROOT}"/}"
  BIN="${BUILD_DIR}/${NAME}"

  printf '%-46s ' "${REL}"

  if ! BUILD_LOG="$(${CXX} ${CXXFLAGS} -I"${SHIM}" -o "${BIN}" "${SRC}" 2>&1)"; then
    echo "${R}BUILD FAILED${N}"
    echo "${BUILD_LOG}" | sed 's/^/    /' | head -25
    RESULTS+=("${R}BUILD${N}  ${REL}")
    ((FAILED++))
    continue
  fi

  # Warnings are surfaced but not fatal - a harness that builds dirty still tells us something.
  if [[ -n "${BUILD_LOG}" ]]; then
    WARN_COUNT="$(grep -c 'warning:' <<<"${BUILD_LOG}" || true)"
    [[ "${WARN_COUNT}" -gt 0 ]] && printf '%s(%s warn)%s ' "${Y}" "${WARN_COUNT}" "${N}"
  fi

  if RUN_LOG="$("${BIN}" 2>&1)"; then
    SUMMARY="$(tail -1 <<<"${RUN_LOG}")"
    echo "${G}PASS${N}  ${SUMMARY}"
    RESULTS+=("${G}PASS${N}   ${REL}")
    ((PASSED++))
  else
    echo "${R}FAIL${N}"
    # Show only the failing assertions plus the tail, so CI logs stay readable.
    grep -E '\[FAIL\]' <<<"${RUN_LOG}" | sed 's/^/    /' | head -20
    tail -3 <<<"${RUN_LOG}" | sed 's/^/    /'
    RESULTS+=("${R}FAIL${N}   ${REL}")
    ((FAILED++))
  fi
done

# ---- Optional: syntax-check the REAL headers ------------------------------------------------
# This is the part that would have caught the merge breakage. The harnesses mirror logic; these
# are the actual files the engine compiles. UE headers cannot fully compile standalone (they need
# CoreMinimal, .generated.h, UHT output), so this is a best-effort parse that reports rather than
# gates - but a hard error here is still a strong signal something is structurally wrong.
if [[ "${DO_SYNTAX}" -eq 1 ]]; then
  echo
  echo "${B}--- header syntax survey (best-effort; UE headers need UHT) ---${N}"
  SYNTAX_ERRORS=0
  while IFS= read -r HDR; do
    REL="${HDR#"${ROOT}"/}"
    if ERR="$(${CXX} -fsyntax-only -std=c++17 -I"${SHIM}" -x c++ "${HDR}" 2>&1)"; then
      printf '  %-56s %sok%s\n' "${REL}" "${G}" "${N}"
    else
      # Missing CoreMinimal/generated.h is expected for UE headers, not a real defect.
      if grep -qE "CoreMinimal\.h|\.generated\.h|Components/" <<<"${ERR}"; then
        printf '  %-56s %sskip (needs UE)%s\n' "${REL}" "${Y}" "${N}"
      else
        printf '  %-56s %sSYNTAX ERROR%s\n' "${REL}" "${R}" "${N}"
        head -6 <<<"${ERR}" | sed 's/^/      /'
        ((SYNTAX_ERRORS++))
      fi
    fi
  done < <(find "${ROOT}/Mastery" -name '*.h' -not -path '*/StandaloneShim/*' | sort)
  echo "  headers with real syntax errors: ${SYNTAX_ERRORS}"
  ((FAILED += SYNTAX_ERRORS))
fi

# ---- Summary --------------------------------------------------------------------------------
echo
echo "${B}=== summary ===${N}"
for LINE in "${RESULTS[@]}"; do echo "  ${LINE}"; done
echo
if [[ "${FAILED}" -eq 0 ]]; then
  echo "${G}${B}ALL ${PASSED} HARNESSES PASSED${N}"
else
  echo "${R}${B}${FAILED} FAILED${N} (${PASSED} passed)"
fi

exit "${FAILED}"
