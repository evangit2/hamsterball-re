# Netplay Mod — Full Installation Tutorial (Fresh Install)

This guide covers everything: installing HB+ from scratch, then adding the netplay mod.

---

## Part 1: Install Hamsterball Plus (HB+) Framework

### Step 1: Download HB+

1. Go to **https://github.com/artizard/Hamsterball-Plus/releases**
2. Download **`HamsterBallPlus.zip`** (this is the framework — not the mod template)
3. Also download **`HBModTemplate.zip`** only if you plan to compile mods yourself (you don't need this just to play)

### Step 2: Locate your Hamsterball folder

Typically one of:
- `C:\Program Files (x86)\Raptisoft\Hamsterball\`
- `C:\Users\<yourname>\Music\modded\` (if you have a custom install)
- Wherever you extracted your Hamsterball copy

You should see `Hamsterball.exe` and `bass.dll` in this folder.

### Step 3: Rename the original bass.dll

1. Find `bass.dll` in your Hamsterball folder
2. Right-click → **Rename** → change to `bass_real.dll`

> ⚠️ **This is critical.** HB+ works by replacing `bass.dll` with a proxy DLL that loads mods AND forwards audio calls to the real BASS library (now `bass_real.dll`). If you skip this, the game will have no audio and mods won't load.

### Step 4: Extract HB+ files

1. Open `HamsterBallPlus.zip`
2. Select all files and extract them into your Hamsterball folder
3. When prompted to overwrite/replace, click **Yes**

You should now have:
```
Hamsterball\
├── Hamsterball.exe
├── bass.dll          ← NEW (HB+ proxy, not the original)
├── bass_real.dll     ← renamed original
├── ModConfig.ini     ← NEW (HB+ config file)
├── Mods\             ← NEW (folder for mod DLLs)
│   └── (empty or example mods)
└── ... (other game files)
```

### Step 5: Set Run as Administrator

1. Right-click `Hamsterball.exe` → **Properties**
2. Go to the **Compatibility** tab
3. Check **"Run this program as an administrator"**
4. Click **OK**

> ⚠️ **Required.** HB+ needs admin to modify ModConfig.ini and load custom maps.

### Step 6: Verify HB+ is working

1. Launch Hamsterball
2. You should see a brief console window (or go to Options and see new menu items)
3. Check that audio works (if no audio, you forgot Step 3 — rename bass.dll to bass_real.dll)

### Step 7: Enable the console (optional, recommended for debugging)

1. Open `ModConfig.ini` in a text editor
2. Under `[Config]`, set `ShowConsole=1`
3. Save

---

## Part 2: Install the Netplay Mod

### Step 1: Get the mod files

You need two files:
- **`plus_netplay.dll`** — the HB+ mod (goes in the game's Mods folder)
- **`netplay_relay.py`** — the Python relay (runs alongside the game)

### Step 2: Install the DLL mod

1. Copy `plus_netplay.dll` into your Hamsterball `Mods\` folder
   ```
   Hamsterball\Mods\plus_netplay.dll
   ```
2. That's it — HB+ auto-discovers mods in the Mods folder on next launch

### Step 3: Install Python (if not already installed)

1. Go to **https://www.python.org/downloads/**
2. Download Python 3.10+ (3.12 recommended)
3. Run installer — **check "Add Python to PATH"** during install
4. Verify: open Command Prompt, type `python --version` — should print `Python 3.x.x`

### Step 4: Place the relay script

1. Copy `netplay_relay.py` to a convenient folder (e.g., your Desktop or a `netplay\` folder)
2. You'll run this script BEFORE launching Hamsterball

---

## Part 3: Using the Netplay Mod

### HOST (Player 1) Setup

1. **Start the relay first:**
   - Open Command Prompt
   - `cd` to wherever you put `netplay_relay.py`
   - Run: `python netplay_relay.py`
   - You should see: `Waiting for Hamsterball to start (pipe connection)...`

2. **Launch Hamsterball** (as administrator)

3. **Go to the Options menu**
   - You should see new toggle buttons:
     - **"Netplay: HOST Mode"** — toggle this **ON**
     - **"Netplay: GUEST Mode"** — leave OFF
   - You should see sliders:
     - **"Netplay Port"** — default 5029 (change if port is in use)
     - **"Host IP: Octet 1-4"** — ignore these (host mode)

4. **Check the on-screen overlay** (top-left corner):
   ```
   NETPLAY [HOST] Searching...  Port:5029
   Local FPS:0.0  Remote FPS:0.0  Frame:0
   ---
   ```
   The relay should print: `Guest connected from...` when someone joins

5. **Start a 2P Party Race** (Main Menu → Let's Play → Party → Party Race)
   - The host runs actual 2P physics and streams ball state to the guest

### GUEST (Player 2) Setup

1. **Start the relay first:**
   - Open Command Prompt
   - `cd` to wherever you put `netplay_relay.py`
   - Run: `python netplay_relay.py`

2. **Launch Hamsterball** (as administrator)

3. **Go to the Options menu**
   - Toggle **"Netplay: GUEST Mode"** ON
   - Set the **4 Host IP sliders** to match the host's IP address:
     - E.g., if host is at `192.168.1.50`:
       - Octet 1: `192`
       - Octet 2: `168`
       - Octet 3: `1`
       - Octet 4: `50`
   - Set **"Netplay Port"** to match host's port (default 5029)

4. **Check the overlay:**
   ```
   NETPLAY [GUEST] Connected!  Port:5029
   Local FPS:60.0  Remote FPS:59.8  Frame:42
   Host: 192.168.1.50:5029
   Ball data: LIVE (last 2 frames ago)
   ```

5. **Start any Time Trial race** (guest doesn't need Party Mode)
   - The guest's ball data (position, velocity, input force) streams to the host

### Same-Machine Testing (localhost)

To test on one PC with two game instances:

1. Copy your entire Hamsterball folder to a second location (e.g., `Hamsterball2\`)
2. Install HB+ in both folders (Steps 1-7 above)
3. Install `plus_netplay.dll` in both `Mods\` folders
4. Start **two** relay scripts (one per instance)
5. In Instance 1: HOST mode, default port 5029
6. In Instance 2: GUEST mode, IP `127.0.0.1` (default), port 5029
7. Both should connect within a few seconds

---

## Troubleshooting

### "Pipe error (relay not running?)"
- The Python relay isn't running or crashed
- Make sure you start `netplay_relay.py` **before** launching Hamsterball
- Check the relay's console for error messages

### No audio after installing HB+
- You forgot to rename `bass.dll` → `bass_real.dll` before extracting HB+
- Fix: rename the current `bass.dll` to something else, find your original `bass_real.dll`, rename it back to `bass.dll`, then redo Part 1

### Mod doesn't appear in Options menu
- Check that `plus_netplay.dll` is in the `Mods\` folder (not a subfolder)
- Set `ShowConsole=1` in `ModConfig.ini` and check the console for load errors
- Make sure `GetApiVersion()` returns `HAMSTERBALL_API_VERSION` (it does — compiled correctly)

### Guest can't connect to host
- Check both are on the same port (default 5029)
- Check host's firewall allows the port (Windows Firewall may block it)
- For LAN: ensure both PCs are on the same network
- For localhost testing: use IP `127.0.0.1`

### Overlay shows "STALE" data
- Network lag or relay is overloaded
- Check both clients' FPS — if one is <30fps, data may be delayed
- The overlay shows "LIVE" when data is <10 frames old

---

## File Summary

| File | Location | Purpose |
|------|----------|---------|
| `bass.dll` | Hamsterball\ | HB+ proxy (replaces original) |
| `bass_real.dll` | Hamsterball\ | Original BASS audio library |
| `ModConfig.ini` | Hamsterball\ | HB+ configuration |
| `plus_netplay.dll` | Hamsterball\Mods\ | Netplay mod |
| `netplay_relay.py` | Anywhere | Python TCP relay (run before game) |

## Quick Start Checklist

- [ ] Downloaded HamsterBallPlus.zip from GitHub releases
- [ ] Renamed original `bass.dll` → `bass_real.dll`
- [ ] Extracted HB+ files into Hamsterball folder
- [ ] Set Hamsterball.exe to Run as Administrator
- [ ] Copied `plus_netplay.dll` into `Mods\` folder
- [ ] Installed Python 3.10+ (added to PATH)
- [ ] Saved `netplay_relay.py` somewhere convenient
- [ ] Started relay script (`python netplay_relay.py`)
- [ ] Launched Hamsterball
- [ ] Toggled HOST or GUEST mode in Options
- [ ] Saw "Connected!" in the overlay
