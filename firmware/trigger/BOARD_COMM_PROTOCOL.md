# 板间 WiFi 通信协议 (Board-to-Board WiFi Link Protocol)

本文档定义两台设备之间通过 **WiFi (TCP)** 交换控制 / 状态消息的协议。
本设备（`tuyatest`，TuyaOpen T5AI）已按本协议实现了一个 **TCP 客户端**；
另一台设备只需实现一个兼容的 **TCP 服务器** 即可与本设备互通。

---

## 1. 总览

| 项目 | 约定 |
|---|---|
| 传输层 | TCP（IPv4） |
| 默认端口 | `8787`（本设备界面可改，对端按实际监听端口为准） |
| 消息格式 | **NDJSON** —— 每条消息是一行 JSON，以换行符 `\n`（0x0A）结尾 |
| 字符编码 | UTF-8（JSON 内容通常为 ASCII） |
| 单条消息最大长度 | 约 **2200 字节**（含结尾 `\n`），超长消息会被本设备丢弃 |
| 消息识别 | 顶层必须包含字符串字段 `"type"` |

---

## 2. 角色与连接流程

- **本设备（tuyatest / T5AI）**：TCP **客户端**，主动发起连接。
- **对端设备**：TCP **服务器**，监听固定端口（默认 `8787`）。

连接步骤：

1. 对端设备启动 TCP 服务器，监听 `0.0.0.0:8787`（允许任意来源 IP）。
2. 两台设备接入**同一 WiFi 网络**（同一子网，可互相 ping 通）。
3. 本设备进入 **COMM** 界面（主页上滑）：
   - 顶部显示 `My IP: <本机地址>`（仅供参考 / 调试）。
   - 在 `Peer IP` 输入对端地址（如 `192.168.1.20`），`Port` 输入对端监听端口（默认 `8787`）。
   - 点击 **Connect**。状态显示 `Connecting...` → `Connected`（绿色）。
4. 连接建立后，双方可随时互发 NDJSON 消息。

> 说明：连接方向是**本设备 → 对端**（本设备为客户端）。消息协议本身是对称的，
> 建立连接后任意一方都可以发送任意类型的消息。

---

## 3. 帧格式（Framing）

- 每条消息 = 一行 JSON + 一个 `\n`。
- 接收方按 `\n` 分行解析；应**容忍** `\r`（0x0D，即兼容 `\r\n` 结尾）。
- 本设备发送时会在消息末尾自动追加 `\n`，界面上输入的内容**无需**手动加换行。
- 一条消息**不得**包含裸 `\n`（会破坏分帧）。JSON 内的换行请用 `\n` 转义。
- 超过 ~2200 字节的单条消息会被丢弃，请保持消息精简（URL 除外，一般远小于该限制）。

示例（实际线路上的一帧，`<LF>` 表示换行）：

```
{"type":"status"}<LF>
```

---

## 4. 消息类型

所有消息顶层都有 `"type"` 字段。以下为约定的类型：

### 4.1 `load` — 加载媒体（命令）

要求对端加载并准备播放指定的音视频资源。对端（从板 8788 服务）优先使用
`content_id` 走自己的云端凭据拉取 manifest；`mp3_url` / `mp4_url` 为兼容字段。

| 字段 | 类型 | 说明 |
|---|---|---|
| `type` | string | 固定 `"load"` |
| `content_id` | string | 云端内容 ID（推荐，从板据此自行拉取） |
| `mp3_url` | string | 音频 MP3 的 HTTP(S) 地址（兼容保留） |
| `mp4_url` | string | 视频 MP4 的 HTTP(S) 地址（兼容保留） |

```json
{"type":"load","content_id":"abc123","mp3_url":"http://.../audio","mp4_url":"http://.../video"}
```

对端处理流程：回 `ack` → 拉取并预缓冲 → 就绪后上报 `{"type":"state","state":"ready"}`。

### 4.1.1 播放控制命令（主 → 从）

| 消息 | 说明 |
|---|---|
| `{"type":"start"}` | 从 ready 状态起播（主板发出后双方同刻开始） |
| `{"type":"pause"}` | 暂停 |
| `{"type":"resume"}` | 继续 |
| `{"type":"stop"}` | 停止并释放会话 |

### 4.2 `ack` — 确认（应答）

对收到命令（如 `load`）的确认。

| 字段 | 类型 | 说明 |
|---|---|---|
| `type` | string | 固定 `"ack"` |
| `state` | string | 结果，如 `"accepted"`（已接受） |

```json
{"type":"ack","state":"accepted"}
```

### 4.3 `state` — 播放状态（上报 / 查询）

- **查询**：只带 `type`，请求对方上报当前状态。
- **上报**：携带当前播放状态。

| 字段 | 类型 | 说明 |
|---|---|---|
| `type` | string | 固定 `"state"` |
| `state` | string | 状态名，如 `"idle"` / `"downloading_audio"` / `"playing"` / `"paused"` / `"stopped"` |
| `position_ms` | number | 当前播放位置（毫秒），可选 |
| `duration_ms` | number | 总时长（毫秒），可选 |

```json
{"type":"state"}
```
```json
{"type":"state","state":"playing","position_ms":1234,"duration_ms":60000}
```

### 4.4 `status` — 状态查询（命令）

请求对端回报自身运行状态（资源、连接、版本等，由对端自行定义返回内容）。

```json
{"type":"status"}
```

### 4.5 `error` — 错误（上报）

| 字段 | 类型 | 说明 |
|---|---|---|
| `type` | string | 固定 `"error"` |
| `code` | string | 错误码，如 `"MP3_INVALID"` |
| `retryable` | boolean | 是否可重试 |
| `detail` | string | 人类可读的错误描述 |

```json
{"type":"error","code":"MP3_INVALID","retryable":false,"detail":"unsupported MP3 profile"}
```

> 未识别的 `type` 应被**忽略**（不要断开连接），以保证协议向前兼容。

---

## 5. 对端参考实现（Python TCP 服务器）

对端设备可直接参考 / 移植以下最小实现（逐行读入、按 `\n` 分帧、回写 NDJSON）：

```python
import json
import socket
import threading

HOST = "0.0.0.0"   # 监听所有网卡
PORT = 8787

def handle(conn, addr):
    print(f"[link] client connected: {addr}")
    buf = b""
    try:
        while True:
            data = conn.recv(4096)
            if not data:            # 对端关闭
                break
            buf += data
            while b"\n" in buf:     # 按 \n 分帧
                line, buf = buf.split(b"\n", 1)
                line = line.strip(b"\r")
                if not line:
                    continue
                try:
                    msg = json.loads(line.decode("utf-8"))
                except Exception:
                    continue        # 非法 JSON：忽略
                on_message(conn, msg)
    finally:
        conn.close()
        print(f"[link] client disconnected: {addr}")

def on_message(conn, msg):
    t = msg.get("type")
    if t == "load":
        print("[link] load:", msg.get("mp3_url"), msg.get("mp4_url"))
        send(conn, {"type": "ack", "state": "accepted"})
    elif t == "state":
        send(conn, {"type": "state", "state": "playing",
                    "position_ms": 0, "duration_ms": 0})
    elif t == "status":
        send(conn, {"type": "status", "ok": True})
    # 未知 type：忽略

def send(conn, obj):
    conn.sendall((json.dumps(obj, separators=(",", ":")) + "\n").encode("utf-8"))

def main():
    srv = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    srv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    srv.bind((HOST, PORT))
    srv.listen(5)
    print(f"[link] listening on {HOST}:{PORT}")
    while True:
        conn, addr = srv.accept()
        threading.Thread(target=handle, args=(conn, addr), daemon=True).start()

if __name__ == "__main__":
    main()
```

要点：
- 服务器应允许**重复连接**（本设备断开后可再次 Connect）。
- 发送时在 JSON 末尾追加 `\n`。
- JSON 建议用紧凑格式（无多余空格），减小帧长度。

---

## 6. 断线与重连

- 对端关闭连接（或网络中断）时，本设备会检测到并把状态置为 `Connect failed` / `Disconnected`。
- 本设备**不会自动重连**；需要时在 COMM 界面再次点击 **Connect**。
- 对端服务器应在客户端断开后清理该连接资源，并继续监听新的连接。

---

## 7. 用 COMM 界面快速自测

1. 在 PC 上运行第 5 节的 Python 服务器（PC 与设备同一 WiFi）。
2. 设备 COMM 界面输入 PC 的 IP、端口 `8787`，点 **Connect**（状态变绿 `Connected`）。
3. 点预设按钮 `status` / `state`（会自动填入输入框），再点 **Send**。
4. 服务器回应的消息会以 `>` 开头显示在日志区；本设备发出的以 `<` 开头。
5. 未连接时点 Send 会提示 `! not connected`。

---

## 8. 本设备侧实现位置（供固件开发参考）

| 文件 | 说明 |
|---|---|
| `source/embedded/src/tcp_link.c` / `include/tcp_link.h` | TCP 客户端：后台线程 connect + select/recv，按 `\n` 收帧入环形缓冲；`tcp_link_send()` 发送 |
| `source/embedded/src/ui_comm.c` | COMM 界面：本机 IP 显示、对端 IP/端口输入、Connect/Disconnect、预设消息、自由输入、收发日志 |

- 本设备发送：`tcp_link_send(text, len)` + 追加 `"\n"`。
- 本设备接收：`tcp_link_poll()` 每次取出一行（已去掉 `\n`）。
