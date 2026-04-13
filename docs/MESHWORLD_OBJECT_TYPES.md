# Hamsterball - MESHWORLD Object Types

Objects in .meshworld level files use prefix markers:
- **N:** = Normal/Named objects (physical objects in the level)
- **E:** = Event/Trigger objects (cause effects when conditions met)
- **DN:** = Dynamic Named objects (animated/moving objects)

## Named Objects (N: prefix)

### Obstacles
| Type | Description |
|------|-------------|
| N:BUMPER1/2/3/4 | Bumpers that bounce the ball away |
| N:SPINNER | Spinning obstacle |
| N:SAWTEETH | Saw blade hazard |
| N:MACE | Swinging mace weapon |
| N:MOUSETRAP | Mouse trap hazard |
| N:WATER | Water zone |
| N:TARPIT | Tar/slow zone |
| N:NOCONTROL | Zone that removes ball control |

### Platform/Transport
| Type | Description |
|------|-------------|
| N:BRIDGE | Bridge platform |
| N:WATERWHEEL | Rotating water wheel |
| N:WHEELEMBED | Embedded wheel mechanism |
| N:SWIRL | Swirling vortex |
| N:WAVY | Wavy platform |
| N:SQUAREWOBBLY | Square wobbling platform |
| N:BOUNCE | Bounce pad |
| N:JUMPFIRST | First jump trigger |
| N:JUMPSECOND | Second jump trigger |
| DN:SINKPLATFORM | Sinking platform (dynamic) |
| N:NEONPLATFORM | Neon-lit platform |
| N:GLASS | Transparent glass platform |
| N:SPINNER | Rotating spinner obstacle |

### Bonus/Collectibles
| Type | Description |
|------|-------------|
| N:EXTRATIME | Extra time pickup |
| N:SPEEDCYLINDER | Speed boost cylinder |
| N:TENBONUS1 | 10-second bonus (variant 1) |
| N:TENBONUS2 | 10-second bonus (variant 2) |
| N:SECRET | Secret area trigger |
| N:UNLOCKSECRET | Unlock secret content |

### Physics Modifiers
| Type | Description |
|------|-------------|
| N:ONGEAR | Ball on gear surface |
| N:ONROTATOR | Ball on rotating surface |

## Event Objects (E: prefix)

### Physics Events
| Type | Description |
|------|-------------|
| E:GRAVITY | Change gravity direction |
| E:JUMP | Trigger jump |
| E:BREAK | Break/shatter object |
| E:LAUNCH | Launch ball in direction |
| E:TRAJECTORY | Ball trajectory change |
| E:ACTION | Generic action trigger |
| E:SWALLOW | Swallow/absorb ball |
| E:GROW | Make ball grow |
| E:SHRINK | Make ball shrink |
| E:GROWSOUND | Play grow sound effect |

### Boundary Events
| Type | Description |
|------|-------------|
| E:LIMIT | Level boundary |
| E:LIMITX | X-axis boundary |
| E:LIMITZ | Z-axis boundary |
| E:LIMITPIPE1 | Pipe boundary (variant 1) |
| E:LIMITPIPE2 | Pipe boundary (variant 2) |
| E:PIPERANDOM | Random pipe destination |

### Trigger Events
| Type | Description |
|------|-------------|
| E:SAFESWITCH | Safe toggle switch |
| E:NODIZZY | Prevent dizzy effect |
| E:OPENSESAM | Open gate/door |
| E:DROPLIFT | Drop elevator |
| E:CATAPULTBOTTOM | Catapult launch (bottom) |
| E:ACTIVATESAW1 | Activate saw 1 |
| E:ACTIVATESAW2 | Activate saw 2 |
| E:ALERTSAW1 | Alert saw 1 |
| E:ALERTSAW2 | Alert saw 2 |
| E:MACETRIGGER | Trigger mace swing |
| E:CALLHAMMER | Call hammer object |
| E:HAMMERCHASE | Hammer chase sequence |
| E:HEATON | Turn on heat |
| E:HEATOFF | Turn off heat |
| E:PEGS | Enable pegs |
| E:NOPEGS | Disable pegs |
| E:BRANCH | Branch path trigger |
| E:TRAPPOP | Pop trap door |
| E:BELL | Ring bell (goal/finish) |
| E:SCORE | Score event |
| E:ALERTJUDGES | Alert judges (scoring) |
| E:VACPOPOUT | Vacuum pop-out |
| E:HELPINERTIA | Help inertia (reduce drag) |
| E:UNHELPINERTIA | Un-help inertia (increase drag) |
| E:LIGHTSON | Turn lights on |
| E:LIGHTSOFF | Turn lights off |
| E:ZOOP | Fast travel effect |
| E:BITE | Bite/damage effect |

## Level Structure

### Board Types
| Board | Race Name |
|-------|----------|
| BoardLevel3 (0x41D060) | Dizzy Race |
| Board (Beginner) | Warm-Up Race |
| Board (Intermediate) | Intermediate Race |
| Board (Tower) | Tower Race |
| Board (Expert) | Expert Race |
| Board (Odd) | Odd Race |
| Board (Wobbly) | Wobbly Race |
| Board (Toob) | Toob Race |
| Board (Sky) | Sky Race |
| Board (Up) | Up Race |
| Board (Master) | Master Race |

### Rumble Boards (Arena Mode)
- RumbleBoard (Warmup Arena)
- RumbleBoard (Beginner Arena)
- RumbleBoard (Intermediate Arena)
- RumbleBoard (Dizzy Arena)
- RumbleBoard (Tower Arena)
- RumbleBoard (Up Arena)
- RumbleBoard (Expert Arena)

## Key Mesh Files
| Mesh | Purpose |
|------|---------|
| Meshes\\FunBall | Player ball model |
| Meshes\\8Ball | Enemy/chase ball model |
| Meshes\\mousetrapshadow | Mousetrap shadow |
| Levels\\MouseTrap | Mousetrap level data |
| Levels\\Level3-Tipper | Dizzy race level 1 |
| Levels\\Level3-WaterWheel | Dizzy race level 2 |
| Levels\\Level3-Swirl | Dizzy race level 3 |
| Levels\\Level3-Gluebie | Dizzy race level 4 |