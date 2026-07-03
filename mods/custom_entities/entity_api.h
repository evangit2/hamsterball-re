/*
 * entity_api.h — Shared header for Hamsterball Custom Entities mod.
 *
 * Defines the interface that behavior DLLs in the Behaviours/ folder
 * must implement. Behavior DLLs are loaded by the main custom_entities
 * bass.dll proxy mod when it finds E:CustomName or N:CustomName entities
 * in a MESHWORLD file.
 *
 * The DLL filename must match the entity name (minus the E:/N: prefix).
 * For example, E:Rotator → Behaviours/Rotator.dll
 *
 * Behavior DLLs are regular C DLLs (NOT bass.dll proxies). Compile with:
 *   i686-w64-mingw32-gcc -shared -o Rotator.dll rotator.c -O2 -static -static-libgcc
 */

#ifndef ENTITY_API_H
#define ENTITY_API_H

#include <windows.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * EntityTransform — per-entity transformation data from MESHWORLD
 *
 * Located at MeshWorld+0x28 + meshBufferIndex * 0x50
 * Each entry is 0x50 bytes. Fields verified from Scene_LoadMeshWorld
 * (0x461890) decompilation.
 * ═══════════════════════════════════════════════════════════════════════════ */

#pragma pack(push, 1)
typedef struct {
    /* +0x00 */ DWORD  vtable;      /* Internal vtable pointer */
    /* +0x04 */ float  rotX;        /* Rotation X (radians) */
    /* +0x08 */ float  rotY;        /* Rotation Y (radians) */
    /* +0x0C */ float  rotZ;        /* Rotation Z (radians) */
    /* +0x10 */ float  rotScale;    /* Rotation scale (default 1.0) */
    /* +0x14 */ float  posX;        /* Position X */
    /* +0x18 */ float  posY;        /* Position Y */
    /* +0x1C */ float  posZ;        /* Position Z */
    /* +0x20 */ float  posScale;    /* Position scale (default 1.0) */
    /* +0x24 */ float  extra1[4];   /* Additional transform data */
    /* +0x34 */ float  extra2[4];   /* Additional transform data */
    /* +0x44 */ DWORD  flags;       /* Render flags */
    /* +0x48 */ DWORD  pad[2];      /* Padding to 0x50 */
} EntityTransform;
#pragma pack(pop)

/* Size check — must be exactly 0x50 bytes */
/* static_assert(sizeof(EntityTransform) == 0x50, "EntityTransform size mismatch"); */

/* ═══════════════════════════════════════════════════════════════════════════
 * Board info accessible to behavior DLLs
 *
 * The board pointer passed to behavior functions is the game's Board struct.
 * Key offsets:
 *   board+0x878  → Scene pointer
 *   board+0x29D4 → Ball list (AthenaList)
 *   board+0x2578 → Render object list (AthenaList)
 *
 * Ball struct (from ball list):
 *   ball+0x164 → Position X (float)
 *   ball+0x168 → Position Y (float)
 *   ball+0x16C → Position Z (float)
 *   ball+0x188 → Max speed (float)
 *   ball+0x18  → Player index (int, 0=P1, -1=AI)
 * ═══════════════════════════════════════════════════════════════════════════ */

/* ═══════════════════════════════════════════════════════════════════════════
 * Behavior DLL Interface
 *
 * Each behavior DLL must export these three functions:
 *
 * Behavior_Init:   Called once when the entity is first detected (level load)
 * Behavior_Update: Called every frame (~60fps) while the level is active
 * Behavior_Shutdown: Called when the level ends (board destroyed)
 *
 * Parameters:
 *   transform — Pointer to the entity's EntityTransform (modify rotX/rotY/rotZ
 *                to rotate, posX/posY/posZ to move)
 *   board     — Pointer to the game's Board struct (may be NULL during shutdown)
 * ═══════════════════════════════════════════════════════════════════════════ */

#define BEHAVIOR_API __declspec(dllexport)

#ifdef __cplusplus
extern "C" {
#endif

BEHAVIOR_API void __cdecl Behavior_Init(EntityTransform* transform, void* board);
BEHAVIOR_API void __cdecl Behavior_Update(EntityTransform* transform, void* board);
BEHAVIOR_API void __cdecl Behavior_Shutdown(void);

#ifdef __cplusplus
}
#endif

#endif /* ENTITY_API_H */
