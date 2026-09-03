# physics_objects v2 — Pushable + Tipping Boxes (Physicus Tier 2)

Part of **Hamsterball Physicus**. `CUBE:<meshname>` S1 refs spawn solid boxes that you can push, stand on, and now **tip off edges** instead of hanging in air.

## What changed from v1

v1 was axis-aligned sliding only — the box never rotated, so pushing it half-off a ledge looked wrong (it hovered). **v2 adds rigid-body tumble**:

- **Orientation is real**: each box stores a quaternion + angular velocity. Vertices are rotated + translated every frame (local→world rebuild), and the world matrix (`MW+0x04`) carries the same rotation so rendering matches collision.
- **Ground uses the level's own geometry**: v1's `raycast_down` against the cube's own mesh is replaced by `Mesh_FindClosestCollision (0x465D90)` against `scene+0x8B0` (the same query the ball uses). No approximation.
- **Support-polygon tipping**: 4 bottom-corner raycasts classify the contact as fully supported (≥3 corners), edge (1–2 corners), or airborne (0). Edge contact generates torque `r × F_gravity` (with box inertia `I = ⅓m(half² sums)`) so the box pivots around its support edge and falls. Airborne boxes seed a small tumble from their push velocity so they don't fall perfectly flat.
- **Stability**: 4 substeps/frame (dt=¼), angular damping (grounded vs air), velocity clamps, sleep thresholds, and an out-of-world reset at y<-800. Cost: ~16 raycasts/box/frame; at ≤16 boxes it's a few hundred µs.

## Placing objects

Add an S1 ref point named `CUBE:MyCube` — the engine loads `MyCube.MESH` via `MeshNode_ctor (0x471750)`. The `.MESH` file must be next to `Hamsterball.exe` (or in `levels/`). Up to 16 per level, any level.

## Push / carry / physics

- **Push**: ball AABB overlap beside the box accelerates it along the dominant X/Z axis (`PushForce` / `MaxPushVel`). Off-center pushes add angular kick (`wx/wz`) so a high push tips more.
- **Gravity / friction**: same config as v1 (`physics_objects.ini` auto-created). Angular damping tuned to settle on flats but tumble on edges.
- **Carrying**: standing on top moves the ball with the box (linear + vertical snap to `maxY+26`).
- **Reset**: boxes falling below -800 snap back to spawn and stop.

## Config (`physics_objects.ini` next to the DLL)

```
Gravity=0.55    # per-frame vertical accel
MaxFall=12.0
Friction=0.90   # ground linear decay
PushForce=0.30
MaxPushVel=2.20
```

Tipping uses internal `ANG_DAMP_GROUNDED=0.88`, `ANG_DAMP_AIR=0.995` and inertia from the mesh extents — no extra config needed.

## Safety

Mod-side state only (translation+rotation of our own `MeshWorld` verts). No level geometry or ball memory is written except the sanctioned `force`/`position` carries. Failure mode is visible jank (spin/jitter), not a native crash — and the usual 35s Wine crash-test gates every build.

## Files

- `bass.dll` — the proxy (rename to `bass.dll` in the game dir, original backed up as `bass_real.dll`)
- `physics_objects.log` — spawn + hook diagnostics

Built Aug 2026 — tested crash-free on Wine/Xvfb (38s). Visual tipping needs real Windows verification — the game renders black on llvmpipe.
