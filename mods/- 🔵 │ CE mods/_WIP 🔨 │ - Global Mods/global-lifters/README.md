# Global Lifters Mod

Spawns Up Race lifters (Rotators) on any level with a hotkey.

## Usage

1. Load `GlobalLifters.CEA` in Cheat Engine
2. Enable the script
3. Set `SpawnLifter` to 1 in CE (or use a hotkey to toggle it)
4. The next time `Ball_Update` runs (every frame), a lifter spawns at the player's position

## How It Works

- Hooks `Ball_Update` at `0x00405E22` (inside the player physics update)
- When `SpawnLifter == 1`:
  - Saves player position from ball fields (+0x164/+0x168/+0x16C)
  - Creates a temporary `MeshWorld` from `"levels\levelup-lifter"` (the Up Race lifter mesh)
  - Stores it at `Board+0x4784` (the Up Race lifter mesh field)
  - Calls `CreateUpLevelObjects` (0x4117B0) to spawn a Rotator at the player position
  - Registers the Rotator in 4 lists:
    - `Board+0xCD4` (timer/cleanup list)
    - `Board+0x8AC+0x480+0x1C` (SceneObject render list)
    - `Board+0x10EC` (timer list)
    - `Board+0x8B0+0x18` (CollisionLevel collision list)
  - Calls `SceneObject_CallUpdate` (vtable[0x58]) and `SceneObject_CallRender` (vtable[0x54])

## v2 Fix — Dizzy Race Crash

**Crash address:** `0001:0001C5F0` (near-null access violation)

**Root cause:** Stack corruption from an unmatched `push ebx` before the `SceneObject_CallRender` call.

`SceneObject_CallRender` (0x45DF90) is a `__fastcall(ecx)` tail-call — it takes **zero** stack parameters and does not clean up any stack. The `push ebx` before the call left ESP off by 4 bytes. When `add esp, 68` executed, it only cleaned 68 of 72 bytes on the stack. `popad` then read from shifted positions, giving `ESI` the value of `EBP` instead of the ball pointer. The original instruction `mov eax, [esi+0x0C5C]` then accessed `[EBP + 0x0C5C]`.

On most levels, `EBP` happened to point to valid memory, so the corrupted read silently succeeded (wrong value, no crash). On Dizzy Race, `EBP` was a near-null pointer (`0x0001B994`), producing the access violation at `0x0001B994 + 0x0C5C = 0x0001C5F0`.

**Fix:** Removed the `push ebx` line. Stack is now perfectly balanced — `add esp, 68` exactly matches `sub esp, 68`, and `popad` restores all registers correctly.
