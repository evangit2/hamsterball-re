# Discord Rich Presence Mod

A Hamsterball Plus API mod that displays your current game activity on your Discord profile using raw IPC — no external SDK dependencies.

## What It Shows

- **In a race**: Race name (e.g. "Dizzy Race"), race number (e.g. "Race 3/16"), elapsed timer
- **In arena mode**: "Arena - 2P" (player count)
- **In menu**: "In Menu"
- **Large image**: Hamsterball logo (configurable)

## Setup

### 1. Register a Discord Application

1. Go to https://discord.com/developers/applications
2. Click "New Application" → name it "Hamsterball"
3. Copy the **Application ID** (a number like `1234567890123456789`)
4. (Optional) Go to the "Rich Presence" → "Art Assets" tab and upload a `hamsterball` image (512×512 recommended)

### 2. Configure the Mod

A `discord_rpc.txt` file will be auto-generated next to the game exe on first run. Edit it:

```ini
# Discord Rich Presence Configuration
# Get your Application ID from https://discord.com/developers/applications
app_id=1234567890123456789
# Art asset key name uploaded to Discord Developer Portal
large_image=hamsterball
```

### 3. Install

1. Build the DLL in Visual Studio (using HB+ mod template)
2. Place `DiscordRichPresence.dll` in the game's `Mods\` folder
3. Make sure Discord desktop client is running
4. Launch Hamsterball

## How It Works

The mod communicates with the Discord desktop client via local named pipes (`\\.\pipe\discord-ipc-0` through `discord-ipc-9`). It sends JSON payloads using Discord's IPC protocol:

1. **Handshake** (opcode 1): `{"v":1,"client_id":"APP_ID"}`
2. **SET_ACTIVITY** (opcode 2): JSON with state, details, timestamps, and image assets

The mod runs a background thread that:
- Reads game state (level name, race index, player count) from HB+ API
- Detects state changes and sends presence updates
- Throttles updates to Discord's 15-second rate limit
- Auto-reconnects if Discord restarts
- Reads back Discord responses to detect disconnections

## Toggle

The mod adds a "Discord Rich Presence" toggle in the Options menu (YES/NO). Disabling it clears the presence and stops updates.

## Technical Details

- **No external dependencies**: Uses raw Win32 named pipes (`CreateFileA`) + JSON strings
- **No SDK**: Doesn't link against discord-rpc.lib or discord_partner_sdk.dll
- **Rate limited**: Updates at most every 15 seconds (Discord's limit)
- **Auto-reconnect**: Tries to reconnect every 15 seconds if Discord is closed
- **Graceful degradation**: If Discord isn't running, the mod silently does nothing (no crashes)

## Race Name Mapping

| Index | Race Name |
|-------|-----------|
| 0 | Warm-Up Race |
| 1 | Beginner Race |
| 2 | Intermediate Race |
| 3 | Dizzy Race |
| 4 | Tower Race |
| 5 | Up Race |
| 6 | Neon Race |
| 7 | Expert Race |
| 8 | Odd Race |
| 9 | Toob Race |
| 10 | Wobbly Race |
| 11 | Glass Race |
| 12 | Sky Race |
| 13 | Master Race |
| 14 | Impossible Race |
| 15 | Bug Race |

Race names are read from the game's internal table at `0x4F7080` (verified from binary). The mod uses its own hardcoded copy for simplicity.
