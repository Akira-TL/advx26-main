# MOB Tuya T5AI display

Custom LVGL screen application for the Tuya T5AI-Board (`T5-E1-IPEX`) connected in `/home/akira/Projects/mob`.

## Detected hardware

- USB bridge: `1a86:55d2 QinHeng USB Dual_Serial`
- Download port: `/dev/ttyACM0`
- Log port: `/dev/ttyACM1`
- Log baud rate: `460800`
- Existing firmware is already refreshing LCD and LVGL, so the physical display path is working.

The initial target is the official **3.5-inch 320x480 ILI9488** display configuration. If the attached panel is the 0.9-inch ST7735 board, change the board configuration before flashing.

## Layout

```text
firmware/
  CMakeLists.txt
  app_default.config
  config/
    TUYA_T5AI_BOARD_LCD_3.5.config
  include/
    mob_screen.h
  src/
    main.c
    mob_screen.c
patches/
  tuyaopen-v1.9.0-t5ai-display.patch
scripts/
  apply-tuyaopen-patches.sh
tests/
  stubs/                 # host-only headers for syntax checking
  check_syntax.sh
  check_config.sh
  check_sdk_patch.sh
```

`firmware/src/main.c` owns TuyaOpen/LVGL startup. `firmware/src/mob_screen.c` owns all visual composition, so future screen redesigns do not need to touch board initialization.

## Local checks

```bash
bash tests/check_syntax.sh
bash tests/check_config.sh
bash tests/check_sdk_patch.sh
```

`check_syntax.sh` checks C syntax and interface usage against lightweight stubs. `check_config.sh` prevents the board selection from silently falling back to `SPARKLEIOT_T5AI_DEV`; the required resolved board is `TUYA_T5AI_BOARD` with the 3.5-inch ILI9488/GT1151 module.

A captured startup log can also be checked with:

```bash
bash tests/check_boot_log.sh /path/to/boot.log
```

A real firmware build still requires TuyaOpen and the T5AI toolchain.

## TuyaOpen build

Install TuyaOpen outside this repository, apply the project-owned display patch, then build the canonical `firmware/` application directly:

```bash
git clone --branch v1.9.0 https://github.com/tuya/TuyaOpen.git ~/SDKs/TuyaOpen-v1.9.0
cd /home/akira/Projects/mob
bash scripts/apply-tuyaopen-patches.sh ~/SDKs/TuyaOpen-v1.9.0

source ~/SDKs/TuyaOpen-v1.9.0/export.sh
cd /home/akira/Projects/mob/firmware
tos.py check
tos.py config choice -c TUYA_T5AI_BOARD_LCD_3.5.config
tos.py build
```

The patch is required for this exact panel assembly. It sets the 3.5-inch ILI9488 path to `270°` and uses CPU framebuffer copies for rotated LVGL displays, avoiding the fixed dark band and flicker observed with the TuyaOpen v1.9.0 DMA2D path.

## Flash and monitor

Use the lower-numbered virtual serial port for downloading and the higher-numbered port for logs:

```bash
source ~/SDKs/TuyaOpen-v1.9.0/export.sh
cd /home/akira/Projects/mob/firmware
tos.py flash -p /dev/ttyACM0

tos.py monitor
# Select /dev/ttyACM1 and 460800 baud when prompted.
```

The standalone `tyutool` alternative uses `-p /dev/ttyACM0` when an explicit download port is required.

## Current screen

The current prototype intentionally uses only LVGL built-in primitives and the default font:

- dark full-screen background;
- `MOB` product mark;
- status badge;
- centered readiness card;
- full-width `TAP TO TEST TOUCH` control;
- visible `TOUCH OK` feedback after a successful GT1151 click event;
- no external image/font assets yet.

The display and touch composition remains isolated behind `mob_screen_create()`, so later product UI work does not need to modify TuyaOpen board initialization.

When the touch event reaches LVGL, the UI changes to `TOUCH OK` and the debug UART prints `MOB touch confirmed`. The physical GT1151 click path, 270° display orientation, full-frame composition, and stable output without the previous dark band/flicker have been verified on the attached hardware.
