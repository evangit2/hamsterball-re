/* aibeh.h -- AI behaviour dispatch for mknp_battyball_entities (v1bg).
 *
 * Maps mknp_custom_entities AI names (verbatim, case-insensitive) to native
 * game ctors. v1bg implements the safe native subset:
 *   1 Rotator, 2 Pendulum, 6 Swirl, 7 Flickfloor1, 10 Judge, 11 Glassbreaker,
 *   13 Sign, 15 8ball, 19 Flickfloor2, 20 Flickring, 21 Trode, 33 Bonk,
 *   35 Catapult, 36 Mace, 37 Tipper, 38 Lifter, 39 Speedcylinder,
 *   40 Neonplatform, 41 Trapdoor, 42 Droplifter, 43 Gluebie, 45 Timebutton,
 *   46 6ball (v1dv: BadBall body + custom Levels mesh in slot 2),
 *   47 E:Launch (v1dw: visible launch pad, mod-side driver in cpp;
 *      v1b: e_Launch/Launch/E:Launch/REF:Launch identical).
 * Static PopCylinder spawn shape (visible, solid — motion via drivers):
 *   0 (all other names), 16 Bridgeslam, 23 Chrome, 24 Funball,
 *   25 Tarbubble, 26 Waterwheel, 44 Tarpit/Cheesepit (v1cf: tar driver;
 *      v1b: Tarpit/Cheesepit/E:Cheesepit/REF:Tarpit identical).
 * Woodbridge is handled by the existing entity_frame code (unchanged).
 *
 * INCLUDE POINT: after board defines + g_op_new/g_append/g_remove +
 * get_level + log_mod + memset (e.g. right before scan_spawn_entities).
 * Needs: DWORD, IsBadReadPtr, log_mod, g_op_new, g_append, g_remove,
 * get_level, BOARD_UPDATE_LIST, BOARD_RENDER_LIST, BOARD_COLLISION_LIST,
 * BOARD_SCENE_OBJ, BOARD_SCENE_UPDATE_LIST, LEVEL_SCENEOBJECT,
 * MESHWORLD_SIZE, APP_GFX_DEVICE, memset, strlen (via nocrt macros).
 */
#ifndef AIBEH_H
#define AIBEH_H

/* Native ctor entry points (Ghidra-verified, same as mknp_custom_entities) */
#define AIBEH_ROTATOR_CTOR      0x00435940
#define AIBEH_PENDULUM_CTOR     0x00437700
#define AIBEH_ARENASTANDS_CTOR  0x0043E450
#define AIBEH_SECRET_CTOR       0x0043DFB0
#define AIBEH_GEARLEVEL_CTOR    0x0043A150
#define AIBEH_SIGN_CTOR         0x00443B90
#define AIBEH_BADBALL_CTOR      0x0040AFE0
#define AIBEH_BONK_CTOR         0x00438850
#define AIBEH_CATAPULT_CTOR     0x00437E10
#define AIBEH_MACE_CTOR         0x00438750
#define AIBEH_TIPPER_CTOR       0x00437960
#define AIBEH_GLUEBIE_CTOR      0x00437CB0
#define AIBEH_LIFTER_CTOR       0x00436920
#define AIBEH_SPEEDCYL_CTOR     0x00436A20
#define AIBEH_NEONPLATFORM_CTOR 0x0043E110
#define AIBEH_TRAPDOOR_CTOR     0x00438290
#define AIBEH_ODDLIFTER_CTOR    0x00434E60
#define AIBEH_TIPPER_VISUAL_CTOR 0x004661A0
#define AIBEH_TIPPER_ATTACH     0x00465200
#define AIBEH_LEVEL_RENDERCTOR  0x00465080
#define AIBEH_BOARD_APP         0x878
#define AIBEH_BOARD_BADBALLS    0x29D4

/* Object sizes (same as mknp_custom_entities) */
#define AIBEH_SZ_ROTATOR      0x1508
#define AIBEH_SZ_PENDULUM     0x1504
#define AIBEH_SZ_ARENASTANDS  0x1104
#define AIBEH_SZ_SECRET       0x10EC
#define AIBEH_SZ_GEARLEVEL    0x1100
#define AIBEH_SZ_SIGN         0x10FC
#define AIBEH_SZ_BADBALL      0xC98
#define AIBEH_SZ_BONK         0x1200
#define AIBEH_SZ_CATAPULT     0x1108
#define AIBEH_SZ_MACE         0x110C
#define AIBEH_SZ_TIPPER       0x1104
#define AIBEH_SZ_GLUEBIE      0x110C
#define AIBEH_SZ_LIFTER       0x10F4
#define AIBEH_SZ_SPEEDCYL     0x150C
#define AIBEH_SZ_NEONPLATFORM 0x1104
#define AIBEH_SZ_TRAPDOOR     0x10F8
#define AIBEH_SZ_ODDLIFTER    0x10F4
#define AIBEH_SZ_LEVEL        0x10D0

/* Ctor signatures (MinGW-safe names, no near/far) */
typedef void* (__thiscall *aibeh_rot6_t)(void*, void*, float, float, float, void*);
typedef void* (__thiscall *aibeh_stands3_t)(void*, void*, void*);
typedef void* (__thiscall *aibeh_lifter7_t)(void*, void*, float, float, float, void*, int);
typedef void* (__thiscall *aibeh_speedcyl7_t)(void*, void*, float, float, float, int, void*);
typedef void* (__thiscall *aibeh_oddlift5_t)(void*, void*, float, float, float);
typedef void* (__thiscall *aibeh_trapdoor2_t)(void*, void*);
typedef void* (__thiscall *aibeh_gearlevel5_t)(void*, void*, float, float, float);
typedef void* (__thiscall *aibeh_sign10_t)(void*, void*, int, int, int, int, int, int, int, int);
typedef void* (__thiscall *aibeh_badball2_t)(void*, void*);
typedef void* (__thiscall *aibeh_bonk5_t)(void*, void*, float, float, float);
typedef void* (__thiscall *aibeh_tipvis_t)(void*, int);
typedef void  (__thiscall *aibeh_tipattach_t)(void*, int);
typedef void* (__thiscall *aibeh_renderctor_t)(void*, void*);

static aibeh_rot6_t      aibeh_pfn_rotator   = (aibeh_rot6_t)AIBEH_ROTATOR_CTOR;
static aibeh_rot6_t      aibeh_pfn_pendulum  = (aibeh_rot6_t)AIBEH_PENDULUM_CTOR;
static aibeh_rot6_t      aibeh_pfn_arenast   = (aibeh_rot6_t)AIBEH_ARENASTANDS_CTOR;
static aibeh_rot6_t      aibeh_pfn_secret    = (aibeh_rot6_t)AIBEH_SECRET_CTOR;
static aibeh_gearlevel5_t aibeh_pfn_gearlevel = (aibeh_gearlevel5_t)AIBEH_GEARLEVEL_CTOR;
static aibeh_sign10_t    aibeh_pfn_sign      = (aibeh_sign10_t)AIBEH_SIGN_CTOR;
static aibeh_badball2_t  aibeh_pfn_badball   = (aibeh_badball2_t)AIBEH_BADBALL_CTOR;
static aibeh_bonk5_t     aibeh_pfn_bonk      = (aibeh_bonk5_t)AIBEH_BONK_CTOR;
static aibeh_stands3_t   aibeh_pfn_catapult  = (aibeh_stands3_t)AIBEH_CATAPULT_CTOR;
static aibeh_stands3_t   aibeh_pfn_mace      = (aibeh_stands3_t)AIBEH_MACE_CTOR;
static aibeh_stands3_t   aibeh_pfn_tipper    = (aibeh_stands3_t)AIBEH_TIPPER_CTOR;
static aibeh_stands3_t   aibeh_pfn_gluebie   = (aibeh_stands3_t)AIBEH_GLUEBIE_CTOR;
static aibeh_lifter7_t   aibeh_pfn_lifter    = (aibeh_lifter7_t)AIBEH_LIFTER_CTOR;
static aibeh_speedcyl7_t aibeh_pfn_speedcyl  = (aibeh_speedcyl7_t)AIBEH_SPEEDCYL_CTOR;
static aibeh_rot6_t      aibeh_pfn_neonplat  = (aibeh_rot6_t)AIBEH_NEONPLATFORM_CTOR;
static aibeh_trapdoor2_t aibeh_pfn_trapdoor  = (aibeh_trapdoor2_t)AIBEH_TRAPDOOR_CTOR;
static aibeh_oddlift5_t  aibeh_pfn_oddlift   = (aibeh_oddlift5_t)AIBEH_ODDLIFTER_CTOR;
static aibeh_tipvis_t    aibeh_pfn_tipvis    = (aibeh_tipvis_t)AIBEH_TIPPER_VISUAL_CTOR;
static aibeh_tipattach_t aibeh_pfn_tipattach = (aibeh_tipattach_t)AIBEH_TIPPER_ATTACH;
static aibeh_renderctor_t aibeh_pfn_renderctor = (aibeh_renderctor_t)AIBEH_LEVEL_RENDERCTOR;

/* Exact case-insensitive match (avoids Gear matching Gear_Level etc.) */
static int aibeh_streq(const char* a, const char* b) {
    int i = 0;
    if (!a || !b) return 0;
    while (a[i] && b[i]) {
        char ca = a[i], cb = b[i];
        if (ca >= 'A' && ca <= 'Z') ca += 32;
        if (cb >= 'A' && cb <= 'Z') cb += 32;
        if (ca != cb) return 0;
        i++;
    }
    return a[i] == b[i];
}

/* v1b: strip E:/REF: prefix (ci) so bare, REF: and E: behaviours map identically */
static const char* aibeh_noprefix(const char* s) {
    if (!s) return s;
    if ((s[0] == 'E' || s[0] == 'e') && s[1] == ':') return s + 2;
    if ((s[0] == 'R' || s[0] == 'r') && (s[1] == 'E' || s[1] == 'e') &&
        (s[2] == 'F' || s[2] == 'f') && s[3] == ':') return s + 4;
    return s;
}

/* Behaviour string -> AI type. Verbatim AI names from mknp_custom_entities.
 * Unknown or empty -> 0 (static PopCylinder fallback, always safe).
 * v1b prefix-blind: REF:/E: stripped, so REF:Woodbridge == Woodbridge,
 * E:Launch == e_Launch == Launch (47), E:Cheesepit == Cheesepit == Tarpit (44). */
static int aibeh_type(const char* beh) {
    if (!beh || !beh[0]) return 0;
    beh = aibeh_noprefix(beh);
    if (aibeh_streq(beh, "Rotator")) return 1;
    if (aibeh_streq(beh, "Pendulum")) return 2;
    if (aibeh_streq(beh, "Swirl")) return 6;
    if (aibeh_streq(beh, "Flickfloor1")) return 7;
    if (aibeh_streq(beh, "Judge")) return 10;
    if (aibeh_streq(beh, "Glassbreaker")) return 11;
    if (aibeh_streq(beh, "Sign")) return 13;
    if (aibeh_streq(beh, "8ball")) return 15;
    if (aibeh_streq(beh, "6ball")) return 46;
    if (aibeh_streq(beh, "Bridgeslam")) return 16;
    if (aibeh_streq(beh, "Flickfloor2")) return 19;
    if (aibeh_streq(beh, "Flickring")) return 20;
    if (aibeh_streq(beh, "Trode")) return 21;
    if (aibeh_streq(beh, "Chrome")) return 23;
    if (aibeh_streq(beh, "Funball")) return 24;
    if (aibeh_streq(beh, "Tarbubble")) return 25;
    if (aibeh_streq(beh, "Waterwheel")) return 26;
    if (aibeh_streq(beh, "Bonk")) return 33;
    if (aibeh_streq(beh, "Catapult")) return 35;
    if (aibeh_streq(beh, "Mace")) return 36;
    if (aibeh_streq(beh, "Tipper")) return 37;
    if (aibeh_streq(beh, "Lifter")) return 38;
    if (aibeh_streq(beh, "Speedcylinder")) return 39;
    if (aibeh_streq(beh, "Neonplatform")) return 40;
    if (aibeh_streq(beh, "Trapdoor")) return 41;
    if (aibeh_streq(beh, "Droplifter")) return 42;
    if (aibeh_streq(beh, "Gluebie")) return 43;
    if (aibeh_streq(beh, "Tarpit")) return 44;
    if (aibeh_streq(beh, "Cheesepit")) return 44;
    if (aibeh_streq(beh, "Timebutton")) return 45;
    if (aibeh_streq(beh, "e_Launch")) return 47;
    if (aibeh_streq(beh, "Launch")) return 47;
    return 0;
}

/* 1 = static PopCylinder fallback (caller uses create_grid_cube+grid_show) */
static int aibeh_is_static(int t) {
    return (t == 0 || t == 16 || t == 23 || t == 24 || t == 25 || t == 26 || t == 44 || t == 47);
}

/* 0 = ctor takes no mesh (skip MeshWorld load + file validation) */
static int aibeh_needs_mesh(int t) {
    if (t == 10 || t == 15 || t == 33 || t == 41 || t == 42) return 0;
    return 1;
}

/* Alloc size for native types (0 = static, caller uses POPCYLINDER_SIZE) */
static unsigned aibeh_size(int t) {
    switch (t) {
        case 1: return AIBEH_SZ_ROTATOR;
        case 2: return AIBEH_SZ_PENDULUM;
        case 6: return AIBEH_SZ_ROTATOR;
        case 7: case 19: case 20: case 21: return AIBEH_SZ_ARENASTANDS;
        case 10: return AIBEH_SZ_GEARLEVEL;
        case 11: return AIBEH_SZ_SECRET;
        case 13: return AIBEH_SZ_SIGN;
        case 15: return AIBEH_SZ_BADBALL;
        case 46: return AIBEH_SZ_BADBALL;
        case 33: return AIBEH_SZ_BONK;
        case 35: return AIBEH_SZ_CATAPULT;
        case 36: return AIBEH_SZ_MACE;
        case 37: return AIBEH_SZ_TIPPER;
        case 38: return AIBEH_SZ_LIFTER;
        case 39: case 45: return AIBEH_SZ_SPEEDCYL;
        case 40: return AIBEH_SZ_NEONPLATFORM;
        case 41: return AIBEH_SZ_TRAPDOOR;
        case 42: return AIBEH_SZ_ODDLIFTER;
        case 43: return AIBEH_SZ_GLUEBIE;
        default: return 0;
    }
}

/* Spawn a native object. mesh = preloaded MeshWorld (NULL when !needs_mesh).
 * app = App ptr (0 if unknown, Sign falls back to zeros). Returns obj or NULL. */
static void* aibeh_spawn(DWORD board, float px, float py, float pz,
                         void* mesh, int type, DWORD app) {
    void* obj;
    if (!board) return NULL;
    switch (type) {
        case 1:
            obj = g_op_new(AIBEH_SZ_ROTATOR);
            if (!obj) return NULL;
            memset(obj, 0, AIBEH_SZ_ROTATOR);
            aibeh_pfn_rotator(obj, (void*)board, px, py, pz, mesh);
            *(float*)((char*)obj + 0x10E8) = 0.0f;
            return obj;
        case 2:
            obj = g_op_new(AIBEH_SZ_PENDULUM);
            if (!obj) return NULL;
            memset(obj, 0, AIBEH_SZ_PENDULUM);
            aibeh_pfn_pendulum(obj, (void*)board, px, py, pz, mesh);
            *(float*)((char*)obj + 0x10E8) = 0.0f;
            return obj;
        case 6:
            obj = g_op_new(AIBEH_SZ_ROTATOR);
            if (!obj) return NULL;
            memset(obj, 0, AIBEH_SZ_ROTATOR);
            aibeh_pfn_rotator(obj, (void*)board, px, py, pz, mesh);
            *(float*)((char*)obj + 0x10E8) = 0.0f;
            return obj;
        case 7: case 20: case 21:
            obj = g_op_new(AIBEH_SZ_ARENASTANDS);
            if (!obj) return NULL;
            memset(obj, 0, AIBEH_SZ_ARENASTANDS);
            aibeh_pfn_arenast(obj, (void*)board, px, py, pz, mesh);
            return obj;
        case 19:
            obj = g_op_new(AIBEH_SZ_ARENASTANDS);
            if (!obj) return NULL;
            memset(obj, 0, AIBEH_SZ_ARENASTANDS);
            aibeh_pfn_arenast(obj, (void*)board, px, py, pz, mesh);
            *(DWORD*)((char*)obj + 0x10DC) = 2;
            *(DWORD*)((char*)obj + 0x10E0) = 0;
            return obj;
        case 10:
            obj = g_op_new(AIBEH_SZ_GEARLEVEL);
            if (!obj) return NULL;
            memset(obj, 0, AIBEH_SZ_GEARLEVEL);
            aibeh_pfn_gearlevel(obj, (void*)board, px, py, pz);
            return obj;
        case 11:
            obj = g_op_new(AIBEH_SZ_SECRET);
            if (!obj) return NULL;
            memset(obj, 0, AIBEH_SZ_SECRET);
            aibeh_pfn_secret(obj, (void*)board, px, py, pz, mesh);
            return obj;
        case 13: {
            int gfx1 = 0, gfx2 = 0;
            if (app && !IsBadReadPtr((void*)(app + 0x58C), 8)) {
                gfx1 = *(int*)(app + 0x58C);
                gfx2 = *(int*)(app + 0x590);
            }
            obj = g_op_new(AIBEH_SZ_SIGN);
            if (!obj) return NULL;
            memset(obj, 0, AIBEH_SZ_SIGN);
            aibeh_pfn_sign(obj, (void*)board, gfx1, gfx2,
                           *(int*)&px, *(int*)&py, *(int*)&pz, 0, 0, 0);
            return obj;
        }
        case 15: {
            DWORD* vt;
            obj = g_op_new(AIBEH_SZ_BADBALL);
            if (!obj) return NULL;
            memset(obj, 0, AIBEH_SZ_BADBALL);
            aibeh_pfn_badball(obj, (void*)board);
            vt = *(DWORD**)obj;
            if (vt && !IsBadReadPtr(vt, 8)) {
                DWORD f1 = vt[1];
                if (f1 > 0x400000 && !IsBadReadPtr((void*)f1, 4)) {
                    typedef void (__thiscall *v1_t)(void*);
                    ((v1_t)f1)(obj);
                }
            }
            *(float*)((char*)obj + 0xC60) = px;
            *(float*)((char*)obj + 0xC64) = py;
            *(float*)((char*)obj + 0xC68) = pz;
            /* v1dt: 8ball mesh slot (custom_entities v54 pattern). Ball_Render
             * draws App+0x244[ball+0x754*4] only when 754<3: copy the 8Ball
             * mesh (App+0x268, index 9) into slot 1 (App+0x248) + set 754=1. */
            if (app && !IsBadReadPtr((void*)(app + 0x268), 4)) {
                DWORD m8 = *(DWORD*)(app + 0x268);
                if (m8 && !IsBadReadPtr((void*)(app + 0x248), 4)) {
                    *(DWORD*)(app + 0x248) = m8;
                    *(DWORD*)((char*)obj + 0x754) = 1;
                } else log_mod("  ENT 8ball: mesh missing, sphere fallback");
            }
            return obj;
        }
        case 46: {
            DWORD* vt;
            obj = g_op_new(AIBEH_SZ_BADBALL);
            if (!obj) return NULL;
            memset(obj, 0, AIBEH_SZ_BADBALL);
            aibeh_pfn_badball(obj, (void*)board);
            vt = *(DWORD**)obj;
            if (vt && !IsBadReadPtr(vt, 8)) {
                DWORD f1 = vt[1];
                if (f1 > 0x400000 && !IsBadReadPtr((void*)f1, 4)) {
                    typedef void (__thiscall *v1_t)(void*);
                    ((v1_t)f1)(obj);
                }
            }
            *(float*)((char*)obj + 0xC60) = px;
            *(float*)((char*)obj + 0xC64) = py;
            *(float*)((char*)obj + 0xC68) = pz;
            /* v1dv: 6ball = BadBall body + custom MESHWORLD skin.
             * Same BadBall_ctor+v[1]+home as 8ball (case 15). Custom
             * MeshWorld* (Levels/<mesh>, loaded by caller) goes into
             * App mesh slot 2 (App+0x24C, SphereBreak2) + 754=2, the
             * FunBall pattern from custom_entities (Ball_Render only
             * draws slots 0-2). Slot 1 stays 8ball-exclusive, no clash.
             * Bad mesh/app = 8ball-mesh fallback (never invisible). */
            if (mesh && !IsBadReadPtr((void*)mesh, 4) &&
                app && !IsBadReadPtr((void*)(app + 0x24C), 4)) {
                *(DWORD*)(app + 0x24C) = (DWORD)mesh;
                *(DWORD*)((char*)obj + 0x754) = 2;
            } else if (app && !IsBadReadPtr((void*)(app + 0x268), 4)) {
                DWORD m8 = *(DWORD*)(app + 0x268);
                if (m8 && !IsBadReadPtr((void*)(app + 0x248), 4)) {
                    *(DWORD*)(app + 0x248) = m8;
                    *(DWORD*)((char*)obj + 0x754) = 1;
                    log_mod("  ENT 6ball: custom mesh bad, 8ball fallback");
                } else log_mod("  ENT 6ball: mesh missing, sphere fallback");
            } else log_mod("  ENT 6ball: mesh missing, sphere fallback");
            return obj;
        }
        case 33:
            obj = g_op_new(AIBEH_SZ_BONK);
            if (!obj) return NULL;
            memset(obj, 0, AIBEH_SZ_BONK);
            aibeh_pfn_bonk(obj, (void*)board, px, py, pz);
            return obj;
        case 35:
            obj = g_op_new(AIBEH_SZ_CATAPULT);
            if (!obj) return NULL;
            memset(obj, 0, AIBEH_SZ_CATAPULT);
            aibeh_pfn_catapult(obj, (void*)board, mesh);
            *(float*)((char*)obj + 0x10D8) = px;
            *(float*)((char*)obj + 0x10DC) = py;
            *(float*)((char*)obj + 0x10E0) = pz;
            return obj;
        case 36:
            obj = g_op_new(AIBEH_SZ_MACE);
            if (!obj) return NULL;
            memset(obj, 0, AIBEH_SZ_MACE);
            aibeh_pfn_mace(obj, (void*)board, mesh);
            *(float*)((char*)obj + 0x10D8) = px;
            *(float*)((char*)obj + 0x10DC) = py;
            *(float*)((char*)obj + 0x10E0) = pz;
            return obj;
        case 37: {
            void* render_level;
            void* visual;
            obj = g_op_new(AIBEH_SZ_TIPPER);
            if (!obj) return NULL;
            memset(obj, 0, AIBEH_SZ_TIPPER);
            aibeh_pfn_tipper(obj, (void*)board, mesh);
            *(float*)((char*)obj + 0x10D8) = px;
            *(float*)((char*)obj + 0x10DC) = py;
            *(float*)((char*)obj + 0x10E0) = pz;
            if (!mesh || IsBadReadPtr(mesh, 0x100)) {
                log_mod("  ENT Tipper: mesh invalid, visual skipped (non-solid)");
                return obj;
            }
            render_level = g_op_new(AIBEH_SZ_LEVEL);
            if (!render_level) return obj;
            memset(render_level, 0, AIBEH_SZ_LEVEL);
            render_level = aibeh_pfn_renderctor(render_level, mesh);
            if (!render_level) {
                log_mod("  ENT Tipper: render Level failed (non-solid)");
                return obj;
            }
            visual = g_op_new(AIBEH_SZ_LEVEL);
            if (!visual) return obj;
            memset(visual, 0, AIBEH_SZ_LEVEL);
            visual = aibeh_pfn_tipvis(visual, (int)render_level);
            if (!visual) {
                log_mod("  ENT Tipper: visual failed (non-solid)");
                return obj;
            }
            *(DWORD*)((char*)obj + 0x10D4) = (DWORD)visual;
            aibeh_pfn_tipattach(visual, (int)obj);
            { char tbuf[96]; snprintf(tbuf, sizeof(tbuf),
                "  ENT Tipper: visual ok obj=0x%X vis=0x%X",
                (DWORD)obj, (DWORD)visual); log_mod(tbuf); }
            return obj;
        }
        case 38:
            obj = g_op_new(AIBEH_SZ_LIFTER);
            if (!obj) return NULL;
            memset(obj, 0, AIBEH_SZ_LIFTER);
            aibeh_pfn_lifter(obj, (void*)board, px, py, pz, mesh, 0);
            return obj;
        case 39: case 45: {
            DWORD sc_col, lvl_mw, mb_list, mb_count, mb_items;
            int mi;
            obj = g_op_new(AIBEH_SZ_SPEEDCYL);
            if (!obj) return NULL;
            memset(obj, 0, AIBEH_SZ_SPEEDCYL);
            aibeh_pfn_speedcyl(obj, (void*)board, px, py, pz, 0, mesh);
            *(float*)((char*)obj + 0x10D4) = px;
            *(float*)((char*)obj + 0x10D8) = py;
            *(float*)((char*)obj + 0x10DC) = pz;
            sc_col = *(DWORD*)((char*)obj + 0x10E0);
            if (!sc_col || IsBadReadPtr((void*)sc_col, 0x20)) return obj;
            lvl_mw = *(DWORD*)((char*)sc_col + 0x08);
            if (lvl_mw && !IsBadReadPtr((void*)(lvl_mw + 0x2C), 0x410)) {
                mb_list = lvl_mw + 0x2C;
                mb_count = *(DWORD*)(mb_list + 0x04);
                mb_items = *(DWORD*)(mb_list + 0x40C);
                for (mi = 0; mi < (int)mb_count && mb_items &&
                     !IsBadReadPtr((void*)mb_items, mb_count * 4); mi++) {
                    DWORD mb = ((DWORD*)mb_items)[mi];
                    if (mb && !IsBadReadPtr((void*)mb, 0x48C))
                        *(DWORD*)((char*)mb + 0x47C) = (DWORD)obj;
                }
            }
            *(DWORD*)((char*)obj + 0x47C) = (DWORD)obj;
            /* v1dt: Timebutton rides the SpeedCyl shape (custom_entities
             * v55n_38: native 0x436C10 ctor crashes, SpeedCyl 0x436A20 is
             * proven). Without the vtable swap a Timebutton S1 would spin +
             * launch like a SpeedCyl. 0x4D5830 = TimeButton vtable,
             * +0x10E5=1 render-once, +0x10E4=0 not-pressed latch. Press
             * reward (N:EXTRATIME + timer) still needs a dispatch driver. */
            if (type == 45) {
                *(DWORD*)obj = 0x4D5830;
                *(char*)((char*)obj + 0x10E5) = 1;
                *(char*)((char*)obj + 0x10E4) = 0;
                log_mod("  ENT Timebutton: vtable 0x4D5830 set (press pending)");
            }
            return obj;
        }
        case 40:
            obj = g_op_new(AIBEH_SZ_NEONPLATFORM);
            if (!obj) return NULL;
            memset(obj, 0, AIBEH_SZ_NEONPLATFORM);
            aibeh_pfn_neonplat(obj, (void*)board, px, py, pz, mesh);
            return obj;
        case 41: {
            DWORD app2, mesh_table;
            if (!board || IsBadReadPtr((void*)(board + AIBEH_BOARD_APP), 4)) {
                log_mod("  ENT Trapdoor: board unreadable, skip");
                return NULL;
            }
            app2 = *(DWORD*)(board + AIBEH_BOARD_APP);
            mesh_table = (app2 && !IsBadReadPtr((void*)(app2 + 0x878), 4))
                         ? *(DWORD*)(app2 + 0x878) : 0;
            if (!mesh_table || IsBadReadPtr((void*)mesh_table, 0x5A4) ||
                !*(DWORD*)(mesh_table + 0x594)) {
                log_mod("  ENT Trapdoor: native mesh missing, skip");
                return NULL;
            }
            obj = g_op_new(AIBEH_SZ_TRAPDOOR);
            if (!obj) return NULL;
            memset(obj, 0, AIBEH_SZ_TRAPDOOR);
            aibeh_pfn_trapdoor(obj, (void*)board);
            *(float*)((char*)obj + 0x10D8) = px;
            *(float*)((char*)obj + 0x10DC) = py;
            *(float*)((char*)obj + 0x10E0) = pz;
            return obj;
        }
        case 42: {
            DWORD app2, mesh_table;
            if (!board || IsBadReadPtr((void*)(board + AIBEH_BOARD_APP), 4)) {
                log_mod("  ENT Droplifter: board unreadable, skip");
                return NULL;
            }
            app2 = *(DWORD*)(board + AIBEH_BOARD_APP);
            mesh_table = (app2 && !IsBadReadPtr((void*)(app2 + 0x878), 4))
                         ? *(DWORD*)(app2 + 0x878) : 0;
            if (!mesh_table || IsBadReadPtr((void*)mesh_table, 0x5CC) ||
                !*(DWORD*)(mesh_table + 0x5C8)) {
                log_mod("  ENT Droplifter: native mesh missing, skip");
                return NULL;
            }
            obj = g_op_new(AIBEH_SZ_ODDLIFTER);
            if (!obj) return NULL;
            memset(obj, 0, AIBEH_SZ_ODDLIFTER);
            aibeh_pfn_oddlift(obj, (void*)board, px, py, pz);
            return obj;
        }
        case 43: {
            DWORD bname = 0;
            const char* bn = NULL;
            const char* want = "Board (Dizzy)";
            int wi = 0, ok = 1;
            obj = g_op_new(AIBEH_SZ_GLUEBIE);
            if (!obj) return NULL;
            memset(obj, 0, AIBEH_SZ_GLUEBIE);
            aibeh_pfn_gluebie(obj, (void*)board, mesh);
            *(float*)((char*)obj + 0x10D4) = px;
            *(float*)((char*)obj + 0x10D8) = py;
            *(float*)((char*)obj + 0x10DC) = pz;
            /* v1dt: Dizzy proximity list (custom_entities v55j_12 pattern).
             * Native DizzyBoard_Update slows the ball only for members of
             * board+0x4378. Never touch board+0x6080 (sorted-insert crash). */
            if (board && !IsBadReadPtr((void*)(board + 0x868), 4)) {
                bname = *(DWORD*)(board + 0x868);
                if (bname && !IsBadReadPtr((void*)bname, 16)) bn = (const char*)bname;
            }
            if (bn) {
                while (want[wi]) {
                    char a = bn[wi], b2 = want[wi];
                    if (a >= 'A' && a <= 'Z') a += 32;
                    if (b2 >= 'A' && b2 <= 'Z') b2 += 32;
                    if (a != b2) { ok = 0; break; }
                    wi++;
                }
                if (ok && bn[wi] == '\0') {
                    g_append((void*)(board + 0x4378), obj);
                    log_mod("  ENT Gluebie: Dizzy list +0x4378 joined");
                } else log_mod("  ENT Gluebie: non-Dizzy, visual+solid only");
            }
            return obj;
        }
        default:
            return NULL;
    }
}

/* Register a native object into game lists.
 * update + render + scene tree always; collision selective (crash-safe):
 * Rotator family +0x10D4, Catapult +0x10D4 Level, SpeedCyl +0x10E0 Level,
 * BadBall also joins the bad-balls list. All others: no collision list. */
static void aibeh_show(DWORD board, DWORD obj, int type) {
    DWORD col_obj, scene_col, level, sceneobj;
    if (!board || !obj) return;
    if (IsBadReadPtr((void*)obj, 0x20)) return;
    g_append((void*)(board + BOARD_UPDATE_LIST), (void*)obj);
    g_append((void*)(board + BOARD_RENDER_LIST), (void*)obj);
    if (type == 15 || type == 46)
        g_append((void*)(board + AIBEH_BOARD_BADBALLS), (void*)obj);
    col_obj = 0;
    if (type == 1 || type == 2 || type == 6 || type == 35)
        col_obj = *(DWORD*)((char*)obj + 0x10D4);
    else if (type == 39 || type == 45)
        col_obj = *(DWORD*)((char*)obj + 0x10E0);
    if (col_obj && col_obj > 0x10000 && !IsBadReadPtr((void*)col_obj, 0x20)) {
        g_append((void*)(board + BOARD_COLLISION_LIST), (void*)col_obj);
        scene_col = 0;
        if (!IsBadReadPtr((void*)(board + BOARD_SCENE_OBJ), 4))
            scene_col = *(DWORD*)(board + BOARD_SCENE_OBJ);
        if (scene_col) g_append((void*)(scene_col + 0x18), (void*)col_obj);
    }
    level = get_level(board);
    if (level) {
        sceneobj = 0;
        if (!IsBadReadPtr((void*)(level + LEVEL_SCENEOBJECT), 4))
            sceneobj = *(DWORD*)(level + LEVEL_SCENEOBJECT);
        if (sceneobj) g_append((void*)(sceneobj + 0x1C), (void*)obj);
    }
}

/* Remove a native object from game lists. Size-guarded: never touches
 * offsets past the allocation (BadBall 0xC98 has no +0x10E4 flags). */
static void aibeh_remove(DWORD board, DWORD obj, int type, unsigned size) {
    DWORD col_obj, scene_col, level, sceneobj;
    if (!board || !obj) return;
    if (IsBadReadPtr((void*)obj, 0x20)) return;
    if (size > 0x10E6 && !IsBadReadPtr((void*)obj, 0x10E6)) {
        *(unsigned char*)((char*)obj + 0x10E5) = 1;
        *(unsigned char*)((char*)obj + 0x10E4) = 1;
    }
    col_obj = 0;
    if ((type == 1 || type == 2 || type == 6 || type == 35) && size > 0x10D8)
        col_obj = *(DWORD*)((char*)obj + 0x10D4);
    else if ((type == 39 || type == 45) && size > 0x10E4)
        col_obj = *(DWORD*)((char*)obj + 0x10E0);
    else if (type == 0 && size > 0x10E4)
        col_obj = *(DWORD*)((char*)obj + 0x10E0);
    if (col_obj && col_obj > 0x10000 && !IsBadReadPtr((void*)col_obj, 4)) {
        g_remove((void*)(board + BOARD_COLLISION_LIST), (int)col_obj);
        scene_col = 0;
        if (!IsBadReadPtr((void*)(board + BOARD_SCENE_OBJ), 4))
            scene_col = *(DWORD*)(board + BOARD_SCENE_OBJ);
        if (scene_col) g_remove((void*)(scene_col + 0x18), (int)col_obj);
    }
    g_remove((void*)(board + BOARD_UPDATE_LIST), (int)obj);
    g_remove((void*)(board + BOARD_SCENE_UPDATE_LIST), (int)obj);
    g_remove((void*)(board + BOARD_RENDER_LIST), (int)obj);
    if (type == 15 || type == 46)
        g_remove((void*)(board + AIBEH_BOARD_BADBALLS), (int)obj);
    level = get_level(board);
    if (level) {
        sceneobj = 0;
        if (!IsBadReadPtr((void*)(level + LEVEL_SCENEOBJECT), 4))
            sceneobj = *(DWORD*)(level + LEVEL_SCENEOBJECT);
        if (sceneobj) g_remove((void*)(sceneobj + 0x1C), (int)obj);
    }
}

#endif /* AIBEH_H */
