# TuyaOpen T5AI + PN532 NFC 读卡器示例

在 Tuya T5AI 开发板（`TUYA_T5AI_BOARD`）上通过 **GPIO 软件 I2C** 驱动 PN532 NFC
模块，并在板载 LCD 上用 LVGL 显示识别状态与卡片 UID，支持触屏按钮重新扫描。

## 功能

- PN532 在线 / 离线状态显示
- 扫描 ISO14443A 卡片并显示 UID
- 触屏按钮触发重新扫描
- PN532 离线时在屏幕上显示诊断信息

## 硬件接线

PN532 模块拨码开关设置为 **I2C 模式**（SW1=ON, SW2=OFF），改动拨码后需给模块
完全断电再上电。

| PN532 | T5AI P11 排针 |
|---|---|
| SCL | 3 脚 P02 |
| SDA | 4 脚 P03 |
| VCC | 14 脚 3.3V |
| GND | 13 / 17 脚 GND |

> 不要接 5V。日志串口为板载 USB 转串口（与 P00/P01 复用，勿占用）。

## 源码结构

```
source/embedded/
├── src/
│   ├── tuya_app_main.c     # 入口
│   └── app_nfc.c           # LVGL 界面 + 扫描线程
├── include/
│   └── app_nfc.h
├── usr_board/
│   ├── pn532_i2c.c         # PN532 驱动（定时开漏软件 I2C）
│   ├── pn532_i2c.h
│   └── usr_board.c
├── CMakeLists.txt
└── app_default.config      # T5AI 板级 Kconfig
```

## 关键实现说明

PN532 的 I2C 从机会做 **时钟拉伸（clock stretching）**：处理每个字节时主动拉低
SCL。因此软件 I2C 必须：

1. SCL 使用开漏语义——置高时切换为上拉输入，并**轮询等待 SCL 真正变高**后才
   继续（`pn532_i2c.c` 中 `i2c_scl_release()`，超时 5ms）；
2. SDA 同样用"输出低 / 上拉输入"模拟开漏（T5AI 的 `tkl_gpio` 不支持真开漏）；
3. 地址阶段带重试——PN532 内部忙时会临时 NAK 自己的地址（7 位地址 `0x24`）；
4. 使用 `tkl_system_sleep_us()` 产生真实微秒延时，时钟速率低于 100 kHz。

若 SCL 用推挽输出，会与 PN532 的时钟拉伸冲突，表现为读回 `0xFC` 之类的错位
数据和地址 NAK。

## 构建与烧录

需要 [TuyaOpen SDK](https://github.com/tuya/TuyaOpen)：

```bash
cd source/embedded
tos.py build
tos.py flash -p <烧录串口>      # T5AI 双串口：一个烧录，一个日志
tos.py monitor -p <日志串口>
```

正常启动日志：

```
PN532 address 0x24 ACK, status=0x00
PN532 timed software I2C: SCL=P02 SDA=P03 address=0x24
PN532 I2C online, IC=0x32 firmware=1.6 support=0x07
```
