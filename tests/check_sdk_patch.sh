#!/usr/bin/env bash
set -euo pipefail

readonly PROJECT_ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
readonly PATCH_FILE="${PROJECT_ROOT}/patches/tuyaopen-v1.9.0-t5ai-display.patch"
readonly SDK_ROOT="${1:-${TUYAOPEN_ROOT:-${HOME}/SDKs/TuyaOpen-v1.9.0}}"

if [[ ! -s "${PATCH_FILE}" ]]; then
    printf 'missing SDK patch: %s\n' "${PATCH_FILE}" >&2
    exit 1
fi

grep -Fq 'TUYA_DISPLAY_ROTATION_270' "${PATCH_FILE}"
grep -Fq 'Rotated display uses CPU framebuffer copy' "${PATCH_FILE}"

if [[ -d "${SDK_ROOT}/.git" ]]; then
    if git -C "${SDK_ROOT}" apply --check "${PATCH_FILE}" 2>/dev/null; then
        printf 'SDK patch check passed: clean v1.9.0 tree can accept the patch.\n'
    elif git -C "${SDK_ROOT}" apply --reverse --check "${PATCH_FILE}" 2>/dev/null; then
        printf 'SDK patch check passed: patch is already applied.\n'
    else
        printf 'SDK patch check failed against %s\n' "${SDK_ROOT}" >&2
        exit 1
    fi
else
    printf 'SDK not found; patch content check passed.\n'
fi
