#!/usr/bin/env bash
set -euo pipefail

readonly PROJECT_ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
readonly PATCH_FILE="${PROJECT_ROOT}/patches/tuyaopen-v1.9.0-t5ai-display.patch"
readonly SDK_ROOT="${1:-${TUYAOPEN_ROOT:-${HOME}/SDKs/TuyaOpen-v1.9.0}}"

if [[ ! -d "${SDK_ROOT}/.git" ]]; then
    printf 'error: TuyaOpen SDK Git repository not found: %s\n' "${SDK_ROOT}" >&2
    exit 1
fi

if [[ ! -f "${PATCH_FILE}" ]]; then
    printf 'error: patch file not found: %s\n' "${PATCH_FILE}" >&2
    exit 1
fi

if ! git -C "${SDK_ROOT}" tag --points-at HEAD | grep -Fxq 'v1.9.0'; then
    printf 'error: expected TuyaOpen v1.9.0 at %s\n' "${SDK_ROOT}" >&2
    printf 'current revision: %s\n' "$(git -C "${SDK_ROOT}" describe --tags --always --dirty)" >&2
    exit 1
fi

if git -C "${SDK_ROOT}" apply --check "${PATCH_FILE}" 2>/dev/null; then
    git -C "${SDK_ROOT}" apply "${PATCH_FILE}"
    printf 'applied TuyaOpen T5AI display patch to %s\n' "${SDK_ROOT}"
elif git -C "${SDK_ROOT}" apply --reverse --check "${PATCH_FILE}" 2>/dev/null; then
    printf 'TuyaOpen T5AI display patch is already applied in %s\n' "${SDK_ROOT}"
else
    printf 'error: patch cannot be applied cleanly to %s\n' "${SDK_ROOT}" >&2
    exit 1
fi
