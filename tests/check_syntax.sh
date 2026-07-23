#!/usr/bin/env bash
set -euo pipefail

project_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

cc \
  -std=c11 \
  -Wall \
  -Wextra \
  -Werror \
  -fsyntax-only \
  -I"${project_root}/tests/stubs" \
  -I"${project_root}/firmware/playback/include" \
  "${project_root}/firmware/playback/src/main.c" \
  "${project_root}/firmware/playback/src/mob_screen.c"

echo "C syntax check passed."
