# 双板互联方案：BLE 建链 → 交换 IP → TCP 互联

> 主机（Trigger）= `tuyatest`（本仓库，T5AI）
> 从机（Playback）= `advx26-playback`，分支 `develop/akira`（T5AI）
> 目标：无需任何第三方（路由器可选、无云、无手机）即可让两板建立稳定通信；
> 有 WiFi 环境时升级为 TCP 高速通道，用于下发播放会话与状态回传。

---

## 1. 现状盘点（已核实的代码事实）

### 1.1 从机 advx26-playback（不需要大改的部分）

| 能力 | 位置 | 状态 |
|---|---|---|
| BLE GATT 外围（Peripheral） | `src/playback_board_link_gatt.c` | 已实现，开机即广播 |
| 广播名 | `PLAYBACK`（设备名 `SoundPola Playback`） | 已实现 |
| 服务 UUID（128bit，小端字节序） | `51 C0 01 33 AF 80 A3 81 F9 4A 75 DD B5 87 D8 68` | 已实现 |
| Command 特征（主机→从机，Write / Write-NR） | UUID `0E 78 1C DE 37 D8 79 B6 EB 46 6E EC A5 F7 6F C6` | 已实现 |
| Report 特征（从机→主机，Notify + CCCD） | UUID `BE EF CD AF 85 D6 43 A9 7C 4A C9 61 6A 80 B3 D1` | 已实现 |
| ATT MTU | 默认 23，最大 247（单包有效载荷 244） | 已实现 |
| 消息分片帧 | `src/playback_board_link_wire.c`：magic `0xA7` + 16 字节头 | 已实现 |
| JSON 协议层 | `src/playback_board_link_json.c`：envelope `{schema_version, type, session_id, sequence_id, payload}` | 已实现 |
| 命令集 | HELLO / GET_STATUS / LOAD_SESSION / PLAY / PAUSE / SEEK_MS / STOP | 已实现 |
| 报告集 | HELLO_ACK / ACK / NACK / STATE / PROGRESS / COMPLETED / ERROR | 已实现 |
| 握手门禁 | 未 HELLO 的命令一律回 `NACK{NOT_HANDSHAKEN}` | 已实现 |
| TCP JSON server | `src/playback_board_link_tcp.*`，端口 **8787**，同一 JSON envelope | 已实现 |
| 媒体管线 | HTTP Range 下载 → tinyh264 视频 + MP3 音频，≤30s，480×320 | 已实现 |
| WiFi 连接 | `src/playback_network.c` — **编译期硬编码** `DEMO_WIFI_SSID/PASSWORD`，为空则启动失败 | 需改造 |
| 开机界面 | `src/mob_screen.c` — 无“黑底连接中”界面 | 需新增 |

### 1.2 主机 tuyatest

| 能力 | 位置 | 状态 |
|---|---|---|
| BLE（任何角色） | 无 | **完全缺失，需新增 central** |
| WiFi STA + UI | `ui_wifi` 等 | 已有 |
| TCP 客户端/服务端 | `src/tcp_link.c`（聊天用，端口 8788，LF 行协议） | 已有，可参考但协议不同 |
| UART board_link | `src/board_link.c`（LF 行 JSON） | 已有（与本方案无关，保留） |
| 云端播放列表 | `src/ui_playlist.c`：KV `sp_cloud_server`/`sp_cloud_token`，GET `/api/v1/contents` | 已有 |

### 1.3 关键约束

- Board Link 单条 JSON 消息上限 **4KB**；BLE 单包最多 244 字节 ⇒ 必须使用从机现成的 `0xA7` 分片帧（**直接移植 `playback_board_link_wire.c` 到主机**，两边同一份实现，避免协议漂移）。
- 从机 GATT 用的是 Beken `bk_bt` 原生 API；主机新代码建议用 TuyaOpen `tal_bluetooth`（central 角色），两者空中协议兼容，互不影响。
- 从机 TCP server 固定 8787；主机 `ui_pair` HTTP 也在 8787 —— 不冲突（一个是从机监听，一个是主机监听），但主机 TCP 客户端连接目标必须是**从机 IP**:8787。

---

## 2. 总体架构

```
┌─────────── 主机 tuyatest (TRIGGER) ───────────┐      ┌──────── 从机 playback (PLAYBACK) ────────┐
│                                               │      │                                           │
│  link_mgr（链路状态机，新增）                  │      │  playback_app（已有）                      │
│    ├─ ble_central.c（新增, tal_bluetooth）     │◄BLE─►│  playback_board_link_gatt.c（已有）        │
│    ├─ board_link_wire.c（移植 0xA7 分片）      │      │  playback_board_link_wire.c（已有）        │
│    ├─ board_link_proto.c（JSON envelope 编解码）│      │  playback_board_link_json.c（已有）        │
│    └─ tcp_client.c（新增, 连从机 8787）        │◄TCP─►│  playback_board_link_tcp.c（已有）         │
│                                               │      │  playback_network.c（改：运行时连 WiFi）    │
│  WiFi STA（已有）                              │      │  mob_screen.c（改：黑底等待界面）           │
└───────────────────────────────────────────────┘      └───────────────────────────────────────────┘
```

**双通道原则**：BLE 是永远在线的控制/引导通道；TCP 是可选的高速通道。TCP 可用时命令优先走 TCP，TCP 断开自动回落 BLE，BLE 断开则整体回到扫描重连。媒体数据永远不走板间链路——从机自己从网络下载。

---

## 3. 链路状态机（主机侧 link_mgr）

```
        ┌──────────────────────────────────────────────────────┐
        ▼                                                      │
  [SCANNING] ──发现UUID匹配──► [BLE_CONNECTING] ──连接+MTU+订阅──► [BLE_CONNECTED]
        ▲                            │失败/超时                     │
        │                            └────────► SCANNING            │发 HELLO
        │                                                          ▼
        │◄─────BLE断开────── [HANDSHAKEN] ◄──收 HELLO_ACK───────────┘
        │                        │
        │                        │发 SET_NETWORK{ssid,pswd}
        │                        ▼
        │                  [WIFI_PENDING] ──收 NETWORK_STATE{connected,ip}──► [WIFI_READY]
        │                        │超时(30s)重发,3次后停留在HANDSHAKEN                │
        │                        ▼                                                │TCP connect 从机IP:8787
        │                   (可继续纯BLE工作)                                       ▼
        │                                             [TCP_CONNECTING] ──TCP上再HELLO/HELLO_ACK──► [TCP_LINKED]
        │                                                     │失败: 5s退避重试, 命令走BLE              │
        └─────────────────────────────────────────────────────┴──────TCP断开: 回WIFI_READY,命令回落BLE──┘
```

### 从机侧界面状态（mob_screen 改造）

| 链路状态 | 屏幕 |
|---|---|
| 开机、无 BLE 连接 | 黑底 + “等待主机连接…”（呼吸动画可选） |
| BLE 已连接未握手 | 黑底 + “主机已连接，握手中…” |
| 已握手 | 黑底 + “已连接”/状态栏（含 WiFi/IP 信息） |
| LOAD_SESSION 后 | 进入现有播放流程界面 |
| 链路断开 | 回到“等待主机连接…” |

---

## 4. 协议细节

### 4.1 传输帧（BLE 与 UART 共用，主机需移植）

`playback_board_link_wire.c` 的 `0xA7` 分片帧：16 字节头 + 载荷，支持把 ≤4KB 的 JSON 消息切成 ≤244 字节的 ATT 包。主机侧**逐字节复制该文件**（含单元语义），不要重写。

### 4.2 JSON envelope（两个通道完全一致）

```json
{
  "schema_version": 1,
  "type": "HELLO",
  "session_id": "s-xxxx",
  "sequence_id": 42,
  "payload": { }
}
```

- `sequence_id` 主机侧单调递增；从机去重。
- 从机对每条命令回 ACK/NACK（含对应 sequence_id）。

### 4.3 握手

主机 → `HELLO{role:"TRIGGER", boot_id:"<随机串,开机生成>", max_message_bytes:4096, capabilities:[...]}`
从机 → `HELLO_ACK{role:"PLAYBACK", boot_id:"<从机boot_id>", ...}`

- BLE 通道、TCP 通道各自独立握手（从机侧现有实现即如此）。
- **boot_id 语义**：主机缓存从机 boot_id；心跳/重连后若 boot_id 变化 ⇒ 从机重启过 ⇒ 主机自动重发 SET_NETWORK + 重下发会话。

### 4.4 新增命令 SET_NETWORK（从机需实现）

```json
{ "type": "SET_NETWORK",
  "payload": { "ssid": "xxx", "password": "yyy" } }
```

从机行为：
1. 立即 ACK；
2. 调新 API `playback_network_connect(ssid, pswd)`（运行时连网，替代 DEMO_WIFI_* 编译期依赖；凭据写 KV，重启可复用）；
3. 结果异步回报：

```json
{ "type": "NETWORK_STATE",
  "payload": { "connected": true, "ip": "192.168.1.23", "rssi": -52 } }
```

失败回 `NETWORK_STATE{connected:false, reason:"AUTH_FAIL|TIMEOUT|..."}`。

### 4.5 心跳

- 主机在当前活跃通道每 **5s** 发 `GET_STATUS`；从机回 `STATE`。
- 连续 **3 次**无响应 ⇒ 判该通道断链：TCP 断 → 回落 BLE；BLE 断 → 回 SCANNING。
- 从机侧 15s 未收到任何命令 ⇒ UI 回“等待主机”。

### 4.6 通道选择规则（主机）

| 命令 | 通道 |
|---|---|
| HELLO / GET_STATUS | 各通道各自发 |
| SET_NETWORK | BLE（此时尚无 TCP） |
| LOAD_SESSION / PLAY / PAUSE / SEEK_MS / STOP | TCP 优先，TCP 不可用走 BLE |
| 从机报告（STATE/PROGRESS/…） | 从机在收到命令的通道上回 |

---

## 5. 实施计划

### 阶段 A：主机 BLE central（工作量最大）

新文件（`source/embedded/src|include`）：
1. `board_link_wire.c/h` — 从 playback 仓库移植 0xA7 分片编解码；
2. `ble_central.c/h` — `tal_bluetooth` central：
   - init：`tal_ble_bt_init(TAL_BLE_ROLE_CENTRAL, evt_cb)`；
   - 扫描：过滤广播中的 128bit 服务 UUID（或名称 "PLAYBACK" 兜底）；
   - 连接 → MTU 247 → 服务发现 → 写 CCCD 订阅 report → 提供 `send(bytes)` / `on_rx(bytes)`；
   - 断链事件上抛给 link_mgr；
3. `board_link_proto.c/h` — envelope 组包/解析（cJSON），HELLO/SET_NETWORK/LOAD_SESSION 等构造器 + 报告解析器；
4. `link_mgr.c/h` — 第 3 节状态机 + 心跳定时器 + 通道选择 + boot_id 缓存。

验收：主机日志可见 `HELLO_ACK received (ble)`，从机 GATT status `handshake_complete=true`。

### 阶段 B：从机改造

1. `SET_NETWORK` 命令接入 `playback_board_link_json.c`（命令枚举、解析、分发）；
2. `playback_network.c`：拆出 `playback_network_connect(ssid,pswd)`；启动不再因 DEMO_WIFI 为空而失败；连网结果回调发 `NETWORK_STATE`（含本机 IP，取自 netmgr/`tal_net_get_ip`）；凭据存 KV；
3. `mob_screen.c`：新增黑底等待/握手中/已连接三态界面，由 board link 状态回调驱动；
4. 确认 TCP server 在 WiFi 起来后自动监听 8787（现有代码已如此，仅回归验证）。

验收：手机 BLE 调试工具或主机发 SET_NETWORK，从机连网并回 NETWORK_STATE，屏幕状态正确切换。

### 阶段 C：主机 TCP 通道 + 整合

1. `tcp_board_link.c/h`（主机新文件，勿与聊天用 `tcp_link.c` 混用）：client 连 `从机IP:8787`，LF 行 JSON（与从机 TCP server 帧格式对齐——**实施前核实从机 TCP 是 LF 行还是 0xA7 帧**，以代码为准）；
2. link_mgr 接入：WIFI_READY 后自动建 TCP、TCP 上二次握手、通道切换与回落；
3. 主机业务接入：ui_playlist 选中条目 → 主机从云端解析出 video/audio URL → 组 LOAD_SESSION 经链路下发。

验收（集成测试脚本）：
1. 两板上电（任意顺序）→ 30s 内 BLE 握手完成；
2. 主机下发 SET_NETWORK → 从机 60s 内回 NETWORK_STATE(connected)；
3. TCP 建立并二次握手；
4. 主机发 LOAD_SESSION + PLAY → 从机下载并播放，PROGRESS/COMPLETED 正常回传；
5. 拔路由器 → TCP 断，命令回落 BLE，NACK/STATE 仍可达；
6. 从机断电重启 → 主机检测 boot_id 变化，自动重新引导全流程。

---

## 6. 风险与对策

| 风险 | 影响 | 对策 |
|---|---|---|
| `tal_bluetooth` central 在 T5AI 上的服务发现/CCCD 写细节与文档不符 | 阶段 A 延期 | 先写最小 central demo（扫描+连接+订阅 echo）单独验证，再接协议层 |
| 从机 GATT 是 bk_bt 原生实现，广播/ATT 行为有厂商特化 | 互联失败 | 用手机 nRF Connect 先抓从机广播与特征行为作为基准 |
| BLE 与 WiFi 共存（同一 2.4G 射频）吞吐/稳定性 | 心跳抖动 | 心跳超时取 3 次×5s 的宽松值；大 payload 走 TCP |
| 从机 TCP server 实际帧格式待核实（LF 行 vs 0xA7） | 阶段 C 返工 | 实施前读 `playback_board_link_tcp.c` 确认，以代码为准 |
| 4KB LOAD_SESSION 经 BLE 分片传输耗时（~20 包） | 回落模式延迟 | 可接受（一次性下发）；必要时 payload 精简 |
| 主机 flash/RAM 增量（BLE 协议栈） | 构建失败 | 打开 BLE Kconfig 后先空跑编译看尺寸 |

## 7. 里程碑顺序

1. **A1**：主机最小 BLE central demo 连上从机并订阅成功（风险最高，先做）
2. **A2**：wire + proto + link_mgr，BLE HELLO 握手打通
3. **B**：从机 SET_NETWORK / 运行时连网 / 黑屏界面
4. **C**：TCP 通道 + 通道切换 + LOAD_SESSION 端到端播放
5. 集成回归（第 5 节验收清单）
