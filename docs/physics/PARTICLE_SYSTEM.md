# Particle & Trail System

## Overview
Hamsterball uses a simple particle system for trail effects behind the ball.
The only particle emitter is `Ball_CreateTrailParticles` (0x401DD0), which spawns
small `RumbleScore` objects in a circular pattern around the ball.

## Ball_CreateTrailParticles (0x401DD0)

Creates 9 trail particles arranged in a circle (iterates local_50 from 0 to 0x168
in steps of 0x28, giving 9 particles: 0x168/0x28 = 9).

### Algorithm
```
for each of 9 positions (angle_step = 0x28 in local_50):
  1. Get camera matrix from Graphics (via scene->graphics)
     cam_right = (fVar3..fVar5) from graphics->0x744+0x5c (3 floats)
     cam_up    = (fVar6..fVar8) from graphics->0x744+0x68 (3 floats)
  
  2. Calculate angle: sin(Wave_Sin(local_50)), cos(Wave_Cos(local_50))
  
  3. Offset = ball_radius * (sin*cos_right + cos*cos_up)
     offset_right = radius * cam_right * sin_angle
     offset_up    = radius * cam_up * cos_angle
  
  4. Create RumbleScore particle (0x28 bytes):
     RumbleScore_ctor(ball_scene)
     
  5. Particle position = ball_pos + offset_right - offset_up
     position[0] = offset_right.x + ball.pos.x - offset_up.x
     position[1] = offset_right.y + ball.pos.y - offset_up.y
     position[2] = offset_right.z + ball.pos.z - offset_up.z
  
  6. Particle velocity = offset_right - offset_up (normalized)
     velocity = (offset_right - offset_up) * (1.0 / (RNG_Rand(0x14) + 20))
     // _DAT_004cf310 = 1.0f division factor
  
  7. Append to scene->0x3b00 (particle list)
```

### App Struct Offsets
| Offset | Type | Description |
|--------|------|-------------|
| ball+0x14 | Scene* | Scene pointer |
| ball+0x164 | Vec3 | Ball position (x,y,z) |
| ball+0x284 | float | Ball radius (27.0f default) |
| scene+0x878 | void* | Graphics subsystem |
| scene+0x3b00 | AthenaList | Particle list head |
| graphics+0x744 | void* | Camera matrix base |

### RumbleScore Particle Struct (0x28 bytes)
Used for both RumbleBoard score popup effect and ball trail particles.

| Offset | Type | Field |
|--------|------|-------|
| 0x00 | vtable* | Virtual table pointer |
| 0x08 | float | position.x |
| 0x0C | float | position.y |
| 0x10 | float | position.z |
| 0x14 | float | velocity.x |
| 0x18 | float | velocity.y |
| 0x1C | float | velocity.z |
| ... | ... | Additional fields (lifetime, color, etc.) |

## WaterRipple System

Water ripples are a mesh deformation system, not a particle system.

### Functions
| Address | Name | Description |
|---------|------|-------------|
| 0x46A820 | WaterRipple_dtor | Destructor |
| 0x46A8A0 | WaterRipple_AllocBuffers | Allocate vertex buffers |
| 0x46A930 | WaterRipple_AdvancePhase | Advance ripple phase |
| 0x46A940 | WaterRipple_DeletingDtor | Deleting destructor |
| 0x46A960 | WaterRipple_UpdateVertices | Deform mesh vertices based on ripple |
| 0x46AF30 | WaterRipple_Ctor | Constructor |
| 0x46B070 | WaterRipple_Render | Render ripple effect |

### How Water Ripples Work
1. WaterRipple_Ctor initializes phase=0, allocates vertex buffer copies
2. Each frame: WaterRipple_AdvancePhase increments phase counter
3. WaterRipple_UpdateVertices applies concentric sine wave displacement to mesh Y coordinates
4. WaterRipple_Render draws the deformed water mesh with alpha blending
5. Ball_TestPlaneIntersection checks if ball is touching water surface