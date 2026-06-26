# Hamsterball Unreleased & Hidden Features

Reverse-engineered from Hamsterball.exe (V3.6.c, PE32 i386) via Ghidra decompilation and string analysis.

---

## 1. Party Race / Local 2-Player Split-Screen

**Status: Fully coded, menu-accessible in retail but possibly cut from some distributions**

A complete 2-player local mode exists in the binary:

- **Functions:** `App_Is2PMode` (0x00427910), `App_Start2PRace` (0x00429230), `PartyMenu_ctor` (0x0042fc10)
- **Strings:** `"PARTY GAMES!"`, `"CHOOSE A PARTY RACE!"`, `"PARTY RACE (2P ONLY)"`, `"THE PARTY RACE REQUIRES THAT PLAYER 1 AND PLAYER 2 BE HUMAN PLAYERS!"`
- **Controls:** `2PController1` through `2PController4` — 4 separate control profiles for local multiplayer
- **Level asset:** `Levels\\Level10-2PBridge` — a dedicated 2P bridge level for the Master race
- **Textures:** `partyrace.png`, `Winner2p.png`
- **Menu flag:** Sets `App+0x237=1` (is_2p_mode), `App+0x234=1`, and enables player slots at `+0x677`, `+0x717`, `+0x7b7`
- **Player 2 status:** `"PLAYER 2: %s"`, `"PLAYER 2: COMPUTER"`, `"PLAYER 2: OFF"` — P2 can be human, AI, or disabled

The 2P mode checks player slots (`+0xB2C`, `+0xB30`, `+0xB34` against value 100) to determine if players 3/4 are active.

---

## 2. Mirror Tournament

**Status: Unlockable in retail, but hidden behind win condition**

A mirrored (reversed) version of the tournament mode:

- **Strings:** `"MIRROR TOURNAMENT"`, `"MirrorTournament"`, `"THE MIRROR TOURNAMENT ISN'T UNLOCKED YET! TO UNLOCK THE MIRROR TOURNAMENT, YOU NEED TO WIN A TOURNAMENT AT NORMAL OR FRENZIED DIFFICULTY!"`
- **Mirror textures:** `arrow1-mirrored.png`, `goal-lit-mirrored.png`, `goal-mirrored.png`, `goal-round-lit-mirrored.png`, `goal-round-mirrored.png`, `mirror.png`, `sign-bewarethetar-mirrored.png`
- **Render functions:** `Level_RenderWithMirror4` (0x00413fc0), `Level_RenderWithMirror5` (0x004151e0) — render scenes with mirrored projection matrices (flips projection on one axis then restores)
- **Unlock check:** `TournamentManager` (0x00433ac0) checks for `"LOCKED"` string to show the unlock dialog

The mirror rendering works by adjusting projection matrix offsets (`+0x790`/`+0x794`) by a delta value, rendering 4 mirror planes, then restoring.

---

## 3. "NetworkConnection" — Actually an InputDevice (Not Networking)

**Status: Misleading name — this is an InputDevice class, NOT network multiplayer**

- **Function:** `NetworkConnection_Ctor` (0x0046dfa0) — base InputDevice constructor
- **Struct layout (20 bytes / 0x14):**
  - `+0x00`: char* name (`"Keyboard"`, `"Mouse/Trackball"`, `"Not Connected"`)
  - `+0x04`: InputHandler* parent
  - `+0x08`: int type (0=none, 1=keyboard, 2=mouse, 4-7=gamepad 1-4)
  - `+0x0C`: float sensitivity (1.0f default, 0.0 when not connected)
  - `+0x10`: void* device_data (keyboard buffer, joystick state, etc.)
- **Created in:** `App_Initialize_Full` — 4 instances at `App+0x550..0x55C`, then `InputDevice_SetType` configures each as keyboard/mouse/gamepad1/gamepad2
- **`"Not Connected"`** is just the default name when no gamepad/joystick is physically plugged in — NOT a network status

**WS2_32.dll imports (socket, connect, send, recv, etc.) exist in the binary but are used exclusively by:**
1. **eSellerate DRM** — HTTP connections to `store.esellerate.net` for activation/serial verification
2. **Raptisoft crash reporter** — sends error reports to Raptisoft's server

**There is no online multiplayer code in Hamsterball.** The `MPMenu` ("MP Menu" / `App_StartMPRace`) stands for Multiplayer **Party** (local), offering "PARTY RACE (2P ONLY)" and "RODENT RUMBLE (1-4P)" — both local modes.

---

## 4. Secret Objects & Arena Unlock System

**Status: Fully implemented, part of tournament progression**

Hidden collectibles in race levels that unlock arenas:

- **Function:** `CreateSecretObjects` (0x0040baa0) — factory for `SECRET` and `SECRETUNLOCK` objects
- **Types:**
  - `N:SECRET` — marks a secret spot found (calls `Rotator_MarkTriggered` at 0x004371f0, sets `+0x10E4=1`)
  - `N:UNLOCKSECRET` / `SECRETUNLOCK` — triggers arena unlock (calls `CheckArenaUnlock` at 0x0040aba0)
- **Level files:** `Levels\Secret`, `Levels\Secret-Unlock`
- **Unlock mechanism:** `CheckArenaUnlock` uses a switch on player profile index (`profile+8`, cases 4-15) to set unlock flags at offsets `+0x85A` through `+0x868` on the App struct
- **Unlock text:** `"THIS ARENA ISN'T UNLOCKED YET! TO UNLOCK %s ARENA, YOU NEED TO FIND THE SECRET UNLOCK SPOT IN THE %s RACE DURING A NORMAL OR FRENZIED TOURNAMENT GAME!"`
- **Gate:** Only works when `App+0x23C != 0` (not Pipsqueak difficulty) AND `App+0x234 == 0` (in tournament mode) AND profile `+0x11 == 0`

`Secret_ctor` (0x0043dfb0) inherits from `Stands` (static mesh object), allocates a CollisionLevel, and sets up timer-based update/render callbacks.

---

## 5. Tournament System (Full Campaign)

**Status: Fully implemented, retail feature**

A complete tournament mode with progression, saving, and multiple difficulty levels:

- **Manager:** `TournamentManager` (0x00433ac0) — handles menu routing: BACK → MainMenu, LOCKED → unlock dialog, 1PT → single player tournament
- **Save file:** `DATA\TOURNAMENT.SAV` / `DATA\tournament.sav`
- **Save/Load:** `TourneyMenu_LoadSaveAndShow` (0x004265a0), `TourneyMenu_WriteSave` (0x004264b0), `Tourney_SaveTournament` (0x00446730)
- **Continue dialog:** `TourneyContinueDialog_Ctor` (0x00445e60) — "CONTINUE TOURNAMENT?" prompt
- **Rollback:** `"YOU HAVE LOST THE TOURNAMENT. IN ORDER TO RESUME, YOU NEED TO SELECT 'ROLLBACK' ON THE MENU!"`
- **Difficulty:** Pipsqueak / Normal / Frenzied — affects time bonuses and unlock eligibility
- **Race unlock text:** `"THIS RACE ISN'T UNLOCKED YET! TO UNLOCK %s RACE, YOU NEED TO REACH IT WHILE PLAYING A NORMAL OR FRENZIED TOURNAMENT GAME!"`
- **Rankings:** Final ranking by score, with rank textures at `textures\ranks\%d.jpg`
- **Tournament textures:** Per-race preview images (`tourney-beginner.png` through `tourney-impossible.png`)
- **Music:** Tournament adjusts music tempo (`MusicPlayer_SetTempoScale` 1.0 for main, 0.5 for secondary)

---

## 6. Demo/Trial Limitations

**Status: Embedded DRM — retail had a demo version**

- **Free play counter:** `"You have %d free play remaining!"` — limited number of plays before purchase required
- **Demo end:** `"You have reached the end of the demo version of Hamsterball! But, if you buy now, you can continue, right here, right now! Or, click cancel to return to the main menu."`
- **Purchase prompts:** `"BUY HAMSTERBALL AND YOU CAN SAVE YOUR HIGH SCORES!"`, `"CLICK HERE TO BUY!"`, `"CLICK HERE TO REGISTER HAMSTERBALL!"`
- **Demo slowdown:** `"DEMO, THE SLOWER THE GAME"` — demo version intentionally slows gameplay
- **Tournament save lock:** `"DID YOU KNOW THAT IF YOU BUY HAMSTERBALL, YOU CAN CONTINUE YOUR TOURNAMENT LATER? YOU CAN BUY NOW, AND PICK UP WHERE YOU LEFT OFF NEXT TIME YOU PLAY!"`
- **High score lock:** Demo users cannot save high scores — `"BUY HAMSTERBALL AND YOU CAN SAVE YOUR HIGH SCORES!"`
- **Demo textures:** `demo.png`

---

## 7. eSellerate DRM System

**Status: Embedded third-party DRM (eSellerate by Esellerate)**

A full e-commerce/DRM system is embedded in the binary:

- **Activation:** `Activate`, `ActivateSerialNumber`, `Activate using another computer with web access`
- **Serial/Key:** `Enter Activation Key`, `Activation Key:`, `Activation URL:`, `ManualActivation.txt`
- **Manual activation:** URL `http://activate.esellerate.net`, `"Click the URL below to direct your web browser to the manual product activation web site"`
- **Server communication:** `cmd=DoHandshake&encryptedClientData=`, `cmd=GetServerPublicKey`
- **Encryption:** `"We encrypt any serial number transmissions and/or product downloads"`
- **Reinstall:** `"hold down the Alt key and click the Reinstall button"`
- **Purchase flow:** `Purchase`, `Software\eSellerate\Common\PurchaseInfo`, `Billing Information (Preview Mode)`
- **Coupons:** `COUPON`, `COUPONERRMSG`, `COUPONSAVINGS`, `COUPONTYPE`, `CROSSSELLGROUPDESC`

---

## 8. Raptisoft Crash Reporter / Bug Tracker

**Status: Fully implemented error reporting system**

An embedded crash reporter that sends data to Raptisoft:

- **Strings:** `"RaptisoftBugTracker"`, `"RaptisoftCrashWindow"`, `"Raptisoft Utility"`, `"*** BEGIN RAPTISOFT SESSION ***"`, `"*** END RAPTISOFT SESSION ***"`
- **Error report:** `"Sending Error Report..."`, `"Error report sent successfully!"`, `"Could not send error report:"`, `"Error: Could not contact Raptisoft reporting server!"`
- **User prompt:** `"%s has encountered an unexpected error! Pressing the 'Send Report' button will send this error report to Raptisoft via the web (an internet connection is required)."`
- **Report format:** XML-based — `<ERRORREPORT>`, `</ERRORREPORT>`, `<MODULE>`, `<description>`
- **Safe mode:** `"SAFE MODE: %s"` / `"SAFE MODE: OFF"` — `SAFEMODE` flag, `OptionsMenu_UpdateSafeModeText` (0x00442630)
- **Debug:** `ASTART-DEBUG` string, `DebugSetMute` (D3D8 function)

---

## 9. Auto-Update System

**Status: Fully implemented update checker**

- **Strings:** `"Check For Update"`, `"CheckForUpdate"`, `"Checking for new updater software"`, `"CHECKFORUPDATEMSGTEXT"`
- **Update URL pattern:** `?selector=UpdaterStub&vers=` — queries Raptisoft server for version updates
- **Mechanism:** Separate updater stub process (`UpdaterStub`)

---

## 10. Medal / Rank System (Gold/Silver/Bronze/Weasel)

**Status: Fully implemented — time-based ranking per level**

Each race track has medal time thresholds:

- **Tiers:** Gold, Silver, Bronze, and a special "Weasel" rank
- **Strings:** `"GOLD TIME:"`, `"SILVER TIME:"`, `"BRONZE TIME:"`, `"WEASEL'S TIME:"`, `"BEST RACE TIME:"`
- **Icons:** `gold-icon.png`, `silver-icon.png`, `bronze-icon.png`, `gold-small.png`, `silver-small.png`, `bronze-small.png`
- **Golden Weasel:** Special top rank — `goldenweasel-icon.png`, `goldenweasel.png`, `textures\ranks\weasel.png`, `weaselbox.png`
- **Rank textures:** `textures\ranks\%d.jpg` (numbered rank images)
- **Display:** `"YOUR RANK:"`, `"FINAL RANKING:"`, `"FINAL RANKING BY SCORE:"`, `"FINAL SCORE: %.0f"`
- **Registry:** `Medals` key

---

## 11. Bonus Systems

### 11a. Glass Break Bonus
- **Meshes:** `Meshes\GlassBonus`, `Meshes\GlassBonus-Smashed` — a glass panel that breaks
- **Sound:** `sounds\GlassBonus`, `sounds\bonuspop`
- **Description:** "IF YOU CAN MANAGE TO BREAK SOME GLASS HERE, YOU'LL GET A NICE TIME BONUS!"
- **Mechanism:** Glass level has breakable panels that grant time bonus when shattered

### 11b. Time Bonus Events
- **`N:TENBONUS1`** / **`N:TENBONUS2`** — bonus time triggers (10-second bonuses)
- **`N:EXTRATIME`** — extra time pickup
- **Strings:** `"Bonus +%d!"`, `"Bonus 20 - %d = %d"`, `"EXTRA TIME:"`, `"SUPER BONUS:"`, `"SURVIVAL BONUS:"`
- **Bell bonus:** "RING THE BELL ON THE BIG JUMP FOR AN EXTRA FIVE SECONDS!" (Expert race)

### 11c. Survival Bonus (Arena/Rumble)
- `"SURVIVAL BONUS:"` — points for surviving in Rodent Rumble arena mode

---

## 12. Hidden Game Object Types (N: Events)

All named object types found in the binary. Many are well-known, but some are more obscure:

| N: Tag | Level(s) | Description |
|---|---|---|
| `N:SECRET` | Secret levels | Secret collectible (marks triggered) |
| `N:UNLOCKSECRET` | Secret-Unlock | Arena unlock trigger |
| `N:TENBONUS1` | Various | 10-second time bonus |
| `N:TENBONUS2` | Various | Second 10-second time bonus type |
| `N:EXTRATIME` | Various | Extra time pickup |
| `N:GLASS` | Glass race | Glass breakable surface |
| `N:BOUNCE` | Impossible arena | Bounce pad |
| `N:JUMPFIRST` | Tower | First jump trigger |
| `N:JUMPSECOND` | Tower | Second jump trigger |
| `N:NOCONTROL` | Various | Disable player control (cutscene) |
| `N:ONGEAR` | Master | Gear riding surface |
| `N:ONROTATOR` | Various | Rotating platform surface |
| `N:SAWTEETH` | Expert | Saw blade hazard |
| `N:SPEEDCYLINDER` | Up | Speed boost cylinder |
| `N:SPINNY` | Expert | Spinning platform |
| `N:SQUAREWOBBLY` | Wobbly | Square wobbling platform |
| `N:TRAPDOOR` | Tower/Toob | Trapdoor (falls away) |
| `N:WHEELEMBED` | Various | Embedded wheel mechanism |
| `N:NEONPLATFORM` | Neon | Glowing platform |
| `N:WATERWHEEL` | Toob | Water wheel |
| `N:SWIRL` | Toob | Swirling water effect |
| `N:TARPIT` | Toob | Tar pit (slows ball) |
| `N:MOUSETRAP` | Intermediate | Mouse trap hazard |
| `N:WAVY` | Various | Wavy platform |
| `N:WATER` | Toob | Water surface |

### Hidden E: (Event) Triggers

Event triggers that fire gameplay effects. Some appear unused or very rare:

| E: Tag | Likely Purpose |
|---|---|
| `E:OPENSESAME` | Secret door opening (Sesame reference) |
| `E:SAFESWITCH` | Safety switch toggle |
| `E:VACPOPOUT` | Vacuum pop-out effect |
| `E:ZOOP` | Quick movement/teleport effect |
| `E:SHRINK` | Shrink ball (Odd race gravity mechanics) |
| `E:GROW` | Grow ball |
| `E:GROWSOUND` | Grow sound effect |
| `E:GRAVITY` | Gravity flip (Odd race) |
| `E:SWALLOW` | Swallow ball (despawn/kill) |
| `E:TRAJECTORY` | Set ball trajectory |
| `E:SHRINKCENTER` | Shrink toward center |
| `E:LIGHTSOFF` / `E:LIGHTSON` | Neon race lighting toggle |
| `E:HEATOFF` / `E:HEATON` | Heat effect toggle |
| `E:CATAPULTBOTTOM` | Catapult bottom trigger |
| `E:DROPLIFT` | Drop lift platform |
| `E:CALLHAMMER` | Summon BONK (hammer) |
| `E:HAMMERCHASE` | Activate BONK chase mode |
| `E:MACETRIGGER` | Mace swing trigger |
| `E:ALERTJUDGES` | Alert judge objects |
| `E:SAFESWITCH` | Safe mode switch |
| `E:PIPERANDOM` | Random pipe selection |
| `E:TRAPPOP` | Trap pop effect |
| `E:ACTIVATESAW1` / `E:ACTIVATESAW2` | Activate saw blades |
| `E:ALERTSAW1` / `E:ALERTSAW2` | Alert saw blades |
| `E:PEGS` / `E:NOPEGS` | Show/hide pegs |
| `E:NODIZZY` | Disable dizzy effect |
| `E:BRANCH` | Branch path selection |
| `E:BREAK` | Break object |
| `E:LIMIT` / `E:LIMITX` / `E:LIMITZ` / `E:LIMITPIPE1` / `E:LIMITPIPE2` | Movement limits |
| `E:HELPINERTIA` / `E:UNHELPINERTIA` | Assist/impede ball inertia |

---

## 13. BounceBall / FollowBall System

**Status: Fully implemented — appears in specific levels**

A ball-spawning system where certain objects spawn chasing balls:

- **`BounceBall_Update`** (0x00440840): Timer-based spawner. After a countdown (initial 120.0 seconds = `0x42F00000`), spawns a `FollowBall`:
  - Searches for `"BallPath"` object in the scene via `Level_FindObjectByName`
  - Reads `"FOLLOWBALLSPOT"` from hash table for spawn position
  - Creates FollowBall with `FollowBall_Ctor` (0x0043ebc0)
  - Sets ball velocity to `(-3.0, 10.0, 0)` — launches upward
  - Sets FollowBall state to `+0x80C = 0xF` (chase mode 15)
  - Adds to ball list (`board+0x29D4`)
  - Plays 3D spawn sound
- **`FollowBall_Update`** (0x0043ecc0): AI follow/chase logic for spawned ball

The BounceBall has two phases: active (counting down) and inactive (counting up). State stored at `+0x439` (timer) and `+0x43A` (phase flag).

---

## 14. Hamster Character Animations

**Status: Mesh assets referenced — may be unused or for menu/cutscene**

Hamster character model meshes beyond the ball:

- `Meshes\Hamster-Waiting` — idle/standing hamster
- `Meshes\Hamster-trot1` — trot animation frame 1
- `Meshes\Hamster-trot2` — trot animation frame 2
- `Meshes\Hamster-trot3` — trot animation frame 3
- `Meshes\FunBall` — unknown ball variant (possibly bouncy/fun mode)
- `Meshes\RBGlare` — ball glare effect
- `Meshes\YellowLink` — unknown (possibly chain/link visual)
- `Meshes\dawgshadow` / `dawgshoe` / `dawgshoe2` — Block Dawg (enemy) shadow and shoe meshes

---

## 15. CreateNoDizzy Object

**Status: Implemented but obscure**

- **Function:** `CreateNoDizzy` (0x0040c5d0) — creates an object that disables the dizzy/spin effect
- **Event:** `E:NODIZZY` — triggered when ball enters this zone
- Likely used on specific levels where the player would otherwise get dizzy (e.g., after spinning platforms)

---

## 16. Rodent Rumble (Arena) — Full 4-Player Mode

**Status: Retail feature, fully implemented**

Arena combat mode for 1-4 players:

- **Strings:** `"RODENT RUMBLE (1-4P)"`, `"THE RODENT RUMBLE REQUIRES AT LEAST TWO HUMAN OR COMPUTER PLAYERS!"`
- **13 arenas:** Warmup, Beginner, Intermediate, Dizzy, Tower, Up, Expert, Odd, Toob, Wobbly, Neon, Glass, Impossible, Master, Sky
- **AI players:** `"PLAYER 2: COMPUTER"` — CPU-controlled opponents
- **Scoring:** Survival bonus, knockoff count tracking
- **Pause:** `PauseRumbleMenu_ctor` (0x00430330) — `"Pause Rumble Menu"`
- **Special:** `ImpossibleArenaCollisionEvents` (0x00418600) has unique `N:BOUNCE` handling not in other arenas

---

## 17. Registry / Config Persistence

**Status: Retail — standard config system**

Registry keys for persistent game state:

- `CONTROL1` through `CONTROL4` — per-player control mappings
- `2PController1` through `2PController4` — 2P mode control mappings
- `BestTime` — per-level best times
- `Medals` — medal/rank achievement tracking
- `SafeMode` — safe rendering mode flag
- `MirrorTournament` — mirror tournament unlock flag
- `SAFEMODE` — safe mode toggle

---

## Summary Table

| Feature | Status | Notes |
|---|---|---|
| Party Race (2P split-screen) | ✅ Implemented | Requires 2 human players, dedicated level asset |
| Mirror Tournament | ✅ Unlockable | Win tournament at Normal/Frenzied |
| Online Multiplayer | ❌ Not present | NetworkConnection is InputDevice, WS2_32 is for DRM/crash reporter |
| Secret Collectibles | ✅ Implemented | Unlock arenas in tournament mode |
| Tournament Campaign | ✅ Retail | Full save/load, 3 difficulties, rollback |
| Demo/Trial Mode | ✅ Embedded | Free play counter, purchase prompts |
| eSellerate DRM | ✅ Embedded | Full activation/serial system |
| Crash Reporter | ✅ Implemented | Sends reports to Raptisoft server |
| Auto-Update | ✅ Implemented | UpdaterStub version check |
| Medal/Rank System | ✅ Retail | Gold/Silver/Bronze/Weasel per level |
| Glass Break Bonus | ✅ Implemented | Glass level time bonus |
| BounceBall/FollowBall | ✅ Implemented | Timed ball spawner with chase AI |
| Hamster Character Meshes | ⚠️ Referenced | Trot animation frames — may be unused |
| NoDizzy Zone | ✅ Implemented | Disables dizzy effect |
| Safe Mode | ✅ Retail | Graphics fallback mode |

---

*Document generated via Ghidra decompilation + string analysis of Hamsterball.exe V3.6.c (md5: 7d25019366b8d7f55906325bd630d7fe). All addresses are RVAs in the loaded image.*
