#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 1 ]]; then
  echo "Usage: bash tests/check_boot_log.sh <boot-log>" >&2
  exit 2
fi

log_file="$1"

if [[ ! -f "$log_file" ]]; then
  echo "Boot log not found: $log_file" >&2
  exit 2
fi

require_pattern() {
  local pattern="$1"
  local description="$2"

  if ! grep -aEq -- "$pattern" "$log_file"; then
    echo "Missing boot evidence: $description" >&2
    exit 1
  fi
}

reject_pattern() {
  local pattern="$1"
  local description="$2"

  if grep -aEq -- "$pattern" "$log_file"; then
    echo "Boot validation failed: $description" >&2
    exit 1
  fi
}

require_pattern 'Platform board:[[:space:]]+TUYA_T5AI_BOARD' 'TUYA_T5AI_BOARD selection'
require_pattern 'tdd_tp_gt1151\.c' 'GT1151 driver initialization'
require_pattern 'lv_vendor_start complete' 'LVGL runtime start'
reject_pattern 'create touchpad node failed' 'LVGL touchpad node creation failed'
reject_pattern 'touchpad dev .* not found' 'registered touchpad device was not found'
reject_pattern 'HardFault|assert failed|fatal error' 'fatal runtime failure'

echo 'T5AI display and touch boot-log check passed.'
