# Netplay Architecture Redesign v7

## Research Summary

Based on Glenn Fiedler's networking articles and Ruoyu Sun's "Game Networking Demystified":

### Current Problems
1. **Desync quickly** — Writing force accumulators (ball+0x2BC) which get consumed/zeroed each frame. Our writes are overwritten by the game's own input processing.
2. **Both players get keyboard/mouse input** — Game applies local input to BOTH P1 and P2. We need to suppress input for the remote player.
3. **No "in multiplayer match" detection** — Mod doesn't know if we're in a race or menu.
4. **Bad UX** — HOST/GUEST toggles instead of proper matchmaking.

### Correct Architecture: Distributed Authority + State Sync

From the research:
- **Distributed Authority** (like Dark Souls, Journey): Each client has authority over their own player. No dedicated server needed.
- **State Synchronization**: Send actual game state (position, velocity) not just inputs. Higher bandwidth but no desync from non-deterministic physics.
- **Interpolation**: Smooth remote player movement between received states.

### Implementation Plan

#### 1. Input Suppression (fix: both players get keyboard)
- **HOST**: Set P2's control slot (App+0xB28+1*4) to 99 (OFF) during races. This stops the game from reading local input for P2. Then we write P2's velocity from network.
- **GUEST**: Set P1's control slot (App+0xB28+0*4) to 99 during races. Local keyboard only controls P2 (which is the guest's local player).
- Restore control slots when leaving race.

#### 2. State Sync (fix: desync)
- **HOST**: Streams P1 position+velocity every frame. Receives P2 velocity from guest, writes to P2's physics object.
- **GUEST**: Streams P2 position+velocity every frame. Receives P1 state from host, writes to P1's ball directly (position + velocity).
- Use velocity (phys+0xCA4/CA8/CAC) which persists, not force accumulators which get consumed.
- Add interpolation for smooth remote movement.

#### 3. Match Detection (fix: no multiplayer state)
- Check if we're in a race by verifying board exists + P1 ball exists + not in loading screen.
- `g_inRace` flag set when board is valid and race countdown is over.
- Clear on scene_end / level_start.

#### 4. Matchmaking UX (fix: bad settings)
- Replace HOST/GUEST toggles with a single "Netplay: Connect" toggle.
- `netplay.txt` config: `host` or `guest` on first line, IP:port on second line.
- Relay handles matchmaking: host creates server, guest connects.
- No in-game menu changes needed — config file drives everything.

#### 5. Pause = Stop Input, Keep Game Live
- Already implemented (v6.3): intercept board+0x874, clear it, set g_localPaused.
- While paused: stop sending local input, keep receiving remote state.

### Data Flow

```
HOST (Client 1):
  Local: P1 (keyboard/mouse) → game physics → P1 position/velocity
  Network: P1 state → relay → guest
           Guest P2 input ← relay ← guest
  Apply: Write guest P2 velocity to P2's physics object

GUEST (Client 2):
  Local: P2 (keyboard/mouse) → game physics → P2 position/velocity
  Network: P2 state → relay → host
           Host P1 state ← relay ← host
  Apply: Write host P1 position+velocity to P1's ball
```

### Key Offsets
- App+0xB28 + playerIndex*4 = control slot (0-3=device, 99=OFF, 100=CPU)
- Ball+0x1A4 = PhysicsObject*
- Phys+0xCA4/CA8/CAC = velocity x/y/z (persists, not consumed)
- Ball+0x164/168/16C = position x/y/z
- Board+0x874 = pause flag
