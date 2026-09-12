/* Chapter 1 - bumper physics, game function pointers, mesh path table (LevelFeatures.c lines 1426-1661) */
/* ═══════════════════════════════════════════════════════════════════════════
 * Apply bumper bounce physics
 * ═══════════════════════════════════════════════════════════════════════════ */

static void ApplyBumperBounce(void *board, void *ball, void *collPair) {
    DWORD *pair = (DWORD *)collPair;
    DWORD meshBuf = pair[COLL_MESHBUF / 4];
    if (!meshBuf || IsBadReadPtr((void *)meshBuf, 0x870)) return;

    char *name = *(char **)(meshBuf + MESHBUF_NAME);
    if (!name || IsBadReadPtr(name, 9)) return;

    if (my_strnicmp(name, "N:BUMPER", 8) != 0)
        return;

    /* Determine level for per-level physics constants */
    int level = GetCurrentLevel(board);
    float velScale = (level == 14) ? BUMPER_VEL_SCALE_MASTER : BUMPER_VEL_SCALE_BEGINNER;
    float maxSpeed = (level == 14) ? BUMPER_MAX_SPEED_MASTER : BUMPER_MAX_SPEED_BEGINNER;

    if (IsBadReadPtr(ball, 0x1A8)) return;
    float posX = *(float *)((char *)ball + BALL_POS_X);
    float posY = *(float *)((char *)ball + BALL_POS_Y);
    float posZ = *(float *)((char *)ball + BALL_POS_Z);

    DWORD *app = *(DWORD **)((char *)board + BOARD_APP_PTR);
    if (app && !IsBadReadPtr(app, 0x500)) {
        DWORD soundList = app[APP_SOUNDFX_LIST / 4];
        if (soundList && g_SoundPlay3D) {
            g_SoundPlay3D((void *)soundList, posX, posY, posZ, 1.0f);
        }
    }

    DWORD phys = *(DWORD *)((char *)ball + BALL_PHYS_PTR);
    if (!phys || IsBadReadPtr((void *)phys, 0xCB0)) return;

    float *velX = (float *)(phys + PHYS_VEL_X);
    float *velY = (float *)(phys + PHYS_VEL_Y);
    float *velZ = (float *)(phys + PHYS_VEL_Z);

    float vx = *velX * velScale;
    float vz = *velZ * velScale;
    float vy = 0.0f;

    float speedSq = vx * vx + vz * vz;
    if (speedSq < BUMPER_MIN_SPEED * BUMPER_MIN_SPEED) {
        if (speedSq > 0.0001f) {
            float speed = sqrtf(speedSq);
            float scale = BUMPER_MIN_SPEED / speed;
            vx *= scale; vz *= scale;
        }
    }
    speedSq = vx * vx + vz * vz;
    if (speedSq > maxSpeed * maxSpeed) {
        float speed = sqrtf(speedSq);
        if (speed > 0.0001f) {
            float scale = maxSpeed / speed;
            vx *= scale; vz *= scale;
        }
    }

    *velX = vx; *velY = vy; *velZ = vz;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Universal collision logic
 * ═══════════════════════════════════════════════════════════════════════════ */

void __cdecl BumperCollisionLogic(void *board, void *ball, void *collPair) {
    int level = GetCurrentLevel(board);
    if (level == 0) return;
    // g_objectEnabled removed — S1-driven via IsS1CollisionEnabled
    ApplyBumperBounce(board, ball, collPair);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Naked thunk for DispatchCollisionEvents hook
 * ═══════════════════════════════════════════════════════════════════════════ */

__attribute__((naked)) static void Hook_DispatchCollisionEvents(void) {
    __asm__ __volatile__(
        "pushl %%ebp\n\t"
        "movl  %%esp, %%ebp\n\t"
        "pushl %%ecx\n\t"
        "pushl %%edx\n\t"

        "pushl 12(%%ebp)\n\t"      /* collPair */
        "pushl 8(%%ebp)\n\t"       /* ball */
        "pushl -4(%%ebp)\n\t"      /* board (saved on stack) */
        "call  _BumperCollisionLogic\n\t"
        "addl  $12, %%esp\n\t"

        "popl  %%edx\n\t"
        "popl  %%ecx\n\t"
        "popl  %%ebp\n\t"

        "jmpl  *_g_OriginalDispatch\n\t"
        :: : "eax", "memory"
    );
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Game function typedefs
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef void *(__cdecl *operator_new_t)(unsigned int size);
typedef void *(__thiscall *Level_MeshWorldCtor_t)(void *mem, void *gfx, const char *meshPath);
typedef void *(__thiscall *Level_RenderCtor_t)(void *mem, void *meshWorld);
typedef void (__thiscall *Level_InitScene_t)(void *board);
typedef void (__thiscall *Scene_CollectByNameFilter_t)(void *meshWorld, char *name, void *destList);
typedef void *(__thiscall *AthenaList_Init_t)(void *this, int capacity);
typedef void *(__thiscall *Board_ctor_t)(void *this, int app);
typedef void (__thiscall *LoadRaceData_t)(void *board, const char *raceName);
typedef int (__thiscall *Vec3_Init_t)(void *out, float x, float y, float z);
typedef void (__thiscall *Matrix_Identity_t)(void *out); /* native 0x453200: ECX-only */
typedef void *(__thiscall *MeshNode_ctor_t)(void *mem, void *gfx, const char *path);
typedef void *(__thiscall *Sprite_ctor_t)(void *mem, void *gfx, const char *path);
typedef void (__thiscall *TipperVisual_Attach_t)(void *renderObj, void *meshWorld);
typedef void (__thiscall *Level_AssignTex_t)(void *board, void *meshWorld);
typedef int (__thiscall *Sound_GetNextChannel_t)(void *soundDevice);
typedef void (__thiscall *Scene_RenderIfVisible_t)(int obj, float unk); /* native 0x459610: ret $0x4 */
typedef void (__thiscall *AthenaList_Append_t)(void *list, int item);
typedef int (__thiscall *AthenaList_GetSize_t)(void *list);
typedef int (__thiscall *AthenaList_GetIterator_t)(void *list);
typedef void (__stdcall *eh_vector_ctor_t)(void *base, DWORD stride, int count, void *ctor_fn, void *dtor_fn);
typedef void (__thiscall *Level_AssignTexScales_t)(void *board, void *meshWorld);

/* CreateDynamicObjects ctor typedefs */
typedef void *__thiscall (*Ctor3_t)(void *mem, int board, int meshPtr);  /* Tipper, Gluebie, Catapult, Mace, Glass_Level */
typedef void *__thiscall (*Ctor2_t)(void *mem, int board);  /* Trapdoor */
typedef void *__thiscall (*Ctor4f_t)(void *mem, int board, float x, float y, float z);  /* Bonk, Bell, Gear_Level */
typedef void *__thiscall (*Ctor5f_t)(void *mem, int board, float x, float y, float z, float f);  /* Fan, Spinner_Level */
typedef void *__thiscall (*Ctor3f_t)(void *mem, int board, float x, float y, float z, int meshPtr);  /* Rotator_Impossible, Fallout, NeonPlatform, ArenaStands, Looper, Pendulum, Lifter */
typedef void *__thiscall (*Ctor6f_t)(void *mem, int board, float x, float y, float z, int meshPtr, int pathObj);  /* Saw, Saw2, Blockdawg */
typedef void *__thiscall (*Ctor7f_t)(void *mem, int board, float x, float y, float z, int x2, int y2, int z2, int meshPtr);  /* Gear */
typedef void *__thiscall (*Ctor_Str_t)(void *mem, int board, float x, float y, float z, const char *path);  /* Wavy */
typedef void *__thiscall (*Ctor_Lifter_t)(void *mem, int board, float x, float y, float z, int meshPtr, long num);  /* Lifter (Up) */
typedef void *__thiscall (*Ctor_SpeedCyl_t)(void *mem, int board, float x, float y, float z, int num, int meshPtr);  /* SpeedCylinder */
typedef void *__thiscall (*Ctor_Rotator_t)(void *mem, int board, float x, float y, float z, float f, int meshPtr);  /* Rotator (Sky trapdoor) */
typedef void *__thiscall (*Stands_ctor_t)(void *mem, int meshPtr);  /* Stands */
typedef void *__thiscall (*TipperVisual_ctor_t)(void *mem, int renderObj);  /* TipperVisual */
typedef int  (__thiscall *Level_FindObjectByName_t)(int meshWorld, const char *name);
typedef void (__thiscall *SawBlade_SetVariant_t)(void *obj, int variant);
typedef void (__thiscall *Wavy_Configure_t)(void *obj, int a, int b, int c, int d);
typedef void (__thiscall *Sound_InitChannels_t)(void *obj, int flag);

static operator_new_t g_operatorNew = NULL;
static Level_MeshWorldCtor_t g_LevelMeshWorldCtor = NULL;
static Level_RenderCtor_t g_LevelRenderCtor = NULL;
static Level_InitScene_t g_LevelInitScene = NULL;
static Scene_CollectByNameFilter_t g_CollectByNameFilter = NULL;
static AthenaList_Init_t g_AthenaListInit = NULL;
static Board_ctor_t g_BoardCtor = NULL;
static LoadRaceData_t g_LoadRaceData = NULL;
static Vec3_Init_t g_Vec3Init = NULL;
static Matrix_Identity_t g_MatrixIdentity = NULL;
static MeshNode_ctor_t g_MeshNodeCtor = NULL;
static Sprite_ctor_t g_SpriteCtor = NULL;
static TipperVisual_Attach_t g_TipperVisualAttach = NULL;
static Level_AssignTex_t g_LevelAssignTex = NULL;
static Sound_GetNextChannel_t g_SoundGetNextChannel = NULL;
static Scene_RenderIfVisible_t g_SceneRenderIfVisible = NULL;
static AthenaList_Append_t g_AthenaListAppend = NULL;
static AthenaList_GetSize_t g_AthenaListGetSize = NULL;
static AthenaList_GetIterator_t g_AthenaListGetIterator = NULL;
static eh_vector_ctor_t g_ehVectorCtor = NULL;
static Level_AssignTexScales_t g_LevelAssignTexScales = NULL;
static void *g_ehVectorCtorFn = NULL;
static void *g_Vec3ListFree = NULL;

/* CreateDynamicObjects ctor pointers */
static Ctor3_t g_TipperCtor = NULL;
static TipperVisual_ctor_t g_TipperVisualCtor = NULL;
static Ctor3_t g_GluebieCtor = NULL;
static Ctor3_t g_CatapultCtor = NULL;
static Ctor3_t g_MaceCtor = NULL;
static Ctor3_t g_GlassLevelCtor = NULL;
static Ctor2_t g_TrapdoorCtor = NULL;
static Stands_ctor_t g_StandsCtor = NULL;
static Ctor4f_t g_BonkCtor = NULL;
static Ctor5f_t g_FanCtor = NULL;
static Ctor4f_t g_SawBladeCtor = NULL;
static SawBlade_SetVariant_t g_SawBladeSetVariant = NULL;
static Ctor5f_t g_SpinnerLevelCtor = NULL;
static Ctor5f_t g_SecretCtor = NULL; /* Secret_ctor(mem,board,x,y,z,mesh) — same arity */
static Ctor4f_t g_GearLevelCtor = NULL;
static Ctor4f_t g_BellCtor = NULL;
static Ctor4f_t g_OddLifterCtor = NULL;
static Ctor_Lifter_t g_LifterCtor = NULL;
static Ctor_SpeedCyl_t g_SpeedCylinderCtor = NULL;
static Ctor3f_t g_TimeButtonCtor = NULL;
static Ctor3f_t g_RotatorImpossibleCtor = NULL;
static Ctor6f_t g_SawCtor = NULL;
static Ctor6f_t g_Saw2Ctor = NULL;
static Ctor3f_t g_FalloutCtor = NULL;
static Ctor6f_t g_BlockdawgCtor = NULL;
static Ctor3f_t g_GameLevelCtor = NULL;
static Ctor_Str_t g_WavyCtor = NULL;
static Wavy_Configure_t g_WavyConfigure = NULL;
static Ctor3f_t g_NeonPlatformCtor = NULL;
static Ctor3f_t g_ArenaStandsCtor = NULL;
static Ctor3f_t g_PopcylinderCtor = NULL;
static Ctor3f_t g_PopCylinderCtor = NULL;
static Ctor_Rotator_t g_RotatorCtor = NULL;
static Ctor3f_t g_LooperCtor = NULL;
static Ctor7f_t g_GearCtor = NULL;
static Ctor3f_t g_PendulumCtor = NULL;
static Ctor3f_t g_BreakBridgeCtor = NULL;
static Level_FindObjectByName_t g_LevelFindObjectByName = NULL;
static Sound_InitChannels_t g_SoundInitChannels = NULL;

/* Forward declaration */
static void UniversalPostSetup(void *board);

/* ═══════════════════════════════════════════════════════════════════════════
 * Mesh path table — one per level (for scene loading hook)
 * ═══════════════════════════════════════════════════════════════════════════ */

static const char *g_meshPaths[16] = {
    NULL,                          /* 0 unused */
    "levels\\level1",              /* 1=WarmUp */
    "levels\\levelcascade",        /* 2=Beginner */
    "levels\\level2",              /* 3=Intermediate */
    "levels\\level3",              /* 4=Dizzy */
    "levels\\level4",              /* 5=Tower */
    "levels\\levelup",             /* 6=Up */
    "levels\\leveldark",           /* 7=Neon */
    "levels\\level5",              /* 8=Expert */
    "levels\\level6",              /* 9=Odd */
    "levels\\level8",              /* 10=Toob */
    "levels\\level7",              /* 11=Wobbly */
    "levels\\levelglass",          /* 12=Glass */
    "levels\\level9",              /* 13=Sky */
    "levels\\level10",             /* 14=Master */
    "levels\\levelimpossible",     /* 15=Impossible */
};
