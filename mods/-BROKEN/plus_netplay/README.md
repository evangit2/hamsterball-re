# Netplay Mod (PoC v2)

**Status: PROOF OF CONCEPT — Polished framework, awaiting user verification.**

HB+ API mod for Hamsterball online multiplayer using host-authority architecture.

## What Changed in v2 (Polish Pass)

- **Fixed duplicate FPS calculation**: Single source of truth in `onGameUpdate`
- **Non-blocking pipe reads**: `PeekNamedPipe` + drain loop replaces blocking `ReadFile` that stalled the pipe thread
- **Pipe error recovery**: Auto-disconnect on broken pipe, retry with backoff
- **Host IP UI**: 4 octet sliders (0-255 each) replace hardcoded 127.0.0.1 — guest can target any IP
- **Proper velocity serialization**: Reads from `PhysicsObject+0xCA4/CAC` instead of ball facing direction
- **Full P2 input pipeline**: Now sends 3-axis force (X/Y/Z) from `ball+0x2BC/2C0/2C4`
- **HB+ API ApplyForce**: Host uses `g_api->ApplyForce()` instead of raw memory write (proper guards in Ball_ApplyForceV2)
- **Mutually exclusive roles**: HOST/GUEST can't both be active (internal state enforces this)
- **Data flow indicators**: On-screen display shows "LIVE"/"STALE" with frame age for both ball data (guest) and input (host)
- **Removed unused members**: `m_renderThread`, broken `onGameUpdate` FPS path
- **Struct field names**: `facing` renamed from misleading `vel`, actual velocity now separate field
- **Expanded BallStateMsg**: 3-axis velocity + 2-axis facing (was only 2-axis combined)

## Architecture

```
HOST (Client A)                          GUEST (Client B)
┌──────────────────────────┐            ┌──────────────────────────┐
│ Hamsterball 2P Party Mode│            │ Hamsterball (any mode)   │
│  └ plus_netplay.dll      │            │  └ plus_netplay.dll      │
│     streams ball state   │            │     injects ghost ball    │
│     ← guest input force  │            │     → sends P2 force     │
└─────────┬────────────────┘            └─────────▲────────────────┘
          │ named pipe                            │ named pipe
┌─────────▼────────────────┐            ┌─────────┴────────────────┐
│ netplay_relay.py (HOST)  │◄──TCP─────►│ netplay_relay.py (GUEST) │
│  - TCP server on port     │            │  - TCP client            │
│  - bridges pipe ↔ socket  │            │  - bridges pipe ↔ socket  │
└──────────────────────────┘            └──────────────────────────┘
```

## On-Screen Overlay

```
NETPLAY [HOST] Connected!  Port:5029          (green)
Local FPS:60.0  Remote FPS:59.8  Frame:1234    (yellow)
Remote: Guest: 192.168.1.50                    (blue)
Input: LIVE (last 2 frames ago)                (magenta)
```

## Installation

1. Copy `plus_netplay.dll` to HB+ mods folder
2. Run `python netplay_relay.py` BEFORE starting Hamsterball
3. In HB+ Options menu:
   - **HOST**: Toggle "HOST Mode" → relay opens TCP server
   - **GUEST**: Set 4 IP octet sliders, toggle "GUEST Mode" → relay connects to host

## Components

| File | Purpose |
|------|---------|
| `NetplayMod.cpp` | HB+ mod source (C++) |
| `HamsterballAPI.h` | HB+ API header |
| `plus_netplay.dll` | Compiled mod (MinGW i686, 2 imports) |
| `netplay_relay.py` | Python TCP relay process |

## Pipe Protocol

Binary, little-endian: `[DWORD type][DWORD length][body]`

| Type | Value | Direction | Body |
|------|-------|-----------|------|
| ROLE_SET | 5 | DLL→relay | DWORD role (0=off, 1=host, 2=guest) |
| PORT_SET | 6 | DLL→relay | DWORD port |
| HOST_IP | 8 | DLL→relay | char[] IP string |
| BALL_STATE | 2 | host→guest | BallStateMsg (76 bytes) |
| INPUT_STATE | 3 | guest→host | InputStateMsg (16 bytes) |
| FPS_REPORT | 4 | both | FpsReportMsg (12 bytes) |
| STATUS | 7 | relay→DLL | StatusMsg (72 bytes) |

## BallStateMsg Layout (76 bytes)

```
DWORD  frame
float  p1_pos[3]      // x, y, z (ball+0x164/168/16C)
float  p1_vel[3]      // actual velocity from PhysicsObject+0xCA4/CA8/CAC
float  p1_facing[2]   // facing vector (ball+0x190/194)
float  p1_rot        // roll angle (ball+0x150)
float  p1_radius      // ball+0x284
float  p1_gravity     // gravity plane (ball+0x748)
// P2 repeats same layout
```

## Current Status

- ✅ DLL compiles, loads, `CreateModInstance` verified (7/7 ad-hoc checks)
- ✅ Non-blocking pipe I/O with error recovery
- ✅ TCP networking (host server + guest client)
- ✅ HB+ menu UI (2 toggles + 5 sliders)
- ✅ On-screen overlay (role, FPS, connection, data flow)
- ✅ Full ball state serialization (pos + vel + facing + rot + radius + gravity)
- ✅ 3-axis force input pipeline with HB+ API ApplyForce
- ⚠️ Ghost ball injection (framework only — needs ghost_event-style Ball_ctor)
- ❌ Actual multiplayer testing (needs 2 clients + relay running)

## Technical Details

| Property | Value |
|----------|-------|
| Mod type | HB+ API (not bass.dll proxy) |
| Export | `CreateModInstance` → `NetplayMod` |
| Pipe name | `\\.\pipe\hamsterball_netplay` |
| Default port | 5029 |
| Pipe poll rate | ~250Hz (4ms sleep) |
| Heartbeat interval | 300 frames (~5s at 60fps) |
| Imports | KERNEL32.dll, msvcrt.dll only |
