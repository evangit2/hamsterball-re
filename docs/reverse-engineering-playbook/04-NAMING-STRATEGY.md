# 04 - Naming Strategy

## Naming Unknown Functions

| Pattern | Suggested Name |
|---------|---------------|
| Reads a known import | `ImportName_Wrapper` |
| Calls string search | `Subsystem_FindByName` |
| Manipulates a known struct | `StructName_Action` |
| Main loop | `Subsystem_Update` |
| Only sets a field | `Subsystem_SetField` |

## Be Conservative

If you only know a function calls `Direct3DCreate8`, name it `D3D8_CreateDevice_Wrapper`, not `Graphics_Initialize` unless you prove it initializes the whole subsystem.

## Anti-Pattern: Naming by Call Site

A function called from Ball-related code that actually dereferences CollisionMesh fields should be named `CollisionMesh_*`, not `Ball_*`. Check which struct offsets `this` accesses.
