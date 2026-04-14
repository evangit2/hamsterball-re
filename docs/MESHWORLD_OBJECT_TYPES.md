# MESHWORLD Format — Deep Specification

## Overview

The `.MESHWORLD` format is the core level/scene description format for Hamsterball. It contains:
1. 3D mesh geometry (vertices, faces, normals, UVs)
2. Object placement definitions with positions, rotations, and properties
3. Collision geometry references
4. Event trigger volumes
5. Visual decoration and lighting data

## Complete Object Type Taxonomy

Object types follow a `PREFIX:NAME(MODIFIER)` pattern:

### Prefix System
| Prefix | Category | Description |
|--------|----------|-------------|
| `START` | Player Start | Spawn positions for 1P and 2P |
| `FLAG` | Checkpoint | Race progress checkpoints |
| `SAFESPOT` | Safety | Landing/recovery spot |
| `CAMERALOOKAT` | Camera | Camera target position |
| `CAMERALOCUS` | Camera | Camera focus point |
| `E:` | Event (enter) | Triggered when ball enters volume |
| `N:` | Named Object | Interactive game object with collision |
| `O:` | Object | Static/moving geometry objects |
| `S:` | Structural | Non-shadow decorative geometry |
| `T:` | Texture/Decal | Non-colliding visual elements |
| `W:` | World | World boundary |

### Modifiers (appended in parentheses)
| Modifier | Effect |
|----------|--------|
| `(NOCOLLIDE)` | No collision with ball |
| `(NOSHADOW)` | Does not cast/receive shadows |
| `(WANTZ)` | Uses Z-buffer for depth test |
| `<TIME>NNN</TIME>` | XML-embedded parameter (duration in seconds) |

### E: Events (Ball-Enter Triggers)
| Event | Description |
|-------|-------------|
| E:ACTION | Generic action trigger |
| E:ACTIVATESAW1/2 | Activate sawblade 1 or 2 |
| E:ALERTJUDGES | Notify judge objects |
| E:ALERTSAW1/2 | Alert/activate saw 1 or 2 |
| E:BELL | Ring bell |
| E:BITE | Chomper bite |
| E:BRANCH(A)/(B) | Branch path A or B |
| E:CALLHAMMER | Summon hammer |
| E:CATAPULTBOTTOM | Catapult launch zone |
| E:DROPIN/01 | Drop ball into area |
| E:DROPLIFT | Drop lift platform |
| E:GRAVITY | Change gravity |
| E:GROWSOUND | Play grow sound |
| E:HAMMERCHASE | Start hammer chase |
| E:HEATOFF/HEATON | Heat hazard toggle |
| E:HELPINERTIA/UNHELPINERTIA | Assist/stop inertia |
| E:JUMP | Jump pad |
| E:LAUNCH | Launch ball |
| E:LIGHTSOFF/LIGHTSON | Neon lights toggle |
| E:LIMIT/LIMITX/LIMITZ/LIMITPIPE1/2 | Boundary limits |
| E:MACETRIGGER | Trigger mace swing |
| E:NODIZZY | Disable dizzy effect (with `<TIME>`) |
| E:NOPEGS | Remove pegs |
| E:OPENSESAME | Open door/gate |
| E:PEGS | Add pegs |
| E:PIPEBONK | Pipe collision (random sound from 3) |
| E:PIPERANDOM | Random pipe effect |
| E:POPOUT/01 | Pop ball out of area |
| E:SAFESWITCH(A-Z) | Safe zone switch (multiple variants) |
| E:SCORE1/5/7/9 | Score bonus (1/5/7/9 points) |
| E:SHRINK | Shrink ball |
| E:SWALLOW | Swallow/absorb ball |
| E:TRAJECTORY | Ball trajectory change |
| E:TRAPPOP | Trapdoor popup |
| E:VACPOPOUT | Vacuum popout |
| E:ZOOP | Quick movement effect |

### N: Named Objects (Interactive)
| Object | Description |
|--------|-------------|
| N:BOUNCE(NOSHADOW) | Bounce pad |
| N:BRIDGE | Destructible/movable bridge |
| N:BUMP | Bump obstacle |
| N:BUMPER1-8 | Bumper variants (1-8) |
| N:DROPIN | Drop-in zone |
| N:EXTRATIME | Time bonus pickup |
| N:GLASS | Glass/breakable platform |
| N:GOAL | Race goal finish line |
| N:JUMPFIRST/JUMPSECOND | Jump sequence triggers |
| N:MOUSETRAP | Mousetrap hazard |
| N:NEONPLATFORM | Neon-lighted platform |
| N:NOCONTROL | Disable player control |
| N:ONGEAR(NOSHADOW) | Ride on gear/rotator |
| N:ONPENDULUM | Ride on pendulum |
| N:ONROTATOR | Ride on rotating platform |
| N:SAWTEETH | Saw blade teeth |
| N:SECRET/SECRET(NOSHADOW) | Secret unlock spot |
| N:SINKPLATFORM | Sinking platform |
| N:SPEEDCYLINDER | Speed boost cylinder |
| N:SPINNY | Spinning obstacle |
| N:SQUAREWOBBLY | Square wobbling platform |
| N:SWIRL | Swirl/vortex |
| N:TARPIT | Tar pit (slow zone) |
| N:TENBONUS1/2 | 10-point bonus pickup |
| N:TRAPDOOR | Trapdoor (falls when stepped on) |
| N:UNLOCKSECRET | Unlock secret content |
| N:WATERWHEEL(NOSHADOW) | Water wheel mechanism |
| N:WAVY | Wavy water surface |
| N:WHEELEMBED | Embedded wheel/axle |
| N:LOOPER(NOSHADOW) | Loop-de-loop section |

### O: Object Types (Static/Moving Geometry)
| Object | Description |
|--------|-------------|
| O:EXITTOOB/01-06 | Tube exit points |
| O:HANDLE | Handle/grab point |
| O:SAW | Sawblade assembly |
| O:TOOB | Tube/tunnel section |
| O:TOOBY01-06 | Tube variants |

### S: Structural Types (Non-Shadow Decorative)
| Object | Description |
|--------|-------------|
| S:AXLE(NOSHADOW) | Axle/rotation pivot |
| S:BRICK/BRICKS(NOSHADOW) | Brick walls |
| S:BUGFIXPLANE(NOSHADOW) | Invisible collision fix plane |
| S:EnlargePipe(NOSHADOW) | Pipe widening section |
| S:PIPERIM(NOSHADOW) | Pipe rim geometry |
| S:PIPING(NOSHADOW) | Pipe sections |
| S:Pipes(NOSHADOW) | Multiple pipe geometry |
| S:RAILS(NOSHADOW) | Rail tracks |
| S:Rim(NOSHADOW) | Rim/border geometry |
| S:ShrinkPipe(NOSHADOW) | Pipe narrowing section |
| S:TUBE(NOSHADOW) | Tube geometry |
| S:WALLS(NOSHADOW) | Wall sections |

### T: Texture/Decal Types (Visual Only, No Collision)
| Type | Description |
|------|-------------|
| T:ARROW/01(NOCOLLIDE) | Direction arrows on floor |
| T:ARROWCURVE(NOCOLLIDE) | Curved direction arrows |
| T:BULLSEYE(NOCOLLIDE)(WANTZ) | Target bullseye pattern |
| T:FLOORLIGHTS | Floor lighting patterns |
| T:GOALAREA(NOCOLLIDE) | Goal zone visual overlay |
| T:GOSPOT(NOCOLLIDE) | Go spot indicator |
| T:GoalImage/GoalOverlay(NOCOLLIDE) | Goal visual effects |
| T:JUMPARROW(NOCOLLIDE) | Jump pad arrow indicator |
| T:LIGHT/01/LIGHTS | Light sources |
| T:Lighties | Small light decorations |
| T:NEON/08-13 | Neon light strips |
| T:NEONARROW(NOCOLLIDE) | Neon direction arrows |
| T:NEONRING/01/02 | Neon ring lights |
| T:NEONSTRIP | Neon strip light |
| T:RAMPARROW(NOCOLLIDE) | Ramp direction arrow |
| T:SIDEARROW(NOCOLLIDE) | Side wall direction arrow |
| T:SPEEDARROW | Speed boost arrow |
| T:START/STARTER/STARTSPOT(NOCOLLIDE) | Start position indicators |
| T:STARTSTOP(NOCOLLIDE) | Start/stop markers |
| T:TUBETOP/01/02/03 | Tube top openings |
| T:TunnelArrow(NOCOLLIDE) | Tunnel direction arrow |
| T:WARN/WARNING/Warning(NOCOLLIDE) | Warning signs |
| T:WARNINGDECAL(NOCOLLIDE) | Warning decal texture |
| T:YELLOWARROW(NOCOLLIDE) | Yellow direction arrow |
| T:ZIGZAG(NOCOLLIDE) | Zigzag pattern |

### Start Position Format
| Type | Description |
|------|-------------|
| START1-1 | Player 1 start |
| START2-1 | Player 2 start (2-player mode) |
| START2-2 | Player 2 alternate start |

### Checkpoint System
| Type | Description |
|------|-------------|
| FLAG02 | Checkpoint 2 |
| FLAG04 | Checkpoint 4 |
| FLAG06 | Checkpoint 6 |
| FLAG07 | Checkpoint 7 |

## Known Level Files

### Race Levels (17 levels)
| File | Race |
|------|------|
| Level1.MESHWORLD | Beginner Race |
| Level2.MESHWORLD | Cascade Race |
| Level3.MESHWORLD | Intermediate Race |
| Level4.MESHWORLD | Dizzy Race |
| Level5.MESHWORLD | Tower Race |
| Level8.MESHWORLD | Up Race |
| Level10.MESHWORLD | Neon Race |
| Level5-Bonk.MESHWORLD | Expert Race (bonk arena) |
| LevelImpossible.MESHWORLD | Impossible Race |

### Arena Levels (16 arenas)
| File | Arena |
|------|-------|
| Arena-Beginner.MESHWORLD | Beginner arena |
| Arena-Dizzy.MESHWORLD | Dizzy arena |
| Arena-Expert.MESHWORLD | Expert arena |
| Arena-Glass.MESHWORLD | Glass arena |
| Arena-Impossible.MESHWORLD | Impossible arena |
| Arena-Intermediate.MESHWORLD | Intermediate arena |
| Arena-Master.MESHWORLD | Master arena |
| Arena-Neon.MESHWORLD | Neon arena |
| Arena-Odd.MESHWORLD | Odd arena |
| Arena-Sky.MESHWORLD | Sky arena |
| Arena-SpawnPlatform.MESHWORLD | Spawn platform |
| Arena-Stands.MESHWORLD | Audience stands |
| Arena-Toob.MESHWORLD | Toob arena |
| Arena-Tower.MESHWORLD | Tower arena |
| Arena-Up.MESHWORLD | Up arena |
| Arena-WarmUp.MESHWORLD | Warm-up arena |
| Arena-Wobbly.MESHWORLD | Wobbly arena |

### Sub-Levels (Object Prefabs)
| File | Purpose |
|------|---------|
| Level3-Gluebie.MESHWORLD | Glue trap object |
| Level3-Swirl.MESHWORLD | Swirl vortex |
| Level3-Tipper.MESHWORLD | Tipping platform |
| Level3-WaterWheel.MESHWORLD | Water wheel |
| Level4-Catapult.MESHWORLD | Catapult launcher |
| Level4-Drawbridge.MESHWORLD | Drawbridge |
| Level4-Mace.MESHWORLD | Swinging mace |
| Level4-Trapdoor1/2.MESHWORLD | Trapdoor variants |
| Level4-Turret.MESHWORLD | Turret |
| Level4-Windmill.MESHWORLD | Windmill |
| Level5-Bridge.MESHWORLD | Collapsible bridge |
| Level2-Bridge.MESHWORLD | Bridge section |
| Level10-Bridge1/2.MESHWORLD | Bridge variants (Neon race) |
| Level7-Wobbly5.MESHWORLD | Wobbly platform |
| Level9-PopCylinder1/2.MESHWORLD | Pop cylinder |
| LevelUp-Lifter.MESHWORLD | Lifting platform |
| LevelUp-SpeedCylinder.MESHWORLD | Speed boost |
| LevelUp-Button.MESHWORLD | Button trigger |
| LevelImpossible-Rotator.MESHWORLD | Rotating obstacle |
| Secret.MESHWORLD | Secret area |
| Secret-Unlock.MESHWORLD | Unlock spot |
| PopupSign.MESHWORLD | Popup sign object |
| MouseTrap.MESHWORLD | Mousetrap object |
| Level6-Lifter.MESHWORLD | Level 6 lifter |

## Reimplementation Notes

### Level Loading
The MESHWORLD format is complex and tightly coupled to the Athena engine. For reimplementation:
1. Write a `meshworld2gltf` converter that extracts geometry + objects
2. Parse the binary header, iterate objects, extract type strings + positions
3. Store object definitions as a JSON sidecar file alongside the GLTF
4. Load the geometry in-engine, spawn objects from the JSON definitions
5. Each object type maps to a game entity class in the new engine