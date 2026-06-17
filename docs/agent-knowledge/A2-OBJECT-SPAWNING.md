# A2 - Object Spawning and Level Gating

See also `docs/LEVEL_LOCKED_OBJECTS.md`.

## Self-Loading Objects

Constructor calls `MeshWorld_ctor(..., "levels\\level5-bonk")` with a hardcoded path.

| Object | Constructor | File |
|--------|-------------|------|
| Bonk | 0x438850 | levels\level5-bonk |
| Bumper | 0x40FA20 | levels\level8 |
| Bumper2 | 0x413CE0 | levels\arena-beginner |

Enable on any level by patching `app+0x23C` to non-zero and adding the object name to the MESHWORLD file.

## Scene-Dependent Objects

Constructor takes a pre-loaded mesh pointer from the scene struct. The pointer is populated only by a specific `BoardLevel*_ctor`.

| Object | Constructor | Scene Offset | Loaded By |
|--------|-------------|-------------|-----------|
| Tipper | 0x437960 | scene+0x4394/0x4398 | BoardLevel3 |
| Catapult | 0x437E10 | scene+0x5848 | BoardLevel |
| Gluebie | 0x437CB0 | scene+0x607C | BoardLevel3 |
| BlockDawg | 0x43C310 | scene+0x5840/5844 | BoardLevel |
| BreakBridge | 0x436D70 | scene+0x5410/5414 | BoardLevel |
| PopCylinder | 0x436EE0 | scene+0x5420 | BoardLevel |

To add these to a level, both `app+0x23C` and the level constructor sub-mesh load must be patched.
