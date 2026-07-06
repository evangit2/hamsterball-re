# Netplay Mod (PoC)

**Status: PROOF OF CONCEPT — untested in multiplayer. Awaits user verification.**

HB+ API mod for Hamsterball online multiplayer using host-authority architecture.

## Architecture

```
HOST (Client A)                          GUEST (Client B)
┌──────────────────────────┐            ┌──────────────────────────┐
│ Hamsterball 2P Party Mode│            │ Hamsterball (any mode)   │
│  └ plus_netplay.dll      │            │  └ plus_netplay.dll      │
│     streams ball state   │            │     injects ghost ball    │
│     ← guest input        │            │     → sends P2 input     │
└─────────┬────────────────┘            └─────────▲────────────────┘
          │ named pipe                            │ named pipe
┌─────────▼────────────────┐            ┌─────────┴────────────────┐
│ netplay_relay.py (HOST)  │◄──TCP─────►│ netplay_relay.py (GUEST) │
│  - TCP server on port     │            │  - TCP client            │
│  - bridges pipe ↔ socket  │            │  - bridges pipe ↔ socket  │
└──────────────────────────┘            └──────────────────────────┘
```

## Components

### 1. DLL Mod (`plus_netplay.dll`)
- HB+ API mod (exports `CreateModInstance`)
- UI: Toggle buttons for HOST/GUEST mode, port slider
- On-screen overlay: role, connection status, local/remote FPS
- Named pipe IPC to Python relay
- **HOST**: Reads P1+P2 ball state each frame, sends to relay
- **GUEST**: Sends P2 input force to relay, receives P1 ball state for ghost ball
- FPS measurement and comparison between clients

### 2. Python Relay (`netplay_relay.py`)
- Runs alongside Hamsterball on each client
- Creates named pipe server, waits for DLL to connect
- **HOST mode**: Opens TCP server on configured port, waits for guest
- **GUEST mode**: Connects to host's TCP server
- Bidirectional relay: pipe ↔ TCP for ball state and input
- Connection status reporting back to DLL

## Installation

1. Copy `plus_netplay.dll` to HB+ mods folder
2. Run `netplay_relay.py` BEFORE starting Hamsterball
3. In HB+ Options menu:
   - Toggle "HOST Mode" to host (opens TCP server)
   - OR toggle "GUEST Mode" to connect (connects to host IP)
   - Set port via "Netplay Port" slider

## Current Status (PoC)

- ✅ DLL compiles, loads, `CreateModInstance` works
- ✅ Named pipe IPC framework (DLL ↔ Python)
- ✅ TCP networking framework (Python ↔ Python)
- ✅ HB+ menu UI (toggle buttons, port slider)
- ✅ On-screen status display (role, connection, FPS)
- ✅ Ball state serialization (P1+P2 pos/vel/rot)
- ⚠️ Ghost ball injection (framework only — needs full ghost_event-style implementation)
- ⚠️ Input injection to P2 (basic force accumulator write — needs full ApplyForceV2)
- ❌ Actual multiplayer testing (needs 2 clients + relay running)

## Next Steps

1. **Test with 2 Hamsterball instances** on same machine (localhost)
2. **Implement ghost ball creation** (reuse ghost_event mod pattern: Ball_ctor + scene+0x361C)
3. **Implement proper input injection** (use Ball_ApplyForceV2 instead of raw force write)
4. **Add frame synchronization** (ensure both clients on same frame)
5. **Add position interpolation** (smooth ghost ball movement between updates)

## Files

| File | Purpose |
|------|---------|
| `NetplayMod.cpp` | HB+ mod source code |
| `HamsterballAPI.h` | HB+ API header (copy of docs/agent-knowledge/) |
| `plus_netplay.dll` | Compiled mod (MinGW i686) |
| `netplay_relay.py` | Python relay process |

## Technical Details

| Property | Value |
|----------|-------|
| Mod type | HB+ API (not bass.dll proxy) |
| Export | `CreateModInstance` → `NetplayMod` class |
| Pipe name | `\\.\pipe\hamsterball_netplay` |
| Default port | 5029 |
| Pipe protocol | Binary: [DWORD type][DWORD len][data] |
| TCP protocol | Binary: [uint16 total_len][DWORD type][DWORD data_len][data] |
| Ball state msg | 44 bytes: frame + P1(10 floats) + P2(10 floats) |
| Input state msg | 16 bytes: frame + dir_x + dir_y + force_mult |
| FPS report msg | 16 bytes: local_fps + remote_fps + frame_count |
| Pipe poll rate | ~120Hz (8ms sleep) |
| Heartbeat interval | 300 frames |
