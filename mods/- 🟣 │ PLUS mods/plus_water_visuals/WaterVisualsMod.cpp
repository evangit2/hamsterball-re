/*
 * WaterVisualsMod.cpp — Plus Water Visuals Mod for HB+ v2.1 (MinGW build)
 *
 * A separate mod from plus_water_mod (physics). This one handles the VISUAL
 * side of water using the game's native TarBubble object (vtable 0x4D6E48,
 * 0x1C bytes) — a round textured sprite that floats upward — so no custom
 * sprites or mesh construction are needed.
 *
 * Features:
 * - Entry splash: on E:WATER contact, spawn a burst of bubbles. Speed-tiered:
 *   fast entry -> more bubbles + "dropin" sound; slow entry -> fewer bubbles
 *   + "dropinshort" sound. (Two distinct effects, switched by entrance speed.)
 * - Sparse while submerged: after the splash, ~1 bubble every N seconds while
 *   the ball stays in water — "few and far between".
 * - Rise-to-equilibrium pop: each bubble floats up (constant size) to the water
 *   surface where the ball floats, freezes, then pops after a random 0.5-1.5s.
 * - Self-drive on non-Dizzy/Master boards: the bubble list (board+0x3B00) is
 *   only iterated+rendered natively by DizzyBoard_Update (0x41D512) and
 *   Master FUN_00420da0 (0x420DA0). On those boards the game drives it; on
 *   every other board this mod runs the animation (onGameUpdate) + render
 *   (onRenderApply), gated on the pause flag (board+0x874).
 * - Lifecycle (critical): native bubble update does NOT free the object. The mod
 *   calls dtor 0x44FD40(obj,1) when a bubble pops and on level unload.
 *
 * Author: RodentRacer / Hamsterbot
 */

#include "nocrt.h"
#include "HamsterballAPI.h"
#include "hbplus_api.h"

#include <windows.h>

#define malloc nc_malloc
#define free nc_free
#define memcpy nc_memcpy
#define memset nc_memset
#define strlen nc_strlen
#define strcmp nc_strcmp
#define strncmp nc_strncmp
#define memcmp nc_memcmp
#define strncpy nc_strncpy
#define snprintf nc_snprintf

/* ═══════════════════════════════════════════════════════════════════════════
 * Game addresses (verified against Hamsterball.exe)
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Native TarBubble visual object */
#define BUBBLE_CTOR             0x0044FB50  /* __thiscall(this, app, x,y,z) RET 0x14 */
#define BUBBLE_DTOR             0x0044FD40  /* __thiscall(this, flags) RET 4; flags&1 free */

/* operator_new (size) __cdecl */
#define OPERATOR_NEW            0x004BA57B

/* AthenaList_Append (list, item) __thiscall RET 4 */
#define ATHENALIST_APPEND       0x00453780

/* Board offsets (board = HB+ GetScene(), 0x4400-byte Board/Scene) */
#define BOARD_BUBBLE_LIST       0x3B00   /* embedded AthenaList: count +0x04, items +0x40C */
#define BOARD_PAUSED            0x874    /* byte: 1 = ESC menu open */
#define BOARD_VTABLE_UPDATE1    0x04     /* vtable[1]: board update fn (board type) */

/* Board update fn (vtable[1]) for boards that natively drive the bubble list */
#define BOARD_UPDATE_DIZZY      0x0041D512
#define BOARD_UPDATE_MASTER     0x00420DA0

/* Ball offsets */
#define BALL_POS_X              0x164
#define BALL_POS_Y              0x168
#define BALL_POS_Z              0x16C
#define BALL_PHYS_PTR           0x1A4
#define BALL_RADIUS             0x284
#define BALL_MAX_SPEED          0x188

/* Physics struct offsets */
#define PHYS_VEL_X              0xCA4
#define PHYS_VEL_Y              0xCA8
#define PHYS_VEL_Z              0xCAC

/* App sound slots (App+0x43C = Sounds base; +0x024 dropin, +0x028 dropinshort) */
#define APP_SND_DROPIN          0x460
#define APP_SND_DROPINSHORT     0x464

/* Bubble object layout (0x1C bytes, vtable 0x4D6E48) */
#define BUBBLE_APP              0x04
#define BUBBLE_POS_X            0x08
#define BUBBLE_POS_Y            0x0C
#define BUBBLE_POS_Z            0x10
#define BUBBLE_SCALE            0x14
#define BUBBLE_LIFETIME         0x18

/* ├─ Rise-to-surface animation state.
 * Since we bypass the native update (which shrinks size 0.95×/frame), we track
 * each bubble's float behavior in a side table keyed by object pointer. */
#define BUBBLE_RISE_SPEED       2.0f    /* world units / second up */
#define FREEZE_MIN_MS           500
#define FREEZE_MAX_MS           1500

#define MAX_BUBBLES             512

struct BubbleAnim {
    DWORD obj;              /* native bubble pointer (key) */
    float target_y;         /* equilibrium surface Y to float up to */
    int   phase;            /* 0 = rising, 1 = frozen, 2 = popped */
    int   freeze_ms_left;   /* countdown before pop when frozen */
};

/* AthenaList embedded layout */
#define ALIST_COUNT             0x04
#define ALIST_ITEMS             0x40C

/* Speed tier: entry is "fast" if speed > FAST_FRACTION * max_speed */
#define FAST_FRACTION           0.35f
#define DEFAULT_BURST           10
#define DEFAULT_RATE            0.5f   /* bubbles/sec while submerged */
#define MAX_BALLS               32

/* ═══════════════════════════════════════════════════════════════════════════
 * Native function typedefs
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef void* (__cdecl *operator_new_t)(size_t size);
typedef void* (__thiscall *bubble_ctor_t)(void* obj, void* app, float x, float y, float z);
typedef void  (__thiscall *bubble_update_t)(void* obj);       /* 0x44FBE0 */
typedef void  (__thiscall *bubble_render_t)(void* obj);       /* 0x44F910 */
typedef void* (__thiscall *bubble_dtor_t)(void* obj, int flags); /* 0x44FD40 */
typedef void  (__thiscall *athena_append_t)(void* list, void* item);

/* ═══════════════════════════════════════════════════════════════════════════
 * Config (HB+ sliders)
 * ═══════════════════════════════════════════════════════════════════════════ */

static bool  g_enabled  = true;
static int   g_burst    = DEFAULT_BURST;   /* fast-entry splash bubble count */
static float g_rate     = DEFAULT_RATE;    /* bubbles/sec while submerged */

static IModAPI* g_api    = NULL;
static void*    g_modObj = NULL;

/* ═══════════════════════════════════════════════════════════════════════════
 * Per-ball water state (for entry/exit + sparse spawn)
 * ═══════════════════════════════════════════════════════════════════════════ */

struct WaterVisState {
    DWORD ball;             /* key; 0 = unused */
    int   in_water;
    float water_surface_y;  /* ball Y captured at first contact */
    int   sparse_timer;     /* frames until next sparse bubble (0 = ready) */
};

static WaterVisState g_states[MAX_BALLS];

static WaterVisState* get_vis_state(DWORD ball) {
    int free_idx = -1;
    for (int i = 0; i < MAX_BALLS; i++) {
        if (g_states[i].ball == ball) return &g_states[i];
        if (free_idx == -1 && g_states[i].ball == 0) free_idx = i;
    }
    if (free_idx >= 0) {
        memset(&g_states[free_idx], 0, sizeof(WaterVisState));
        g_states[free_idx].ball = ball;
        return &g_states[free_idx];
    }
    return NULL;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Tiny LCG (no CRT rand)
 * ═══════════════════════════════════════════════════════════════════════════ */

static unsigned long g_rng_state = 0x9E3779B9UL;

static void rng_seed(void) {
    g_rng_state = (unsigned long)GetTickCount() ^ 0x85EBCA6BUL;
}

static float rng_unit(void) {   /* [0, 1) */
    g_rng_state = g_rng_state * 1664525UL + 1013904223UL;
    return (float)((g_rng_state >> 8) & 0xFFFFFF) / 16777216.0f;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Bubble spawn + list helpers
 * ═══════════════════════════════════════════════════════════════════════════ */

static operator_new_t  g_operator_new  = (operator_new_t)OPERATOR_NEW;
static bubble_ctor_t   g_bubble_ctor   = (bubble_ctor_t)BUBBLE_CTOR;
static bubble_render_t g_bubble_render = (bubble_render_t)0x0044F910;
static bubble_dtor_t   g_bubble_dtor   = (bubble_dtor_t)BUBBLE_DTOR;
static athena_append_t g_athena_append = (athena_append_t)ATHENALIST_APPEND;

static BubbleAnim g_anims[MAX_BUBBLES];

static BubbleAnim* anim_get(DWORD obj) {
    for (int i = 0; i < MAX_BUBBLES; i++) if (g_anims[i].obj == obj) return &g_anims[i];
    return NULL;
}
static BubbleAnim* anim_alloc(DWORD obj) {
    for (int i = 0; i < MAX_BUBBLES; i++) if (g_anims[i].obj == 0) {
        memset(&g_anims[i], 0, sizeof(BubbleAnim));
        g_anims[i].obj = obj;
        return &g_anims[i];
    }
    return NULL;
}
static void anim_remove(DWORD obj) {
    for (int i = 0; i < MAX_BUBBLES; i++) if (g_anims[i].obj == obj) { g_anims[i].obj = 0; return; }
}

/* Spawn one native bubble at (x,y,z) rising toward `target_y`. */
static void spawn_bubble(DWORD board, DWORD app, float x, float y, float z, float target_y) {
    if (!board || !app) return;
    if (IsBadReadPtr((void*)(board + BOARD_BUBBLE_LIST), 0x418)) return;

    /* Small horizontal jitter + vertical spread so bursts look natural */
    float jx = (rng_unit() - 0.5f) * 12.0f;
    float jz = (rng_unit() - 0.5f) * 12.0f;
    float jy = rng_unit() * 6.0f;

    void* obj = g_operator_new(0x1C);
    if (!obj) return;

    g_bubble_ctor(obj, (void*)app, x + jx, y + jy, z + jz);
    g_athena_append((void*)(board + BOARD_BUBBLE_LIST), obj);

    BubbleAnim* an = anim_alloc((DWORD)obj);
    if (an) {
        an->target_y = target_y;   /* + 8 so the visible sprite sits at the surface */
        an->phase = 0;             /* rising */
    }
}

static int bubble_list_count(DWORD board) {
    if (!board || IsBadReadPtr((void*)(board + BOARD_BUBBLE_LIST), 0x418)) return 0;
    int n = *(int*)(board + BOARD_BUBBLE_LIST + ALIST_COUNT);
    if (n < 0 || n > 512) return 0;
    return n;
}

/* Remove index i from the embedded AthenaList at board+off. */
static void list_remove_at(DWORD base, int i) {
    DWORD list = base + BOARD_BUBBLE_LIST;
    int n = *(int*)(list + ALIST_COUNT);
    DWORD items = *(DWORD*)(list + ALIST_ITEMS);
    if (!items || i < 0 || i >= n) return;
    for (int k = i; k < n - 1; k++)
        *(DWORD*)(items + k * 4) = *(DWORD*)(items + (k + 1) * 4);
    *(int*)(list + ALIST_COUNT) = n - 1;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Self-drive: update + render + free every live bubble.
 * Only on boards where the game does NOT drive the list natively (i.e. not
 * Dizzy/Master). Gated on the pause flag so bubbles freeze in the ESC menu.
 * ═══════════════════════════════════════════════════════════════════════════ */

static void drive_bubbles(DWORD board, bool paused) {
    if (!board || IsBadReadPtr((void*)board, 0x4400)) return;

    /* Native boards handle update/render/free themselves — leave them alone. */
    DWORD vtable = *(DWORD*)board;
    if (!vtable || IsBadReadPtr((void*)vtable, 0x80)) return;
    DWORD upd = *(DWORD*)(vtable + BOARD_VTABLE_UPDATE1);
    if (upd == BOARD_UPDATE_DIZZY || upd == BOARD_UPDATE_MASTER) return;

    if (paused) return;

    DWORD list = board + BOARD_BUBBLE_LIST;
    if (IsBadReadPtr((void*)list, 0x418)) return;

    /* ── Animation & lifecycle (rise → freeze → random pop) ── */
    if (g_api) {
        App* app = HBAPI(g_api).GetApp();
        int now_ms = app ? (int)GetTickCount() : 0;
        for (int i = 0; i < MAX_BUBBLES; i++) {
            BubbleAnim* an = &g_anims[i];
            if (!an->obj) continue;
            if (IsBadReadPtr((void*)an->obj, 0x1C)) { an->obj = 0; continue; }

            if (an->phase == 0) {   /* rising */
                float cy = *(float*)(an->obj + BUBBLE_POS_Y);
                float rise = BUBBLE_RISE_SPEED / 60.0f;
                cy += rise;
                if (cy >= an->target_y) {
                    cy = an->target_y;
                    an->phase = 1;  /* freeze */
                    an->freeze_ms_left = FREEZE_MIN_MS +
                        (int)(rng_unit() * (float)(FREEZE_MAX_MS - FREEZE_MIN_MS));
                }
                *(float*)(an->obj + BUBBLE_POS_Y) = cy;
            } else if (an->phase == 1) {  /* frozen: hold, then pop */
                an->freeze_ms_left -= 16;
                if (an->freeze_ms_left <= 0) {
                    an->phase = 2;
                    /* pop: free + remove */
                    DWORD obj = an->obj;
                    an->obj = 0;
                    /* remove from list at this index */
                    DWORD items = *(DWORD*)(list + ALIST_ITEMS);
                    int n = *(int*)(list + ALIST_COUNT);
                    for (int k = 0; k < n; k++) {
                        if (*(DWORD*)(items + k * 4) == obj) {
                            list_remove_at(board, k);
                            break;
                        }
                    }
                    g_bubble_dtor((void*)obj, 1);
                }
            }
        }
    }
}

/* Render every live bubble. Must be called from a render context
 * (HB+ onRenderApply). */
static void render_bubbles(void) {
    if (!g_api) return;
    Scene* scene = HBAPI(g_api).GetScene();
    if (!scene) return;
    DWORD board = (DWORD)scene;
    if (IsBadReadPtr((void*)board, 0x4400)) return;

    DWORD vtable = *(DWORD*)board;
    if (!vtable || IsBadReadPtr((void*)vtable, 0x80)) return;
    DWORD upd = *(DWORD*)(vtable + BOARD_VTABLE_UPDATE1);
    if (upd == BOARD_UPDATE_DIZZY || upd == BOARD_UPDATE_MASTER) return; /* native renders */

    DWORD list = board + BOARD_BUBBLE_LIST;
    if (IsBadReadPtr((void*)list, 0x418)) return;
    int n = *(int*)(list + ALIST_COUNT);
    if (n < 1 || n > 512) return;
    DWORD items = *(DWORD*)(list + ALIST_ITEMS);
    if (!items) return;
    for (int i = 0; i < n; i++) {
        DWORD obj = *(DWORD*)(items + i * 4);
        if (!obj || IsBadReadPtr((void*)obj, 0x1C)) continue;
        g_bubble_render((void*)obj);
    }
}

/* Free every live bubble (level unload / cleanup). */
static void clean_bubbles(void) {
    if (!g_api) return;
    Scene* scene = HBAPI(g_api).GetScene();
    if (!scene) return;
    DWORD board = (DWORD)scene;
    if (IsBadReadPtr((void*)board, 0x4400)) return;
    if (IsBadReadPtr((void*)(board + BOARD_BUBBLE_LIST), 0x418)) return;

    DWORD list = board + BOARD_BUBBLE_LIST;
    int n = *(int*)(list + ALIST_COUNT);
    if (n < 1 || n > 512) { *(int*)(list + ALIST_COUNT) = 0; return; }
    DWORD items = *(DWORD*)(list + ALIST_ITEMS);
    if (items && !IsBadReadPtr((void*)items, (size_t)n * 4)) {
        for (int i = 0; i < n; i++) {
            DWORD obj = *(DWORD*)(items + i * 4);
            if (obj && !IsBadReadPtr((void*)obj, 0x1C))
                g_bubble_dtor((void*)obj, 1);
        }
    }
    *(int*)(list + ALIST_COUNT) = 0;
    memset(g_states, 0, sizeof(g_states));
    memset(g_anims, 0, sizeof(g_anims));
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Water entry logic
 * ═══════════════════════════════════════════════════════════════════════════ */

static void entry_splash(DWORD ball) {
    if (!g_api) return;
    Scene* scene = HBAPI(g_api).GetScene();
    if (!scene) return;
    DWORD board = (DWORD)scene;
    DWORD app = (DWORD)HBAPI(g_api).GetApp();
    if (!board || !app) return;

    float px = *(float*)(ball + BALL_POS_X);
    float py = *(float*)(ball + BALL_POS_Y);
    float pz = *(float*)(ball + BALL_POS_Z);

    /* Entry speed from the physics velocity vector */
    DWORD phys = *(DWORD*)(ball + BALL_PHYS_PTR);
    float vx = 0, vy = 0, vz = 0;
    if (phys && !IsBadReadPtr((void*)phys, 0xCB0)) {
        vx = *(float*)(phys + PHYS_VEL_X);
        vy = *(float*)(phys + PHYS_VEL_Y);
        vz = *(float*)(phys + PHYS_VEL_Z);
    }
    float speed2 = vx * vx + vy * vy + vz * vz;
    float max_speed = *(float*)(ball + BALL_MAX_SPEED);
    if (max_speed <= 0.0f || max_speed > 1000.0f) max_speed = 5.0f;
    bool fast = (speed2 > (FAST_FRACTION * max_speed) * (FAST_FRACTION * max_speed));

    /* Two distinct effects switched by entry speed */
    int n = fast ? g_burst : (g_burst / 3);
    if (n < 2) n = 2;

    /* Equilibrium = captured water surface Y for this ball */
    float eq_y = px;   /* safe default */
    WaterVisState* st = get_vis_state(ball);
    if (st && st->water_surface_y != 0.0f) eq_y = st->water_surface_y;

    for (int i = 0; i < n && i < 40; i++)
        spawn_bubble(board, app, px, py, pz, eq_y);

    void* snd = *(void**)(app + (fast ? APP_SND_DROPIN : APP_SND_DROPINSHORT));
    if (snd && !IsBadReadPtr(snd, 4)) {
        HBAPI(g_api).Play3dSoundEffect(snd, Vec3(px, py, pz), 1.0f);
    }
}

static void trigger_water_contact(DWORD ball, const char* name) {
    (void)name;
    if (!ball || IsBadReadPtr((void*)ball, 0x300)) return;
    WaterVisState* st = get_vis_state(ball);
    if (!st) return;
    if (st->in_water) return;   /* already in — no splash, just keep swimming */

    st->in_water = 1;
    st->water_surface_y = *(float*)(ball + BALL_POS_Y);
    st->sparse_timer = 0;       /* first sparse bubble comes after the splash */

    entry_splash(ball);
}

static void trigger_water_exit(DWORD ball) {
    if (!ball || IsBadReadPtr((void*)ball, 0x300)) return;
    WaterVisState* st = get_vis_state(ball);
    if (!st) return;
    st->in_water = 0;
    st->water_surface_y = 0.0f;
    st->sparse_timer = 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Per-frame: exit detection + sparse bubbles while submerged
 * ═══════════════════════════════════════════════════════════════════════════ */

static void apply_visuals(DWORD ball) {
    if (!g_enabled || !ball || IsBadReadPtr((void*)ball, 0x300)) return;
    WaterVisState* st = get_vis_state(ball);
    if (!st) return;

    if (!st->in_water) return;

    float ball_y = *(float*)(ball + BALL_POS_Y);
    float radius = *(float*)(ball + BALL_RADIUS);
    if (radius <= 0.0f || radius > 1000.0f) return;

    /* Same exit heuristic as the physics mod: clearly above the surface */
    if (ball_y - radius > st->water_surface_y + radius * 0.5f) {
        trigger_water_exit(ball);
        return;
    }

    /* Sparse bubbles: ~1 every (1/rate) seconds. Frame rate assumed 60fps for
     * the timer; the physics mod is frame-based too, so this matches its feel.
     * Default 0.5/s -> one bubble every 2 seconds. */
    if (st->sparse_timer > 0) {
        st->sparse_timer--;
        return;
    }
    int interval = (g_rate > 0.01f) ? (int)(60.0f / g_rate) : 3600;
    if (interval < 30) interval = 30;

    if (!g_api) return;
    Scene* scene = HBAPI(g_api).GetScene();
    if (!scene) return;
    DWORD board = (DWORD)scene;
    DWORD app = (DWORD)HBAPI(g_api).GetApp();
    if (board && app) {
        spawn_bubble(board, app,
                     *(float*)(ball + BALL_POS_X),
                     *(float*)(ball + BALL_POS_Y),
                     *(float*)(ball + BALL_POS_Z),
                     st->water_surface_y);
    }
    st->sparse_timer = interval;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * HB+ vtable callbacks (manual vtables — MinGW-safe)
 * ═══════════════════════════════════════════════════════════════════════════ */

static void* __thiscall sc_dtor(void* thisptr, int flags) {
    if (flags & 1) operator delete(thisptr);
    return thisptr;
}
static const char* __thiscall get_mod_name(void*) { return "Water Visuals"; }
static const char* __thiscall get_author(void*) { return "RodentRacer"; }
static int __thiscall get_version(void*) { return HAMSTERBALL_API_VERSION; }
static const char* __thiscall get_contributors(void*) { return "Hamsterbot"; }

static void __thiscall init_impl(void* thisptr, IModAPI* api) {
    g_api = api;
    g_modObj = thisptr;
    rng_seed();

    CustomButton btn("WATER_VIS", "Water Visuals");
    btn.defaultState = true;
    HBAPI(api).CreateToggleButton(btn, thisptr);

    CustomSlider s1("WVIS_BURST", "Splash Bubbles", (float)DEFAULT_BURST);
    s1.lowerBound = 0.0f; s1.upperBound = 30.0f; s1.stepSize = 1.0f; s1.decimalPlaces = 0;
    HBAPI(api).CreateSlider(s1, thisptr);

    CustomSlider s2("WVIS_RATE", "Submerged Bubbles/s", DEFAULT_RATE);
    s2.lowerBound = 0.0f; s2.upperBound = 5.0f; s2.stepSize = 0.1f; s2.decimalPlaces = 1;
    HBAPI(api).CreateSlider(s2, thisptr);
}

static void __thiscall button_toggle(void*, const char* id, bool state) {
    if (strcmp(id, "WATER_VIS") == 0) g_enabled = state;
}

static void __thiscall slider_change(void*, const char* id, float value) {
    if      (strcmp(id, "WVIS_BURST") == 0) g_burst = (int)value;
    else if (strcmp(id, "WVIS_RATE") == 0)  g_rate = value;
}

static void __thiscall event_collide(void*, void* ball, const char* eventPlaneID) {
    if (!g_enabled || !ball || !eventPlaneID) return;
    if (strncmp(eventPlaneID, "E:WATEREXIT", 11) == 0) {
        trigger_water_exit((DWORD)ball);
    } else if (strncmp(eventPlaneID, "E:WATER", 7) == 0) {
        trigger_water_contact((DWORD)ball, eventPlaneID);
    }
}

static void __thiscall ball_update(void* thisptr, void* ball) {
    (void)thisptr;
    if (!g_enabled || !ball) return;
    apply_visuals((DWORD)ball);
}

static void __thiscall game_update(void*) {
    if (!g_enabled) return;
    if (!g_api) return;
    Scene* scene = HBAPI(g_api).GetScene();
    if (!scene) return;
    DWORD board = (DWORD)scene;
    if (IsBadReadPtr((void*)board, 0x4400)) return;

    bool paused = *(BYTE*)(board + BOARD_PAUSED) != 0;
    drive_bubbles(board, paused);
}

static void __thiscall scene_end(void*) { clean_bubbles(); }
static void __thiscall level_start(void*) { clean_bubbles(); }

static void __thiscall render_apply(void*, void*, float*) {
    if (g_enabled) render_bubbles();
}
static void __thiscall cycle_option_change(void*, const char*, const char*) {}
static void __thiscall text_render(void*) {}
static void __thiscall ball_bump(void*, void*, void*) {}

/* 17-entry vtable (HB+ v2.0/v2.1) */
static void* g_vtable[17] = {
    (void*)sc_dtor,              // [0]  ~HamsterballAPI
    (void*)get_mod_name,         // [1]  GetModName
    (void*)get_author,           // [2]  GetAuthorName
    (void*)get_version,          // [3]  GetApiVersion
    (void*)get_contributors,     // [4]  GetContributors
    (void*)init_impl,            // [5]  Initialize
    (void*)ball_update,          // [6]  onBallUpdate
    (void*)render_apply,         // [7]  onRenderApply
    (void*)button_toggle,        // [8]  onButtonToggle
    (void*)slider_change,        // [9]  onSliderChange
    (void*)cycle_option_change,  // [10] onCycleOptionChange (v2.0+)
    (void*)game_update,          // [11] onGameUpdate
    (void*)event_collide,        // [12] onEventPlaneCollide
    (void*)text_render,          // [13] onTextRenderLoop
    (void*)ball_bump,            // [14] onBallBump
    (void*)scene_end,            // [15] onSceneEnd
    (void*)level_start,          // [16] onLevelStart
};

extern "C" __declspec(dllexport) HamsterballAPI* CreateModInstance() {
    void* obj = operator new(8);
    *(void**)obj = g_vtable;
    *(void**)((char*)obj + 4) = NULL;
    return (HamsterballAPI*)obj;
}
