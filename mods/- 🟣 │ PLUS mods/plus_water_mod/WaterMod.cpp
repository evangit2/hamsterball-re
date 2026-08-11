/*
 * WaterMod.cpp — Water Physics Mod for HB+ v2.1
 *
 * Port of hamsterball_water_mod v7.9 (bass.dll proxy) to HB+ API.
 *
 * v7.9 fixes:
 *   F1 — Hook 4 now patches the PLAYER ball vtable slot 8 @ 0x4CF334
 *     (base 0x4CF314, Ball_SplitDeath). Previously 0x4CF3A0 (BADBALL
 *     vtable) was hooked, so the player was never protected.
 *   F2 — g_states table: dead-slot recycling in get_ball_state +
 *     lookup-only peek_ball_state for Hook 4 (no more table exhaustion).
 *   F3 — prev_submersion=1.0 on water entry (no more entry-frame
 *     double-damp of Y velocity).
 *
 * Simplifications vs bass.dll version:
 * - Hook 1 (DispatchCollisionEvents trampoline) → onEventPlaneCollide callback
 * - Hook 2 (Phase 15 code cave + FPU save/restore) → onBallUpdate callback (no FPU issues!)
 * - BASS proxy shell removed entirely
 * - Config INI → HB+ sliders (adjustable in Options menu)
 *
 * Hook 3 (Type 5 suppressor) and Hook 4 (vtable[8] Ball_FallDeath) remain as
 * raw memory patches — they hook addresses inside Ball_Update that fire BEFORE
 * the onBallUpdate callback, so they can't be replaced by HB+ callbacks.
 *
 * Features (same as v7):
 * - E:WATER collision event triggers water entry (velocity damping + surface capture)
 * - Per-frame drag, horizontal drag, and buoyancy while submerged
 * - Dizzy immunity while submerged (clears bounce counter + sets immunity timer)
 * - Fall death suppression during water + 120-frame grace period after exit
 * - Type 5 mesh-penetration suppression while submerged/in grace
 *
 * Author: RodentRacer / Hamsterbot
 */
#include "HamsterballAPI.h"
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * Constants
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Ball struct offsets */
#define BALL_POS_X              0x164
#define BALL_POS_Y              0x168
#define BALL_POS_Z              0x16C
#define BALL_PHYS_PTR           0x1A4
#define BALL_RADIUS             0x284
#define BALL_FALLING_FLAG        0x2E9   /* BYTE: set by type 5 collision, E:LIMIT */
#define BALL_BOUNCE_COUNT       0x2EC   /* INT: dizzy bounce counter */
#define BALL_DIZZY_IMMUNITY     0x2F4   /* INT: dizzy immunity timer in frames */

/* Physics struct offsets */
#define PHYS_VEL_X              0xCA4
#define PHYS_VEL_Y              0xCA8
#define PHYS_VEL_Z              0xCAC

#define GRACE_PERIOD_FRAMES     120     /* ~5 seconds at 25fps */
#define MAX_BALLS               32

/* Hook 3: Type 5 collision suppressor — code cave at 0x407377 in Ball_Update
 * Prevents geometric mesh-penetration from setting 0x2E9 while submerged.
 * Needed because death check at 0x40721F runs BEFORE onBallUpdate. */
#define ADDR_TYPE5_JNZ          0x00407377
#define TYPE5_SKIP_TARGET       0x0040743D
#define TYPE5_NEXT_INSTR        0x0040737D
#define TYPE5_PATCH_SIZE        6

/* Hook 4: Ball vtable[8] — Ball_FallDeath suppression.
 * v7.9: Hook the PLAYER ball vtable (base 0x4CF314, slot 8 @ 0x4CF334 =
 * Ball_SplitDeath 0x409050). The player's death dispatch (CALL vtable[8]
 * at 0x406C76/0x40721F) reads THIS slot. The old value 0x4CF3A0 was the
 * BADBALL vtable (slot 8 @ 0x4CF3C0 = Ball_FallDeath 0x409480) — that
 * protected BadBalls from fall death but never the player. */
#define ADDR_BALL_VTABLE        0x004CF314
#define VTABLE_SLOT_FALLDEATH   8

/* ═══════════════════════════════════════════════════════════════════════════
 * Config (static globals, updated via HB+ sliders)
 * ═══════════════════════════════════════════════════════════════════════════ */

static float g_entry_damping     = 0.90f;
static float g_drag              = 0.02f;
static float g_horizontal_drag   = 0.04f;
static float g_buoyancy_strength = 1.0f;
static bool  g_enabled           = true;

/* ═══════════════════════════════════════════════════════════════════════════
 * Per-ball water state (static — accessible from code cave + callbacks)
 * ═══════════════════════════════════════════════════════════════════════════ */

struct WaterState {
    DWORD ball;              /* ball pointer (key); 0 = unused */
    int   in_water;          /* currently in water? */
    float water_surface_y;   /* ball's Y at moment of contact */
    int   grace_frames;      /* death suppression frames after leaving water */
    float prev_submersion;   /* last frame's submersion (surface crossing) */
};

static WaterState g_states[MAX_BALLS];

static WaterState* get_ball_state(DWORD ball) {
    int free_idx = -1;
    for (int i = 0; i < MAX_BALLS; i++) {
        if (g_states[i].ball == ball) return &g_states[i];
        if (free_idx == -1 && g_states[i].ball == 0) free_idx = i;
    }
    /* v7.9: recycle dead slots before allocating. A slot whose ball is
     * neither in water nor in grace is stale (ball fell to death, was
     * respawned, or left the level) — its pointer key is dead. Reuse it
     * instead of letting the table fill up with corpses. */
    if (free_idx == -1) {
        for (int i = 0; i < MAX_BALLS; i++) {
            if (g_states[i].in_water == 0 && g_states[i].grace_frames == 0) {
                free_idx = i;
                break;
            }
        }
    }
    if (free_idx >= 0) {
        memset(&g_states[free_idx], 0, sizeof(WaterState));
        g_states[free_idx].ball = ball;
        return &g_states[free_idx];
    }
    return nullptr;
}

/* Lookup-only getter — never allocates a slot. Used by Hook 4
 * (fall-death suppression) where the ball may have no water state at
 * all; allocating a slot there would leak table entries for every ball
 * that ever falls to death outside water. */
static WaterState* peek_ball_state(DWORD ball) {
    if (!ball) return nullptr;
    for (int i = 0; i < MAX_BALLS; i++) {
        if (g_states[i].ball == ball) return &g_states[i];
    }
    return nullptr;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Water physics (same logic as bass.dll v7)
 * ═══════════════════════════════════════════════════════════════════════════ */

static float compute_submersion(float ball_y, float radius, float surface_y) {
    if (radius <= 0.0f) return 0.0f;
    float sub = (surface_y - ball_y + radius) / (2.0f * radius);
    if (sub < 0.0f) sub = 0.0f;
    if (sub > 1.0f) sub = 1.0f;
    return sub;
}

/* Trigger: called from onEventPlaneCollide when ball touches E:WATER */
static void trigger_water_contact(DWORD ball) {
    if (!ball || IsBadReadPtr((void*)ball, 0x300)) return;

    float ball_y = *(float*)(ball + BALL_POS_Y);
    float radius = *(float*)(ball + BALL_RADIUS);
    if (radius <= 0.0f || radius > 1000.0f) return;

    DWORD phys = *(DWORD*)(ball + BALL_PHYS_PTR);
    if (!phys || IsBadReadPtr((void*)phys, 0xCB0)) return;

    WaterState* st = get_ball_state(ball);
    if (!st) return;

    /* Only trigger if not already in water */
    if (st->in_water) return;

    /* Set in_water flag */
    st->in_water = 1;

    /* Clear falling flag (set during long fall before reaching water) */
    *(BYTE*)(ball + BALL_FALLING_FLAG) = 0;

    /* Reset bounce counter (same as E:NODIZZY) */
    *(int*)(ball + BALL_BOUNCE_COUNT) = 0;

    /* Damp velocity on entry */
    float damp = g_entry_damping;
    *(float*)(phys + PHYS_VEL_X) *= damp;
    *(float*)(phys + PHYS_VEL_Y) *= damp;
    *(float*)(phys + PHYS_VEL_Z) *= damp;

    /* Capture water surface Y */
    st->water_surface_y = ball_y;

    /* v7.9 (F3): Mark the ball as already-submerged this entry frame.
     * Without this, prev_submersion defaults to 0.0, so the surface
     * crossing check in apply_water_physics fires on the SAME frame
     * (submersion >= 0.5) and damps Y velocity a second time —
     * effective damping becomes entry_damping^2 (e.g. 0.9*0.9 = 0.81)
     * on the entry frame. Setting prev_submersion = 1.0 makes the
     * crossing check see "already submerged" and skip the re-damp;
     * the entry damping above is the ONLY damping applied on entry. */
    st->prev_submersion = 1.0f;
}

/* Per-frame physics: called from onBallUpdate */
static void apply_water_physics(DWORD ball) {
    if (!ball || IsBadReadPtr((void*)ball, 0x300)) return;

    WaterState* st = get_ball_state(ball);
    if (!st) return;

    /* Decrement grace timer every frame */
    if (st->grace_frames > 0) st->grace_frames--;

    /* Clear 0x2E9 and grant dizzy immunity while submerged */
    if (st->in_water) {
        *(BYTE*)(ball + BALL_FALLING_FLAG) = 0;
        *(int*)(ball + BALL_BOUNCE_COUNT) = 0;
        int cur = *(int*)(ball + BALL_DIZZY_IMMUNITY);
        if (cur < GRACE_PERIOD_FRAMES)
            *(int*)(ball + BALL_DIZZY_IMMUNITY) = GRACE_PERIOD_FRAMES;
    }

    if (!st->in_water) return;

    float ball_y = *(float*)(ball + BALL_POS_Y);
    float radius = *(float*)(ball + BALL_RADIUS);
    if (radius <= 0.0f || radius > 1000.0f) return;

    /* Exit condition: ball clearly above surface */
    if (ball_y - radius > st->water_surface_y + radius * 0.5f) {
        st->in_water = 0;
        st->water_surface_y = 0.0f;
        st->prev_submersion = 0.0f;
        st->grace_frames = GRACE_PERIOD_FRAMES;
        return;
    }

    DWORD phys = *(DWORD*)(ball + BALL_PHYS_PTR);
    if (!phys || IsBadReadPtr((void*)phys, 0xCB0)) return;

    float submersion = compute_submersion(ball_y, radius, st->water_surface_y);

    /* Surface crossing detection: apply entry damping on re-entry */
    if (st->prev_submersion < 0.5f && submersion >= 0.5f) {
        *(float*)(phys + PHYS_VEL_Y) *= g_entry_damping;
    }
    st->prev_submersion = submersion;

    /* Apply drag */
    float vscale = 1.0f - g_drag;
    float hscale = 1.0f - (g_drag + g_horizontal_drag);
    if (vscale < 0.0f) vscale = 0.0f;
    if (hscale < 0.0f) hscale = 0.0f;

    *(float*)(phys + PHYS_VEL_X) *= hscale;
    *(float*)(phys + PHYS_VEL_Z) *= hscale;
    *(float*)(phys + PHYS_VEL_Y) *= vscale;

    /* Buoyancy = strength * submersion (0-1) */
    float buoyancy = g_buoyancy_strength * submersion;
    *(float*)(phys + PHYS_VEL_Y) += buoyancy;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Hook 3: Type 5 collision suppressor (code cave)
 *
 * Code cave at 0x407377 (JNZ in Ball_Update's type 5 block).
 * When ball is in water or grace period, skips the entire type 5 death block
 * (sets 0x2E9 + camera switch). This is needed because the death check at
 * 0x40721F runs BEFORE onBallUpdate, so clearing 0x2E9 in the callback is
 * too late to prevent the death check from firing.
 *
 * E:LIMIT events still set 0x2E9 through DispatchCollisionEvents, so the
 * ball can still die from level boundaries while in water.
 * ═══════════════════════════════════════════════════════════════════════════ */

static int __cdecl is_ball_in_water(DWORD ball) {
    if (!ball || IsBadReadPtr((void*)ball, 0x300)) return 0;
    WaterState* st = get_ball_state(ball);
    return (st && (st->in_water || st->grace_frames > 0)) ? 1 : 0;
}

static int (__cdecl *g_is_in_water_ptr)(DWORD) = NULL;
static BYTE* g_type5_cave = NULL;

static void install_type5_hook() {
    BYTE* hook_addr = (BYTE*)ADDR_TYPE5_JNZ;

    /* Verify expected bytes: 0F 85 C0 00 00 00 (JNZ rel32) */
    BYTE expected[] = { 0x0F, 0x85, 0xC0, 0x00, 0x00, 0x00 };
    if (memcmp(hook_addr, expected, TYPE5_PATCH_SIZE) != 0) return;

    g_type5_cave = (BYTE*)VirtualAlloc(NULL, 256, MEM_COMMIT | MEM_RESERVE,
                                         PAGE_EXECUTE_READWRITE);
    if (!g_type5_cave) return;

    g_is_in_water_ptr = is_ball_in_water;

    int p = 0;

    /* PUSHFD — save original flags (from penetration check) */
    g_type5_cave[p++] = 0x9C;
    /* PUSH ESI — ball pointer, becomes __cdecl argument */
    g_type5_cave[p++] = 0x56;
    /* CALL [g_is_in_water_ptr] */
    g_type5_cave[p++] = 0xFF; g_type5_cave[p++] = 0x15;
    *(DWORD*)(g_type5_cave + p) = (DWORD)&g_is_in_water_ptr; p += 4;
    /* ADD ESP, 4 — clean up __cdecl arg */
    g_type5_cave[p++] = 0x83; g_type5_cave[p++] = 0xC4; g_type5_cave[p++] = 0x04;
    /* TEST EAX, EAX — check return value */
    g_type5_cave[p++] = 0x85; g_type5_cave[p++] = 0xC0;
    /* JNZ to in-water cleanup (placeholder, patched later) */
    g_type5_cave[p++] = 0x0F; g_type5_cave[p++] = 0x85;
    int jnz_inwater_offset = p;
    p += 4;

    /* --- Not in water path: --- */
    /* POPFD — restore original flags */
    g_type5_cave[p++] = 0x9D;
    /* JNZ 0x40743D — reproduce original conditional jump */
    g_type5_cave[p++] = 0x0F; g_type5_cave[p++] = 0x85;
    {
        DWORD src = (DWORD)(g_type5_cave + p + 4);
        *(DWORD*)(g_type5_cave + p) = (DWORD)TYPE5_SKIP_TARGET - src;
    }
    p += 4;
    /* Fall through: penetration > 1.0 and not in water → continue to 0x40737D */
    g_type5_cave[p++] = 0xE9;
    *(DWORD*)(g_type5_cave + p) = (DWORD)TYPE5_NEXT_INSTR - (DWORD)(g_type5_cave + p + 4);
    p += 4;

    /* --- In water cleanup path: --- */
    int inwater_cleanup_addr = p;
    /* ADD ESP, 4 — remove saved EFLAGS */
    g_type5_cave[p++] = 0x83; g_type5_cave[p++] = 0xC4; g_type5_cave[p++] = 0x04;
    /* JMP to TYPE5_SKIP_TARGET — skip entire type 5 death block */
    g_type5_cave[p++] = 0xE9;
    {
        DWORD src = (DWORD)(g_type5_cave + p + 4);
        *(DWORD*)(g_type5_cave + p) = (DWORD)TYPE5_SKIP_TARGET - src;
    }
    p += 4;

    /* Patch the in-water JNZ to target the cleanup path */
    {
        DWORD src = (DWORD)(g_type5_cave + jnz_inwater_offset + 4);
        DWORD dst = (DWORD)(g_type5_cave + inwater_cleanup_addr);
        *(DWORD*)(g_type5_cave + jnz_inwater_offset) = dst - src;
    }

    /* Patch hook site: 5-byte JMP + 1 NOP */
    DWORD old_protect;
    VirtualProtect(hook_addr, TYPE5_PATCH_SIZE, PAGE_EXECUTE_READWRITE, &old_protect);
    *(DWORD*)(hook_addr + 1) = (DWORD)g_type5_cave - (DWORD)hook_addr - 5;
    hook_addr[0] = 0xE9;
    hook_addr[5] = 0x90;
    VirtualProtect(hook_addr, TYPE5_PATCH_SIZE, old_protect, &old_protect);
    FlushInstructionCache(GetCurrentProcess(), hook_addr, TYPE5_PATCH_SIZE);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Hook 4: Ball vtable[8] — Ball_FallDeath suppression
 *
 * When Ball_FallDeath is called, checks if ball is in water OR within the
 * grace period after leaving water. If so, skips death entirely.
 * Covers the bounce-out scenario: ball exits water, flies through the air,
 * and the velocity-stop death check fires at the apex.
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef void (__thiscall *ball_falldeath_t)(void *ball);
static ball_falldeath_t orig_Ball_FallDeath = NULL;

static void __thiscall Hook_Ball_FallDeath(void *ball) {
    if (ball) {
        /* Lookup only — do NOT allocate a state slot here (v7.9).
         * Every ball that falls to death outside water would otherwise
         * leak a table entry and eventually exhaust the 32-slot table,
         * silently killing water physics for the whole session. */
        WaterState* st = peek_ball_state((DWORD)ball);
        if (st && (st->in_water || st->grace_frames > 0)) {
            return;  /* suppress death */
        }
    }
    if (orig_Ball_FallDeath) orig_Ball_FallDeath(ball);
}

static void install_falldeath_hook() {
    void** slot = (void**)(ADDR_BALL_VTABLE + VTABLE_SLOT_FALLDEATH * sizeof(void*));
    if (IsBadReadPtr(slot, sizeof(void*))) return;

    DWORD old_protect;
    VirtualProtect(slot, sizeof(void*), PAGE_EXECUTE_READWRITE, &old_protect);
    orig_Ball_FallDeath = (ball_falldeath_t)*slot;
    *slot = (void*)Hook_Ball_FallDeath;
    VirtualProtect(slot, sizeof(void*), old_protect, &old_protect);
    FlushInstructionCache(GetCurrentProcess(), slot, sizeof(void*));
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Mod class
 * ═══════════════════════════════════════════════════════════════════════════ */

class WaterMod : public HamsterballAPI {
    IModAPI* api = nullptr;
    bool m_hooksInstalled = false;

public:
    const char* GetModName() override      { return "Water Mod"; }
    const char* GetAuthorName() override   { return "RodentRacer"; }
    const char* GetContributors() override  { return "Hamsterbot"; }
    int GetApiVersion() override           { return HAMSTERBALL_API_VERSION; }

    void Initialize(IModAPI* modApi) override {
        api = modApi;

        CustomButton btn("WATER_MOD", "Water Physics");
        btn.defaultState = true;
        modApi->CreateToggleButton(btn, this);

        createSlider("WATER_ENTRY_DAMP", "Entry Damping", 0.90f, 0.0f, 1.0f, 0.05f, 2);
        createSlider("WATER_DRAG",        "Drag",           0.02f, 0.0f, 0.1f, 0.005f, 3);
        createSlider("WATER_HDRAG",       "Horizontal Drag", 0.04f, 0.0f, 0.1f, 0.005f, 3);
        createSlider("WATER_BUOY",        "Buoyancy",       1.0f,  0.0f, 2.0f, 0.1f, 2);
    }

    void onButtonToggle(const char* buttonId, bool newState) override {
        if (strcmp(buttonId, "WATER_MOD") == 0) {
            g_enabled = newState;
        }
    }

    void onSliderChange(const char* sliderId, float value) override {
        if      (strcmp(sliderId, "WATER_ENTRY_DAMP") == 0) g_entry_damping = value;
        else if (strcmp(sliderId, "WATER_DRAG") == 0)       g_drag = value;
        else if (strcmp(sliderId, "WATER_HDRAG") == 0)       g_horizontal_drag = value;
        else if (strcmp(sliderId, "WATER_BUOY") == 0)        g_buoyancy_strength = value;
    }

    void onGameUpdate() override {
        /* Install raw memory hooks once (deferred to first game update) */
        if (!m_hooksInstalled) {
            install_type5_hook();
            install_falldeath_hook();
            m_hooksInstalled = true;
        }
    }

    void onEventPlaneCollide(Ball* ball, char* eventPlaneID) override {
        if (!g_enabled || !ball || !eventPlaneID) return;
        if (_strnicmp(eventPlaneID, "E:WATER", 7) != 0) return;
        trigger_water_contact((DWORD)ball);
    }

    void onBallUpdate(Ball* ball) override {
        if (!g_enabled || !ball) return;
        apply_water_physics((DWORD)ball);
    }

    void onSceneEnd() override {
        memset(g_states, 0, sizeof(g_states));
    }

    void onLevelStart() override {
        memset(g_states, 0, sizeof(g_states));
    }

private:
    void createSlider(const char* id, const char* label, float defaultVal,
                      float lower, float upper, float step, int decimals) {
        CustomSlider s(id, label, defaultVal);
        s.lowerBound = lower;
        s.upperBound = upper;
        s.stepSize = step;
        s.decimalPlaces = decimals;
        api->CreateSlider(s, this);
    }
};

extern "C" __declspec(dllexport) HamsterballAPI* CreateModInstance() {
    return new WaterMod();
}
