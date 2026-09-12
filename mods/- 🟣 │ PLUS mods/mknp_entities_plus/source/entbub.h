/* entbub.h -- native drowning-bubble visuals for Tarpit (v1cp).
 * Replicates the Dizzy/Master TarBubble look with REAL native bubble
 * objects (0x1C bytes, vtable 0x4D6E48): ctor 0x44FB50
 * __thiscall(this, app, x, y, z) RET 0x10 (objdump-verified on the local
 * exe; the skill note "RET 0x14" is wrong), update 0x44FBE0
 * __fastcall(this) (scale x0.95/f, life -1, bubble1 pop at 0), render
 * 0x44F910 (vtable[2]), dtor 0x44FD40 __thiscall(this, flags).
 * Vtable slots verified: 0x4D6E48 = [dtor, update, render, dtor].
 * Render math (disassembled, NOT the doc note): visual = spawn - scale - 60
 * (0x4D039C = 60.0). Native refs float ~85 above the tar, so spawn the
 * bubble ball+85: the visual starts AT the ball and rises ~25 as the
 * scale shrinks. Spawn at ball height buries it 85u underground.
 * Render takes one dummy stack arg (ret 4, body never reads it).
 * Cross-level self-drive (doc pattern): board+0x3B00 is only driven on
 * Dizzy/Master boards, so the mod drives its own list from game_update
 * (update stage, like the native board updates -- v1cp drove it from
 * text_render/Draw and that crashed 0001:00078EDD). entity_frame notifies
 * (same thread, same tick, right before); bub_frame spawns+updates+renders.
 * Paused => frozen.
 * Update NEVER frees: dtor(obj, 1) at life end. Unload is drop-flagged
 * (bub_clear) and freed on the next game_update tick -- same-thread, no
 * races. Needs: log_mod, snprintf, IsBadReadPtr, g_op_new,
 * g_api/HBAPI, BOARD_PAUSED.
 */
#ifndef ENTBUB_H
#define ENTBUB_H

#define BUBBLE_CTOR   0x0044FB50
#define BUBBLE_UPDATE 0x0044FBE0
#define BUBBLE_RENDER 0x0044F910
#define BUBBLE_DTOR   0x0044FD40
#define BUBBLE_VTABLE 0x004D6E48
#define BUB_MAX 24

typedef void (__thiscall *bubble_ctor_t)(void* self, void* app,
                                         float x, float y, float z);
typedef void (__fastcall *bubble_upd_t)(DWORD obj);
typedef void (__thiscall *bubble_render_t)(void* self, int unused);
typedef void (__thiscall *bubble_dtor_t)(void* self, int flags);

static DWORD g_bub_objs[BUB_MAX];
static int   g_bub_count = 0;
static int   g_bub_want = 0;     /* legacy flag (kept for clarity) */
#define BUB_QMAX 8
static float g_bub_qx[BUB_QMAX];
static float g_bub_qy[BUB_QMAX];
static float g_bub_qz[BUB_QMAX];
static int   g_bub_qk[BUB_QMAX];   /* v1de: 0 = death (real app), 1 = ambient */
static int   g_bub_qn = 0;
static DWORD g_bub_fakeapp = 0;    /* v1de: proxy app for ambient customs */
/* v1dc: learned tar surface per instance (entry ball Y). S1 home sits
 * below the surface the ball rides on, so home-20 spawns buried. */
static float g_bub_surf[64];
static int   g_bub_surfk[64];
static DWORD g_bub_rng = 0xC10C;
static int   g_bub_drop = 0;     /* unload requested, free on render tick */

static DWORD bub_rand(void) {
    g_bub_rng = g_bub_rng * 1664525u + 1013904223u;
    return (g_bub_rng >> 16) & 0x7FFFu;
}

/* game thread: queue one bubble demand for this tick (death + ambient
 * share the queue; bub_frame rolls each). */
static void bub_enqueue(float x, float y, float z, int kind) {
    if (g_bub_qn >= BUB_QMAX) return;
    g_bub_qx[g_bub_qn] = x;
    g_bub_qy[g_bub_qn] = y;
    g_bub_qz[g_bub_qn] = z;
    g_bub_qk[g_bub_qn] = kind;
    g_bub_qn++;
}

/* game thread: mark bubble demand + position for this tick */
static void bub_notify(float x, float y, float z) {
    if (!g_bub_on) { g_bub_want = 0; return; }  /* v1da death_bubbles */
    g_bub_want = 1;
    bub_enqueue(x, y, z, 0);
}

/* v1db: ambient demand (Dizzy/Master look). v1de: own on-toggle. */
static void bub_ambient(float x, float y, float z) {
    if (!g_buba_on) return;
    g_bub_want = 1;
    bub_enqueue(x, y, z, 1);
}

/* v1dc: record entry ball Y as this instance's surface (once). */
static void bub_learn_surf(int inst, float y) {
    char sbuf[64];
    if (inst < 0 || inst >= 64) return;
    if (g_bub_surfk[inst]) return;
    g_bub_surfk[inst] = 1;
    g_bub_surf[inst] = y;
    snprintf(sbuf, sizeof(sbuf), "  BUB: surf%d learned Y=%d",
             inst, (int)y);
    log_mod(sbuf);
}

/* v1dc: ambient spawn height: learned surface - 20 (the settled death
 * offset), else home + 10 (near-surface guess for S1-at-surface). */
static float bub_surf_y(int inst, float home) {
    if (inst >= 0 && inst < 64 && g_bub_surfk[inst])
        return g_bub_surf[inst] - 20.0f;
    return home + 10.0f;
}

/* unload path (any thread): flag, render tick frees (same-thread safe) */
static void bub_clear(void) {
    g_bub_drop = 1;
    g_bub_want = 0;
    g_bub_qn = 0;
}

/* update stage, every game_update: spawn to board+0x3B00, native drives.
 * v1cs experiment: no static 0x3B00 drive loop exists (every xref is an
 * append); if Warm-Up drives the list, bubbles appear with zero mod
 * driving. Mod-side list only caps spawns + drops on unload. */
static void bub_frame(DWORD board) {
    int i;
    if (g_bub_drop) {
        int n = g_bub_count;
        for (i = 0; i < g_bub_count; i++) {
            DWORD o = g_bub_objs[i];
            if (o && !IsBadReadPtr((void*)o, 0x1C) &&
                *(DWORD*)o == BUBBLE_VTABLE)
                ((bubble_dtor_t)BUBBLE_DTOR)((void*)o, 1);
            g_bub_objs[i] = 0;
        }
        g_bub_count = 0;
        g_bub_drop = 0;
        if (n > 0) {
            char cbuf[48];
            snprintf(cbuf, sizeof(cbuf), "  BUB: dropped %d", n);
            log_mod(cbuf);
        }
        return;
    }
    if (!board) { g_bub_want = 0; g_bub_qn = 0; return; }
    if (!IsBadReadPtr((void*)(board + BOARD_PAUSED), 4) &&
        *(int*)(board + BOARD_PAUSED)) {
        g_bub_want = 0;
        g_bub_qn = 0;
        return;
    }
    {
        DWORD app = g_api ? (DWORD)HBAPI(g_api).GetApp() : 0;
        int qi;
        if (app) {
            for (qi = 0; qi < g_bub_qn; qi++) {
                float g_bub_x = g_bub_qx[qi];
                float g_bub_y = g_bub_qy[qi];
                float g_bub_z = g_bub_qz[qi];
                DWORD useapp = app;
                if (g_bub_qk[qi] == 1 && g_bub_fakeapp)  /* v1de ambient */
                    useapp = g_bub_fakeapp;
                if (g_bub_count < BUB_MAX &&
                    (bub_rand() % 100u) < 20u) {
            void* o = g_op_new(0x1Cu);
            if (o && !IsBadReadPtr(o, 0x1C)) {
                float ox = g_bub_x +
                    (float)((int)(bub_rand() % 21u) - 10);
                /* v1cz: -160 from probe (-20) per test. */
                float oy = g_bub_y - 20.0f + (float)(bub_rand() % 9u);
                float oz = g_bub_z +
                    (float)((int)(bub_rand() % 21u) - 10);
                int was = g_bub_count;
                ((bubble_ctor_t)BUBBLE_CTOR)(o, (void*)useapp, ox, oy, oz);
                if (*(DWORD*)o == BUBBLE_VTABLE) {
                    /* v1ct: half-size test (native 25.0) */
                    *(float*)((char*)o + 0x14) = 12.0f;
                    g_bub_objs[g_bub_count++] = (DWORD)o;
                    /* native drive: hand to the board bubble list */
                    if (!IsBadReadPtr((void*)(board + 0x3B00), 8))
                        ((athena_append_t)ATHENALIST_APPEND)(
                            (void*)(board + 0x3B00), o);
                    if (!was) log_mod("  BUB: show started");
                } else {
                    log_mod("  BUB: ctor vtable mismatch, dropped");
                }
            }
                } /* roll per queued demand */
            } /* qi */
        } /* app */
    }
    g_bub_want = 0;
    g_bub_qn = 0;
    /* v1cs: native drives (update+render+free); drop stale locally */
    for (i = 0; i < g_bub_count;) {
        DWORD o = g_bub_objs[i];
        if (!o || IsBadReadPtr((void*)o, 0x1C) ||
            *(DWORD*)o != BUBBLE_VTABLE) {
            g_bub_objs[i] = g_bub_objs[--g_bub_count];
            continue;
        }
        i++;
    }
}

#endif /* ENTBUB_H */
