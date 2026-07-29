#!/bin/bash
cd "$(dirname "$0")"
python "C:/Users/lazy_lz/TuyaOpenIDE/TuyaOpenSDK/tos.py" build > codex-ui-build.stdout.log 2> codex-ui-build.stderr.log
echo "EXIT=$?"
