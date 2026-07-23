#!/usr/bin/env bash
set -euo pipefail

project_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
source_config="${project_root}/firmware/playback/app_default.config"
generated_config="${project_root}/firmware/playback/.build/cache/using.config"

require_exact_line() {
  local file="$1"
  local expected="$2"

  if ! grep -Fqx -- "$expected" "$file"; then
    printf 'Missing required config in %s: %s\n' "$file" "$expected" >&2
    return 1
  fi
}

require_exact_line "$source_config" 'CONFIG_BOARD_CHOICE_T5AI=y'
require_exact_line "$source_config" 'CONFIG_BOARD_CHOICE_TUYA_T5AI_BOARD=y'
require_exact_line "$source_config" 'CONFIG_TUYA_T5AI_BOARD_LCD_35565=y'
require_exact_line "$source_config" 'CONFIG_ENABLE_LVGL_TP=y'

if [[ -f "$generated_config" ]]; then
  require_exact_line "$generated_config" 'CONFIG_BOARD_CHOICE="TUYA_T5AI_BOARD"'
  require_exact_line "$generated_config" 'CONFIG_BOARD_CHOICE_TUYA_T5AI_BOARD=y'
  require_exact_line "$generated_config" 'CONFIG_TUYA_T5AI_BOARD_LCD_35565=y'
  require_exact_line "$generated_config" 'CONFIG_ENABLE_TP=y'
  require_exact_line "$generated_config" 'CONFIG_ENABLE_LVGL_TP=y'

  if grep -Fqx 'CONFIG_BOARD_CHOICE_SPARKLEIOT_T5AI_DEV=y' "$generated_config"; then
    echo 'Generated config unexpectedly selected SPARKLEIOT_T5AI_DEV.' >&2
    exit 1
  fi
fi

echo 'Tuya board and touch configuration check passed.'
