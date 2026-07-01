"""Known Hamsterball memory addresses and offsets.

All absolute addresses assume image base 0x400000. Use RVA = addr - 0x400000
then resolve at runtime via module base + RVA for ASLR safety.

Addresses are verified against Ghidra decompilations and documented in
the hamsterball-re repository analysis docs.
"""
from __future__ import annotations

import re
from typing import Any, Optional

# Image base for Hamsterball.exe (PE32 i386, no ASLR)
IMAGE_BASE = 0x400000

# ---------------------------------------------------------------------
# Global pointers (absolute addresses)
# ---------------------------------------------------------------------
G_APP_PTR = 0x005341E0          # DAT_005341e0 -> App* (set by App_Ctor)
G_SCENE_PTR = 0x005341E4         # DAT_005341e4 -> Scene* (set by Scene_ctor) [unverified]
G_RENDERER_PTR = 0x005341E8      # [unverified]

# ---------------------------------------------------------------------
# App struct offsets (App is ~2328 bytes, created by App_Ctor @ 0x46DC40)
# ---------------------------------------------------------------------
APP = {
    "target_fps":       0x16C,   # u32 - target update rate (default 100)
    "render_fps":       0x170,   # u32 - target render rate (default 0x4B=75)
    "last_frame_tick":  0x164,   # u32 - GetTickCount() of last update
    "difficulty":       0x23C,   # u32 enum: 0=easy, 1=normal, 2=hard
    "player_score":     0x5E4,   # float[4] - per-player score (+playerIndex*0xA0)
    "control1":         0xB28,   # u32 - registry CONTROL1 key
    "control2":         0xB2C,   # u32 - registry CONTROL2 key
    "control3":         0xB30,   # u32 - registry CONTROL3 key
    "control4":         0xB34,   # u32 - registry CONTROL4 key
    "max_score":        0x5E0,   # [unverified]
    "is_paused":        0x1C,    # bool [unverified]
    "app_state":        0x20,    # u32 enum [unverified]
}

# ---------------------------------------------------------------------
# Ball struct offsets (Ball is ~3148 bytes, vtable @ 0x4CF3A0)
# Created by Ball_ctor @ 0x40AFE0
# ---------------------------------------------------------------------
BALL = {
    "vtable":           0x000,   # ptr -> 0x4CF3A0
    "position":         0x014,   # Vec3 (x, y, z) floats
    "velocity":         0x020,   # Vec3
    "radius":           0x03C,   # float
    "max_speed":        0x284,   # float (set by Ball_InitBattleMode, default 1000)
    "is_8ball":         0x31D,   # bool - true for AI 8-ball
    "is_active":        0x308,   # bool [unverified]
    "dizzy_immunity_timer":      0x2F4,   # i32 - best streak score (E:JUMP etc)
    "friction":         0xC6C,   # float (set by InitBattleMode, default 0.555)
    "speed_scale":      0xC68,   # float (set by InitBattleMode, default 0.0)
    "gravity":          0xC74,   # Vec3 (default (0, -1, 0) but scale=0 kills gravity)
    "team":             0x304,   # u32 [unverified]
    "color":            0x2FC,   # u32 ARGB [unverified]
}

# ---------------------------------------------------------------------
# Scene struct offsets (Scene is large, ~10600+ bytes)
# Created by Scene_ctor. Camera/level data embedded.
# ---------------------------------------------------------------------
SCENE = {
    "ball_list":        0x29D4,  # AthenaList embedded (not pointer)
    "camera_orbit_angle": 0x29BC, # float (mislabeled min_time in docs)
    "camera_orbit_dist": 0x29C0,  # float (mislabeled max_time in docs)
    "meshworld":        0x8AC,   # ptr -> MeshWorld*
    "collision_level":  0x8B0,   # ptr -> CollisionLevel* (vtable 0x4D9068)
    "current_level":    0x29B0,  # u32 - current level index [unverified]
    "game_mode":        0x29B4,  # u32 - race/arena [unverified]
    "ball_count":       0x29D0,  # u32 - number of balls in arena [unverified]
}

# ---------------------------------------------------------------------
# ArenaBoard struct offsets (arena mode, ~18380 bytes)
# ---------------------------------------------------------------------
ARENA_BOARD = {
    "knockoff_counts":  0x47B4,  # int32[4] - per-player fall count
    "knockoff_end":     0x47C0,  # end of array
    "board_state":      0x000,   # [unverified]
}

# ---------------------------------------------------------------------
# SceneObject struct offsets (~212 bytes, vtable @ 0x4D0260)
# ---------------------------------------------------------------------
SCENE_OBJECT = {
    "vtable":           0x000,
    "type":              0x004,  # i32 - object type enum
    "position":         0x008,  # Vec3
    "rotation":         0x014,   # Vec3 (euler angles)
    "scale":            0x020,   # Vec3
    "visible":          0x02C,   # bool [unverified]
}

# ---------------------------------------------------------------------
# Gadget struct offsets (~2156 bytes)
# ---------------------------------------------------------------------
GADGET = {
    "vtable":           0x000,
    "type":              0x004,  # i32 - gadget type enum
    "position":         0x008,   # Vec3
    "active":           0x020,   # bool
}

# ---------------------------------------------------------------------
# Function addresses (absolute, for hooking/calling)
# ---------------------------------------------------------------------
FUNCTIONS = {
    "WinMain":              0x4278E0,
    "GameLoop":             0x46BD80,
    "App_Ctor":             0x46DC40,
    "App_Run":              0x46BD80,
    "Scene_Ctor":           None,    # set by Scene module init
    "Scene_SetCamera":      0x419FA0,
    "Scene_UpdateBalls":    None,
    "Scene_Render":         None,
    "Ball_Ctor":            0x40AFE0,
    "Ball_InitPhysicsDefaults": 0x405100,
    "Ball_InitBattleMode":  None,
    "Ball_GetInputForce":   0x46EC30,
    "Ball_AdvancePosition": 0x403980,  # Ball_FindMeshCollision
    "Mesh_FindClosestCollision": 0x465D90,
    "operator_new":         0x4BA57B,
    "Input_IsKeyDown":      0x46E0B0,
    "InputDevice_PollAndRelease": 0x46EBD0,
    "InputDevice_SetType":  0x46DFC0,
    "Scene_HandleInput":    0x4692F0,
    "OptionsMenu_RenderControls": 0x42E910,
    "OptionsMenu_ctor":     0x442CE0,
    "Difficulty_GetTimeModifier": 0x428ED0,
    "Ball_DizzyImmunity":      0x402400,
    "DirectInput8Create":   0x47C7F0,
    "AABB_FromSphere":      0x477330,
}

# ---------------------------------------------------------------------
# Vtable addresses
# ---------------------------------------------------------------------
VTABLES = {
    "Ball":         0x4CF3A0,
    "Scene":        0x4D0260,
    "CollisionLevel": 0x4D9068,
}

# ---------------------------------------------------------------------
# Symbol registry for address resolution
# ---------------------------------------------------------------------
# Map symbol names to (base_symbol, offset) for chaining
# e.g. "g_App.target_fps" -> (g_App_ptr, APP["target_fps"])
_SYMBOL_MAP = {
    # Direct globals
    "g_App":            G_APP_PTR,
    "g_Scene":          G_SCENE_PTR,
    "g_App_ptr":        G_APP_PTR,
    "g_Scene_ptr":      G_SCENE_PTR,
    "g_Renderer":       G_RENDERER_PTR,
    # App fields
    "app.target_fps":      G_APP_PTR,
    "app.render_fps":      G_APP_PTR,
    "app.last_frame_tick": G_APP_PTR,
    "app.difficulty":      G_APP_PTR,
    "app.is_paused":       G_APP_PTR,
    "app.player_score":    G_APP_PTR,
}

# Map symbol suffixes to their offset within the parent struct
_SYMBOL_OFFSETS = {
    "target_fps":       APP["target_fps"],
    "render_fps":       APP["render_fps"],
    "last_frame_tick":  APP["last_frame_tick"],
    "difficulty":       APP["difficulty"],
    "is_paused":        APP["is_paused"],
    "player_score":     APP["player_score"],
    "control1":         APP["control1"],
    "control2":         APP["control2"],
    "control3":         APP["control3"],
    "control4":         APP["control4"],
    # Ball
    "ball.position":    BALL["position"],
    "ball.velocity":    BALL["velocity"],
    "ball.radius":      BALL["radius"],
    "ball.max_speed":   BALL["max_speed"],
    "ball.is_8ball":    BALL["is_8ball"],
    "ball.dizzy_immunity_timer": BALL["dizzy_immunity_timer"],
    "ball.friction":    BALL["friction"],
    "ball.is_active":   BALL["is_active"],
    # Scene
    "scene.camera_angle":  SCENE["camera_orbit_angle"],
    "scene.camera_dist":   SCENE["camera_orbit_dist"],
    "scene.meshworld":     SCENE["meshworld"],
    "scene.collision_level": SCENE["collision_level"],
    "scene.ball_list":     SCENE["ball_list"],
}


def resolve_symbol(mm, spec: str) -> Optional[int]:
    """Resolve a symbol specification to an absolute address.

    Args:
        mm: MemoryManager instance (for pointer dereferencing)
        spec: symbol specification, one of:
            "g_App"            -> absolute address of g_App pointer
            "g_App+0x16C"      -> g_App pointer address + offset
            "app.target_fps"   -> dereference g_App, add target_fps offset
            "ball.position"    -> requires context (which ball?) - see resolve_ball_field
            "func.Ball_Ctor"   -> absolute address from FUNCTIONS table
            "vtable.Ball"      -> absolute address from VTABLES table

    Returns:
        Absolute address, or None if unresolvable
    """
    spec = spec.strip()

    # Function addresses
    if spec.startswith("func."):
        name = spec[5:]
        addr = FUNCTIONS.get(name)
        return addr

    # Vtable addresses
    if spec.startswith("vtable."):
        name = spec[7:]
        return VTABLES.get(name)

    # App field: "app.xxx" -> dereference g_App ptr, add offset
    if spec.startswith("app."):
        field = spec[4:]
        offset = APP.get(field) or _SYMBOL_OFFSETS.get(f"app.{field}")
        if offset is None:
            return None
        app_ptr = mm.read_u32(G_APP_PTR)
        if not app_ptr:
            return None
        return app_ptr + offset

    # Ball field: "ball.xxx" -> needs ball ptr (caller must resolve first)
    if spec.startswith("ball."):
        field = spec[5:]
        offset = BALL.get(field) or _SYMBOL_OFFSETS.get(f"ball.{field}")
        if offset is None:
            return None
        # Can't resolve without ball pointer - return offset for caller
        return offset

    # Scene field: "scene.xxx" -> dereference g_Scene ptr, add offset
    if spec.startswith("scene."):
        field = spec[6:]
        offset = SCENE.get(field) or _SYMBOL_OFFSETS.get(f"scene.{field}")
        if offset is None:
            return None
        scene_ptr = mm.read_u32(G_SCENE_PTR)
        if not scene_ptr:
            return None
        return scene_ptr + offset

    # g_App / g_Scene with optional +offset
    base_match = re.match(r"^(g_App|g_Scene|g_App_ptr|g_Scene_ptr|g_Renderer)(?:\s*\+\s*(0x[0-9a-fA-F]+|\d+))?$", spec)
    if base_match:
        name = base_match.group(1)
        offset_str = base_match.group(2)
        base_addr = _SYMBOL_MAP.get(name)
        if base_addr is None:
            return None
        if offset_str:
            return base_addr + int(offset_str, 0)
        return base_addr

    # g_App.field style
    app_match = re.match(r"^g_App\.(\w+)$", spec)
    if app_match:
        field = app_match.group(1)
        offset = APP.get(field)
        if offset is None:
            return None
        app_ptr = mm.read_u32(G_APP_PTR)
        if not app_ptr:
            return None
        return app_ptr + offset

    return None


def get_struct_layout(struct_name: str) -> Optional[dict]:
    """Return the known offset map for a struct by name."""
    layouts = {
        "App": APP,
        "Ball": BALL,
        "Scene": SCENE,
        "ArenaBoard": ARENA_BOARD,
        "SceneObject": SCENE_OBJECT,
        "Gadget": GADGET,
    }
    return layouts.get(struct_name)


def list_known_symbols() -> dict[str, Any]:
    """Return a summary of all known symbols for documentation/discovery."""
    return {
        "globals": {
            "g_App": f"0x{G_APP_PTR:08X}",
            "g_Scene": f"0x{G_SCENE_PTR:08X}",
        },
        "structs": {
            "App": {k: f"0x{v:X}" for k, v in APP.items()},
            "Ball": {k: f"0x{v:X}" for k, v in BALL.items()},
            "Scene": {k: f"0x{v:X}" for k, v in SCENE.items()},
            "ArenaBoard": {k: f"0x{v:X}" for k, v in ARENA_BOARD.items()},
            "SceneObject": {k: f"0x{v:X}" for k, v in SCENE_OBJECT.items()},
            "Gadget": {k: f"0x{v:X}" for k, v in GADGET.items()},
        },
        "functions": {k: f"0x{v:08X}" for k, v in FUNCTIONS.items() if v is not None},
        "vtables": {k: f"0x{v:08X}" for k, v in VTABLES.items()},
        "image_base": f"0x{IMAGE_BASE:08X}",
    }
