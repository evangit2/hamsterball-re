/* entsky.h -- Cloudscape skybox sprite (v1z). Replicates Sky Race native
 * clouds exactly (LevelBoard_Sky_ctor 0x41F930 + SkyBoard_RenderDynamic
 * 0x410E80, Ghidra-verified; call shapes mirror LFL lf_04_render_update.c):
 *   load:  Sprite_ctor (0x45D0C0), op_new 0x110, "textures\clouds.png"
 *   pos:   S1 REF:Cloudscape point (native reads the CLOUDSCAPE file entry)
 *   frame: SetProjection(gfx,20,50000) + gfx+0x70C state + vtbl[0x32](0xe,0)
 *          + SetCullMode2(gfx,0,1) + RenderQuad(sprite,x,y,z,23.0,0)
 *          + SetCullMode2(gfx,1,1) + state back + projection restore
 * Marker-style driver (like Woodbridge_area): no instance, no collision,
 * no mesh load (mesh key required by the set parser, ignored here). One
 * global sprite reused across levels (never freed, no dtor needed). Draws
 * from render_apply (render thread). Sky slot 13 keeps native clouds.
 *
 * INCLUDE POINT: mid-file group (after entdefs.h + log_mod + g_op_new +
 * g_api/HBAPI + APP_GFX_DEVICE + g_race_slot). Needs: DWORD, IsBadReadPtr,
 * log_mod, snprintf, memset, g_op_new, g_api, HBAPI, APP_GFX_DEVICE,
 * g_race_slot, g_ent_vis, ent_strip_prefix, player_board.
 */
#ifndef ENTSKY_H
#define ENTSKY_H

#define SKY_SPRITE_CTOR   0x0045D0C0   /* __thiscall(mem, gfx, path) */
#define SKY_RENDER_QUAD   0x0045D660   /* __thiscall(sprite,x,y,z,layer,unk) */
#define SKY_SET_PROJ      0x00454AB0   /* __thiscall(gfx, near, far) */
#define SKY_SET_CULL      0x00453970   /* __thiscall(gfx, mode, persist) */
#define SKY_SPRITE_SIZE   0x110        /* LFL S1EnsureSprite precedent */
#define SKY_TEX_PATH      "textures\\clouds.png"
#define SKY_NEAR_PLANE    20.0f
#define SKY_FAR_PLANE     50000.0f
#define SKY_QUAD_LAYER    23.0f
#define SKY_RACE_SLOT     13           /* Sky: native clouds win, mod idle */

typedef void* (__thiscall *sky_ctor_t)(void* self, void* gfx, const char* path);
typedef void (__thiscall *sky_quad_t)(void* sprite, float x, float y, float z, float layer, int unk);
typedef void (__thiscall *sky_proj_t)(void* gfx, float xp, float fp);
typedef void (__thiscall *sky_cull_t)(void* gfx, int mode, int persist);
typedef void (__thiscall *sky_gfxstate_t)(void* gfx, int a, int b);

static sky_ctor_t g_sky_ctor = (sky_ctor_t)SKY_SPRITE_CTOR;
static sky_quad_t g_sky_quad = (sky_quad_t)SKY_RENDER_QUAD;
static sky_proj_t g_sky_proj = (sky_proj_t)SKY_SET_PROJ;
static sky_cull_t g_sky_cull = (sky_cull_t)SKY_SET_CULL;

static DWORD g_sky_sprite = 0;      /* global reuse, never freed */
static float g_sky_x = 0.0f;
static float g_sky_y = 0.0f;
static float g_sky_z = 0.0f;
static int   g_sky_active = 0;      /* render_apply draws */
static int   g_sky_pending = 0;     /* pos stored, sprite not yet ensured */
static int   g_sky_dead = 0;        /* def present but idle (reason logged) */
static int   g_sky_extra = 0;       /* extra S1 matches ignored */
static DWORD g_sky_board = 0;
static int   g_sky_drew = 0;        /* first-quad log once per activation */

/* prefix-blind exact "cloudscape" (REF:Cloudscape == Cloudscape == E:Cloudscape) */
static int sky_beh_is(const char* beh) {
    const char* w = "cloudscape";
    int i = 0;
    if (!beh || !beh[0]) return 0;
    beh = ent_strip_prefix(beh);
    while (w[i] && beh[i]) {
        char c = beh[i];
        if (c >= 'A' && c <= 'Z') c += 32;
        if (c != w[i]) return 0;
        i++;
    }
    return (w[i] == '\0' && beh[i] == '\0') ? 1 : 0;
}

/* 1 = a Cloudscape def exists in the set (for the no-S1-ref notice) */
static int sky_have_def(void) {
    int d;
    for (d = 0; d < g_ent_count; d++) {
        if (g_ent_beh[d][0] && sky_beh_is(g_ent_beh[d])) return 1;
    }
    return 0;
}

static void sky_reset(void) {
    g_sky_active = 0;
    g_sky_pending = 0;
    g_sky_dead = 0;
    g_sky_extra = 0;
    g_sky_board = 0;
    g_sky_drew = 0;
}

/* First matching S1 wins. Sky slot / visible=false => idle with a reason. */
static void sky_record(DWORD board, int d, const char* nm,
                       float px, float py, float pz) {
    char sbuf[160];
    int L = 0, k = 0;
    if (g_sky_pending || g_sky_active || g_sky_dead) {
        g_sky_extra++;
        return;
    }
    g_sky_x = px;
    g_sky_y = py;
    g_sky_z = pz;
    g_sky_board = board;
    if (g_race_slot == SKY_RACE_SLOT) {
        g_sky_dead = 1;
        log_mod("  SKY: Sky slot keeps native clouds, mod idle");
        return;
    }
    if (d >= 0 && d < ENT_MAX_DEFS && !g_ent_vis[d]) {
        g_sky_dead = 1;
        log_mod("  SKY: visible=false, off");
        return;
    }
    g_sky_pending = 1;
    snprintf(sbuf, sizeof(sbuf), "  SKY: ref=");
    while (sbuf[L]) L++;
    while (k < 40 && nm && nm[k] && L + 1 < (int)sizeof(sbuf))
        sbuf[L++] = nm[k++];
    sbuf[L] = '\0';
    snprintf(sbuf + L, sizeof(sbuf) - (unsigned)L, " at (%d,%d,%d)",
             (int)px, (int)py, (int)pz);
    log_mod(sbuf);
}

/* Main-thread ensure (runs from game_update). Gfx missing => retry next frame. */
static void sky_tick(DWORD board) {
    DWORD app;
    DWORD gfx;
    void* mem;
    void* spr;
    char cbuf[96];
    (void)board;
    if (!g_sky_pending || g_sky_active || g_sky_dead) return;
    if (g_sky_sprite) {
        g_sky_pending = 0;
        g_sky_active = 1;
        snprintf(cbuf, sizeof(cbuf), "  SKY: on (sprite reused) ignored=%d",
                 g_sky_extra);
        log_mod(cbuf);
        return;
    }
    if (!g_api) return;
    app = (DWORD)HBAPI(g_api).GetApp();
    if (!app || IsBadReadPtr((void*)app, 0x200)) return;
    gfx = *(DWORD*)(app + APP_GFX_DEVICE);
    if (!gfx || IsBadReadPtr((void*)gfx, 4)) return;
    mem = g_op_new(SKY_SPRITE_SIZE);
    if (!mem) {
        g_sky_pending = 0;
        log_mod("  SKY: op_new failed, off");
        return;
    }
    memset(mem, 0, SKY_SPRITE_SIZE);
    spr = g_sky_ctor(mem, (void*)gfx, SKY_TEX_PATH);
    if (!spr) {
        g_sky_pending = 0;
        log_mod("  SKY: Sprite_ctor failed, off");
        return;
    }
    g_sky_sprite = (DWORD)spr;
    g_sky_pending = 0;
    g_sky_active = 1;
    snprintf(cbuf, sizeof(cbuf), "  SKY: on sprite=0x%X at (%d,%d,%d) ignored=%d",
             g_sky_sprite, (int)g_sky_x, (int)g_sky_y, (int)g_sky_z,
             g_sky_extra);
    log_mod(cbuf);
}

/* Render-thread draw (runs from render_apply). Mirrors 0x410E80 exactly. */
static void sky_render(void) {
    DWORD board;
    DWORD app;
    DWORD gfx;
    DWORD sprite;
    DWORD* vtbl;
    DWORD fn;
    float fx;
    float fov;
    if (!g_sky_active || !g_sky_sprite) return;
    if (!g_api) return;
    board = player_board();
    if (!board || IsBadReadPtr((void*)board, 0x4400)) return;
    app = (DWORD)HBAPI(g_api).GetApp();
    if (!app || IsBadReadPtr((void*)app, 0x700)) return;
    gfx = *(DWORD*)(app + APP_GFX_DEVICE);
    if (!gfx || IsBadReadPtr((void*)gfx, 0x800)) return;
    sprite = g_sky_sprite;
    if (IsBadReadPtr((void*)sprite, SKY_SPRITE_SIZE)) return;
    g_sky_proj((void*)gfx, SKY_NEAR_PLANE, SKY_FAR_PLANE);
    if (*(char*)(gfx + 0x70C) != 0) {
        vtbl = *(DWORD**)gfx;
        if (vtbl && !IsBadReadPtr(vtbl + 0x32, 4)) {
            fn = vtbl[0x32];
            if (fn > 0x400000 && !IsBadReadPtr((void*)fn, 4))
                ((sky_gfxstate_t)fn)((void*)gfx, 0xe, 0);
        }
        *(char*)(gfx + 0x70C) = 0;
        (*(int*)(gfx + 0x7C8))++;
    }
    g_sky_cull((void*)gfx, 0, 1);
    g_sky_quad((void*)sprite, g_sky_x, g_sky_y, g_sky_z, SKY_QUAD_LAYER, 0);
    g_sky_cull((void*)gfx, 1, 1);
    if (*(char*)(gfx + 0x70C) != 1) {
        vtbl = *(DWORD**)gfx;
        if (vtbl && !IsBadReadPtr(vtbl + 0x32, 4)) {
            fn = vtbl[0x32];
            if (fn > 0x400000 && !IsBadReadPtr((void*)fn, 4))
                ((sky_gfxstate_t)fn)((void*)gfx, 0xe, 1);
        }
        *(char*)(gfx + 0x70C) = 1;
        (*(int*)(gfx + 0x7C8))++;
    }
    fx = *(float*)(gfx + 0x188);
    fov = (fx + fx) * 0.0009765625f + 0.00048828125f;
    g_sky_proj((void*)gfx, SKY_NEAR_PLANE, fov + fov);
    if (!g_sky_drew) {
        g_sky_drew = 1;
        log_mod("  SKY: first quad drawn");
    }
}

#endif /* ENTSKY_H */
