/* Chapter 6 - collision config + UniversalDispatchCollision (LevelFeatures.c lines 4418-5674) */
/* ═══════════════════════════════════════════════════════════════════════════
 * Universal DispatchCollision (Slot 29) — merges all 15 per-level handlers
 *
 * Instead of delegating to per-level handlers, this function implements ALL
 * collision event checks from all 15 levels in one place. Each level-specific
 * event is gated by IsCollisionEventEnabled() which reads from the [COLLISION]
 * section of LevelFeatures.txt. At the end, it calls DispatchCollisionEvents
 * (the global handler) which handles shared events (N:GOAL, N:TARPIT, etc.).
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Collision event config table — maps event name to per-level enabled flags.
 * Populated from [COLLISION] section of LevelFeatures.txt. */
#define MAX_COLLISION_EVENTS 64
typedef struct {
    char name[32];
    int enabled[16]; /* enabled[level] = 1 if active */
} CollisionEventEntry;

static CollisionEventEntry g_collisionEvents[MAX_COLLISION_EVENTS];
static int g_collisionEventCount = 0;

/* Default collision event assignments (matching original game behavior).
 * These are set on startup, then overridden by [COLLISION] section if present. */
/* Parse [COLLISION] section from LevelFeatures.txt */
static void LoadCollisionConfig(char *buf, DWORD bufSize) {
    char *line = buf;
    int inCollisionSection = 0;
    while (line < buf + bufSize) {
        char *eol = line;
        while (*eol && *eol != '\n' && *eol != '\r') eol++;
        char saved = *eol;
        *eol = '\0';
        char *p = line;
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '\0' || *p == '#' || *p == ';') goto next_coll_line;
        if (p[0] == '[') {
            inCollisionSection = (my_strnicmp(p, "[COLLISION", 10) == 0);
            goto next_coll_line;
        }
        if (inCollisionSection) {
            /* Parse: EVENT_NAME = 1 3 5 */
            char *eq = p;
            while (*eq && *eq != '=') eq++;
            if (*eq == '=') {
                *eq = '\0';
                char *name = p;
                char *val = eq + 1;
                /* trim name */
                while (*name == ' ' || *name == '\t') name++;
                int nl = strlen(name);
                while (nl > 0 && (name[nl-1] == ' ' || name[nl-1] == '\t')) name[--nl] = '\0';
                /* find or create entry */
                int idx = -1, i;
                for (i = 0; i < g_collisionEventCount; i++) {
                    if (my_stricmp(g_collisionEvents[i].name, name) == 0) {
                        idx = i; break;
                    }
                }
                if (idx < 0 && g_collisionEventCount < MAX_COLLISION_EVENTS) {
                    idx = g_collisionEventCount++;
                    my_strncpy(g_collisionEvents[idx].name, name, 32);
                    memset(g_collisionEvents[idx].enabled, 0, sizeof(int)*16);
                }
                if (idx >= 0) {
                    /* clear existing, parse level numbers */
                    memset(g_collisionEvents[idx].enabled, 0, sizeof(int)*16);
                    while (*val) {
                        while (*val == ' ' || *val == '\t') val++;
                        if (!*val) break;
                        if (*val == '(') {
                            val++;
                            while (*val && *val != ')') val++;
                            if (*val == ')') val++;
                            continue;
                        }
                        if (*val >= '0' && *val <= '9') {
                            int levelNum = atoi(val);
                            if (levelNum >= 1 && levelNum <= 15)
                                g_collisionEvents[idx].enabled[levelNum] = 1;
                            while (*val && *val >= '0' && *val <= '9') val++;
                        } else {
                            val++;
                        }
                    }
                }
            }
        }
    next_coll_line:
        *eol = saved;
        if (*eol == '\r') eol++;
        if (*eol == '\n') eol++;
        line = eol;
    }
}

/* Collision handler function pointer types */
typedef void (__thiscall *Sound_PlayChannel_t)(int channel);
typedef void (__thiscall *Ball_DizzyImmunity_t)(int *ball, long time);
typedef void (__fastcall *Ball_Grow_t)(int ball);
typedef void (__fastcall *Ball_Shrink_t)(int ball);
typedef void (__thiscall *Ball_SetName_t)(int *ball, char *name);
typedef void (__fastcall *Ball_ApplyTrajectory_t)(int ball);
typedef void (__fastcall *Ball_SetTiltedGravity_t)(int ball);
typedef void (__fastcall *Ball_SetFlatGravity_t)(int ball);
typedef void (__fastcall *Rotator_MarkTriggered_t)(int obj);
typedef void (__fastcall *Rotator_PlayCollisionSound_t)(int obj);
typedef void (__fastcall *Rotator_TriggerSound_t)(int obj);
typedef void (__fastcall *Rotator_StartSound_t)(int obj);
typedef void (__thiscall *Rotator_AddBall_t)(void *obj, int ball);
typedef void (__fastcall *CheckArenaUnlock_t)(int board);
typedef void (__fastcall *Catapult_Launch_t)(int obj);
typedef void (__fastcall *Trapdoor_Open_t)(int obj);
typedef void (__fastcall *Trapdoor_Activate_t)(int obj);
typedef void (__fastcall *Saw_AlertActivate_t)(int obj);
typedef void (__fastcall *Saw_Activate_t)(int obj);
typedef void (__fastcall *Bell_Activate_t)(int obj);
typedef void (__fastcall *Judge_Reset_t)(int obj);
typedef void (__thiscall *ScoreDisplay_SetTime_t)(void *obj, long time);
typedef void (__fastcall *CreateBonkPopup_t)(int obj);
typedef void (__fastcall *Hammer_ChaseStart_t)(int obj);
typedef void (__thiscall *Pendulum_PlayCollisionSound_t)(void *obj, int ball);
typedef void (__thiscall *Pendulum_AddIndex_t)(void *obj, int ball);
typedef int (__thiscall *ScoreObject_ctor_t)(void *mem, int board, int timerOffset, const char *label);
typedef void (__fastcall *Timer_Decrement_t)(int obj);
typedef void *__thiscall (*ArenaScoreParticle_ctor_t)(void *mem, int app);
typedef float *(__thiscall *AthenaHashTable_Lookup_t)(void *hashTable, float *out, const char *key, char *found);
typedef int (__thiscall *AthenaList_ContainsValue_t)(void *list, int item);
typedef void (__thiscall *SceneObject_sub1_ctor_t)(void *out);
typedef void (__thiscall *AthenaString_Set_t)(void *obj, const char *str);
typedef void *__thiscall (*MWParser_ReadTag_t)(void *obj);
typedef void (__thiscall *StreamReader_dtor_t)(void *obj);
typedef void (__thiscall *Audio_PlayMusic_t)(void *musicDevice, const char *name);
typedef float (__thiscall *Difficulty_GetTimeModifier_t)(int app, float time);
typedef char *__cdecl (*AthenaString_Format_t)(int obj, const char *fmt);
typedef void (__cdecl *AthenaString_SprintfToBuffer_t)(char *buf, const char *fmt);
/* (Wave_Fn_t typedef lives near top) */
typedef void (__thiscall *Scene_RegisterObject_t)(void *gfx, int playerSlot, int *obj);
typedef void (__thiscall *AthenaList_RemoveByValue_t)(void *list, int item);
typedef void (__fastcall *NeonPlatform_Activate_t)(int obj);
typedef void (__thiscall *SquareWobbly_Activate_t)(void *obj, int ball);
typedef void (__thiscall *Wavy_Activate_t)(void *obj, int ball);
typedef void (__thiscall *Spinner_Activate_t)(void *obj, int ball);
typedef void (__thiscall *Gear_AddBall_t)(void *obj, int ball);
typedef void (__fastcall *NormalGravityReset_t)(int ball);
typedef void (__fastcall *DropLift_Activate_t)(int obj);
typedef int (__thiscall *CPUID_RNG_Fn_t)(void *ptr, int range, int flag);

/* Collision handler function pointers */
static Sound_PlayChannel_t          g_SoundPlayChannel = NULL;
static Ball_DizzyImmunity_t         g_BallDizzyImmunity = NULL;
static Ball_Grow_t                  g_BallGrow = NULL;
static Ball_Shrink_t                g_BallShrink = NULL;
static Ball_SetName_t               g_BallSetName = NULL;
static Ball_ApplyTrajectory_t       g_BallApplyTrajectory = NULL;
static Ball_SetTiltedGravity_t      g_BallSetTiltedGravity = NULL;
static Ball_SetFlatGravity_t        g_BallSetFlatGravity = NULL;
static Rotator_MarkTriggered_t      g_RotatorMarkTriggered = NULL;
static Rotator_PlayCollisionSound_t g_RotatorPlayCollisionSound = NULL;
static Rotator_TriggerSound_t       g_RotatorTriggerSound = NULL;
static Rotator_StartSound_t         g_RotatorStartSound = NULL;
static Rotator_AddBall_t            g_RotatorAddBall = NULL;
static CheckArenaUnlock_t           g_CheckArenaUnlock = NULL;
static Catapult_Launch_t            g_CatapultLaunch = NULL;
static Trapdoor_Open_t              g_TrapdoorOpen = NULL;
static Trapdoor_Activate_t          g_TrapdoorActivate = NULL;
static Saw_AlertActivate_t          g_SawAlertActivate = NULL;
static Saw_Activate_t               g_SawActivate = NULL;
static Bell_Activate_t              g_BellActivate = NULL;
static Judge_Reset_t                g_JudgeReset = NULL;
static ScoreDisplay_SetTime_t       g_ScoreDisplaySetTime = NULL;
static CreateBonkPopup_t            g_CreateBonkPopup = NULL;
static Hammer_ChaseStart_t          g_HammerChaseStart = NULL;
static Pendulum_PlayCollisionSound_t g_PendulumPlayCollisionSound = NULL;
static Pendulum_AddIndex_t          g_PendulumAddIndex = NULL;
static ScoreObject_ctor_t           g_ScoreObjectCtor = NULL;
static Timer_Decrement_t            g_TimerDecrement = NULL;
static ArenaScoreParticle_ctor_t    g_ArenaScoreParticleCtor = NULL;
static AthenaHashTable_Lookup_t    g_AthenaHashTableLookup = NULL;
static AthenaList_ContainsValue_t  g_AthenaListContainsValue = NULL;
static SceneObject_sub1_ctor_t     g_SceneObjectSub1Ctor = NULL;
static AthenaString_Set_t          g_AthenaStringSet = NULL;
static MWParser_ReadTag_t          g_MWParserReadTag = NULL;
static StreamReader_dtor_t         g_StreamReaderDtor = NULL;
static Audio_PlayMusic_t           g_AudioPlayMusic = NULL;
static Difficulty_GetTimeModifier_t g_DifficultyGetTimeModifier = NULL;
static AthenaString_Format_t       g_AthenaStringFormat = NULL;
static AthenaString_SprintfToBuffer_t g_AthenaStringSprintfToBuffer = NULL;
static Wave_Fn_t                   g_WaveCos = NULL;
/* g_WaveSin forward-declared near top (used by UniversalRenderImpl Tower bob) */
/* (Wave_Fn_t typedef lives near top) */
static Scene_RegisterObject_t      g_SceneRegisterObject = NULL;
static AthenaList_RemoveByValue_t  g_AthenaListRemoveByValue = NULL;
static NeonPlatform_Activate_t     g_NeonPlatformActivate = NULL;
static SquareWobbly_Activate_t     g_SquareWobblyActivate = NULL;
static Wavy_Activate_t             g_WavyActivate = NULL;
static Spinner_Activate_t          g_SpinnerActivate = NULL;
static Gear_AddBall_t              g_GearAddBall = NULL;
static NormalGravityReset_t        g_NormalGravityReset = NULL;
static DropLift_Activate_t         g_DropLiftActivate = NULL;
static CPUID_RNG_Fn_t              g_CPUIDRNG = NULL;

/* ═══════════════════════════════════════════════════════════════════════════
 * The merged UniversalDispatchCollision — handles ALL collision events from
 * all 15 levels, then calls DispatchCollisionEvents for global events.
 * ═══════════════════════════════════════════════════════════════════════════ */

void __thiscall UniversalDispatchCollision(void *board, int *ball, int *collPair) {
    if (!board || !ball || !collPair) goto call_global;
    void* ext = GetBoardExt(board);
    if (!ext) ext = EnsureBoardExt(board);
    if (!ext) goto call_global;
    int level = GetCurrentLevel(board);
    if (level == 0 || level > 15) goto call_global;

    DWORD *cp = (DWORD *)collPair;
    char *name = NULL;
    DWORD meshBuf = cp[1]; /* collPair[1] = mesh buffer */
    if (meshBuf && !IsBadReadPtr((void *)meshBuf, 0x868)) {
        name = *(char **)(meshBuf + 0x864);
        if (!name || IsBadReadPtr(name, 4)) name = NULL;
    }
    if (!name) goto call_global;

    DWORD app = *(DWORD *)((char *)board + BOARD_APP_PTR);
    int difficulty = (app && !IsBadReadPtr((void *)app, 0x800)) ? *(int *)(app + APP_DIFFICULTY) : 0;
    DWORD sceneObj = cp[0]; /* *collPair = scene object pointer */
    DWORD meshObj = (sceneObj && !IsBadReadPtr((void *)sceneObj, 0x500)) ? *(DWORD *)(sceneObj + 0x47C) : 0;

    /* ── Beginner/Toob/Master: N:BUMPER ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_BUMPER)) && my_strnicmp(name, "N:BUMPER", 8) == 0) {
        float px = *(float *)((char *)ball + 0x164);
        float py = *(float *)((char *)ball + 0x168);
        float pz = *(float *)((char *)ball + 0x16C);
        if (g_SoundPlay3D && app) {
            DWORD snd = *(DWORD *)(app + 0x448);
            if (snd) g_SoundPlay3D((void *)snd, px, py, pz);
        }
        /* Per-level velocity scale and max speed (verified via Ghidra):
         * Beginner/Toob: 4.0 scale, 10.0 max (_DAT_004cf41c, _DAT_004cf9f8)
         * Master:        5.0 scale, 12.0 max (_DAT_004cf55c, _DAT_004cf3dc) */
        float velScale = (level == 14) ? 5.0f : 4.0f;
        float maxSpeed = (level == 14) ? 12.0f : 10.0f;
        int phys = ball[0x69];
        if (phys && !IsBadReadPtr((void *)phys, 0xCB0)) {
            float vx = *(float *)(phys + 0xCA4);
            float vz = *(float *)(phys + 0xCAC);
            float vy = 0.0f;
            vx *= velScale; vz *= velScale;
            float speedSq = vx*vx + vz*vz;
            if (speedSq < 25.0f) {
                if (speedSq > 0.0001f) {
                    float s = 5.0f / sqrtf(speedSq);
                    vx *= s; vz *= s;
                }
            }
            speedSq = vx*vx + vz*vz;
            if (speedSq > maxSpeed * maxSpeed) {
                float s = maxSpeed / sqrtf(speedSq);
                vx *= s; vz *= s;
            }
            *(float *)(phys + 0xCA4) = vx;
            *(float *)(phys + 0xCA8) = vy;
            *(float *)(phys + 0xCAC) = vz;
        }
        long bumperNum = atol(name + 8);
        // Orig writes slot 0 (atol("")=0 -> board+0x6428) but slot 0 is never decayed
        // (decay covers 0x642C+) nor rendered (render reads 0..7 of lit 1..8); no vanilla
        // file uses bare N:BUMPER, so skipping the write is behavior-preserving.
        if (name[8] < '1' || name[8] > '8' || name[9] != '\0') goto call_global;
        bumperNum = name[8] - '0';
        DWORD litBase = UNI_BUMPER_LIT;
        // BUMPER_LIT is unified -> ext ; adjust for 1-based naming: lit index = bumperNum-1
        *(DWORD *)((char *)ext + (bumperNum-1) * 4 + litBase) = 0x3F800000;
    }

    /* ── Intermediate: N:BRIDGE ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_BRIDGE)) && my_stricmp(name, "N:BRIDGE") == 0) {
        if (*(int *)((char *)ext + UNI_BRIDGE_STATE) == 3) {
            *(BYTE *)((char *)ball + 0x778) = 1;
        }
        /* Original Intermediate handler returns for N:BRIDGE regardless of
         * state — does NOT call DispatchCollisionEvents. Since N:BRIDGE is
         * not a global event, skipping call_global is equivalent. */
        return;
    }

    /* ── Dizzy: N:WATERWHEEL ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_WATERWHEEL)) && my_strnicmp(name, "N:WATERWHEEL", 12) == 0) {
        *(BYTE *)((char *)ball + 0x778) = 1;
        return; /* Dizzy returns early */
    }

    /* ── Dizzy: N:WHEELEMBED ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_WHEELEMBED)) && my_stricmp(name, "N:WHEELEMBED") == 0) {
        float dx = *(float *)((char *)ball + 0x164) - *(float *)((char *)ext + UNI_WHEELEMBED_X);
        float dy = *(float *)((char *)ball + 0x168) - *(float *)((char *)ext + UNI_WHEELEMBED_Y);
        float dz = *(float *)((char *)ball + 0x16C) - *(float *)((char *)ext + UNI_WHEELEMBED_Z);
        /* Transform via Gfx_ScaleY(-5.0) + Matrix_TransformVec3 */
        if (g_TimerInit && g_TimerCleanup && g_GfxScaleY && g_MatrixTransformVec3 && g_GfxSetPosition && app) {
            void *gfx = *(void **)(app + 0x174);
            if (gfx) {
            char timerBuf[68];
            float transformed[16];
            g_TimerInit(timerBuf);
            g_GfxScaleY(gfx, -5.0f);
            g_MatrixTransformVec3(transformed, &dx);
            dx += *(float *)((char *)ext + UNI_WHEELEMBED_X);
            dy += *(float *)((char *)ext + UNI_WHEELEMBED_Y);
            dz += *(float *)((char *)ext + UNI_WHEELEMBED_Z);
            g_TimerCleanup(timerBuf);
            }
        }
        *(BYTE *)((char *)ball + 0xC3C) = 1;
        *(float *)((char *)ball + 0xC40) = dx;
        *(float *)((char *)ball + 0xC44) = dy;
        *(float *)((char *)ball + 0xC48) = dz;
        *(int *)((char *)ball + 0x808) = 0x32;
        return; /* Dizzy returns early */
    }

    /* ── Dizzy: N:SWIRL ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_SWIRL)) && my_stricmp(name, "N:SWIRL") == 0) {
        *(BYTE *)((char *)ball + 0x779) = 1;
        return; /* Dizzy returns early */
    }

    /* ── Tower/Master: E:CATAPULTBOTTOM ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_CATAPULTBOTTOM)) && my_stricmp(name, "E:CATAPULTBOTTOM") == 0) {
        if (*(int *)((char *)ball + 0x808) < 1) {
            *(int *)((char *)ball + 0x808) = 1000;
            DWORD catList = UNI_CATAPULT_LIST;
            DWORD catCount = UNI_CATAPULT_COUNT;
            DWORD catData = UNI_CATAPULT_DATA;
            int iter = g_AthenaListGetIterator((void *)((char *)ext + catList));
            *(DWORD *)((char *)ext + catList + 8 + iter * 4) = 0;
            int count = *(int *)((char *)ext + catCount);
            int item = 0;
            if (count > 0) {
                item = **(int **)((char *)ext + catData);
                *(DWORD *)((char *)ext + catList + 8 + iter * 4) = 1;
            }
            while (item) {
                if (*(int *)(item + 0x10D4) == sceneObj) {
                    *(int **)(item + 0x10EC) = ball;
                    if (g_CatapultLaunch) g_CatapultLaunch(item);
                    if (g_SoundPlayChannel && app) {
                        int ch = *(int *)(app + 0x464);
                        if (ch) g_SoundPlayChannel(ch);
                    }
                }
                int next = *(int *)((char *)ext + catList + 8 + iter * 4);
                if (*(int *)((char *)ext + catCount) <= next) break;
                item = *(int *)(*(int **)((char *)ext + catData) + next * 4);
                *(int *)((char *)ext + catList + 8 + iter * 4) = next + 1;
            }
        }
    }

    /* ── Tower: E:OPENSESAME ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_OPENSESAME)) && my_stricmp(name, "E:OPENSESAME") == 0) {
        if (g_TrapdoorOpen) {
            int count = *(int *)((char *)ext + UNI_DRAWBRIDGE_COUNT);
            int item = 0;
            if (count >= 1) item = **(int **)((char *)ext + UNI_DRAWBRIDGE_DATA);
            g_TrapdoorOpen(item);
        }
    }

    /* ── Tower: N:TRAPDOOR ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_TRAPDOOR)) && my_stricmp(name, "N:TRAPDOOR") == 0) {
        int iter = g_AthenaListGetIterator((void *)((char *)ext + UNI_TRAPDOOR_LIST));
        *(DWORD *)((char *)ext + UNI_TRAPDOOR_LIST + 8 + iter * 4) = 0;
        int count = *(int *)((char *)ext + UNI_TRAPDOOR_COUNT);
        int item = 0;
        if (count >= 1) {
            item = **(int **)((char *)ext + UNI_TRAPDOOR_DATA);
            *(DWORD *)((char *)ext + UNI_TRAPDOOR_LIST + 8 + iter * 4) = 1;
        }
        while (item) {
            if (*(int *)(item + 0x10D4) == sceneObj || *(int *)(item + 0x10DC) == sceneObj) {
                if (g_TrapdoorActivate) g_TrapdoorActivate(item);
            }
            int next = *(int *)((char *)ext + UNI_TRAPDOOR_LIST + 8 + iter * 4);
            if (*(int *)((char *)ext + UNI_TRAPDOOR_COUNT) <= next) break;
            item = *(int *)(*(int **)((char *)ext + UNI_TRAPDOOR_DATA) + next * 4);
            *(int *)((char *)ext + UNI_TRAPDOOR_LIST + 8 + iter * 4) = next + 1;
        }
    }

    /* ── Tower: E:BITE ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_BITE)) && my_stricmp(name, "E:BITE") == 0) {
        *(DWORD *)((char *)ext + UNI_BITE_STATE) = 0;
        *(DWORD *)((char *)ext + UNI_BITE_SPEED) = 0x41C80000;
    }

    /* ── Tower: E:MACETRIGGER ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_MACETRIGGER)) && my_stricmp(name, "E:MACETRIGGER") == 0) {
        int iter = g_AthenaListGetIterator((void *)((char *)ext + UNI_MACE_LIST));
        *(DWORD *)((char *)ext + UNI_MACE_LIST + 8 + iter * 4) = 0;
        int count = *(int *)((char *)ext + UNI_MACE_COUNT);
        int item = 0;
        if (count >= 1) {
            item = **(int **)((char *)ext + UNI_MACE_DATA);
            *(DWORD *)((char *)ext + UNI_MACE_LIST + 8 + iter * 4) = 1;
        }
        while (item) {
            *(BYTE *)(item + 0x10F0) = 1;
            int next = *(int *)((char *)ext + UNI_MACE_LIST + 8 + iter * 4);
            if (*(int *)((char *)ext + UNI_MACE_COUNT) <= next) break;
            item = *(int *)(*(int **)((char *)ext + UNI_MACE_DATA) + next * 4);
            *(int *)((char *)ext + UNI_MACE_LIST + 8 + iter * 4) = next + 1;
        }
    }

    /* ── Tower: N:MACE ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_MACE)) && my_stricmp(name, "N:MACE") == 0) {
        int iter = g_AthenaListGetIterator((void *)((char *)ext + UNI_MACE_LIST));
        *(DWORD *)((char *)ext + UNI_MACE_LIST + 8 + iter * 4) = 0;
        int count = *(int *)((char *)ext + UNI_MACE_COUNT);
        int item = 0;
        if (count >= 1) {
            item = **(int **)((char *)ext + UNI_MACE_DATA);
            *(DWORD *)((char *)ext + UNI_MACE_LIST + 8 + iter * 4) = 1;
        }
        while (item) {
            if (*(int *)(item + 0x10D4) == sceneObj &&
                *(int *)(item + 0x10E8) != 0x42A00000 &&
                *(int *)(item + 0x10F4) == 0) {
                /* Call ball vtable[+0x20] (break) */
                DWORD *vtbl = *(DWORD **)ball;
                if (vtbl) {
                    void (__thiscall *fn)(int *) = (void (__thiscall *)(int *))vtbl[8];
                    if (fn) fn(ball);
                }
            }
            int next = *(int *)((char *)ext + UNI_MACE_LIST + 8 + iter * 4);
            if (*(int *)((char *)ext + UNI_MACE_COUNT) <= next) break;
            item = *(int *)(*(int **)((char *)ext + UNI_MACE_DATA) + next * 4);
            *(int *)((char *)ext + UNI_MACE_LIST + 8 + iter * 4) = next + 1;
        }
    }

    /* ── Up/Impossible: E:HELPINERTIA ──
     * Impossible gates this on (char)ball[0x768] != 0; Up does not. */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_HELPINERTIA)) && my_stricmp(name, "E:HELPINERTIA") == 0) {
        if (level != 15 || (char)*(int *)((char *)ball + 0x768))
            ball[0xA9] = 0x40200000;
    }

    /* ── Up/Impossible: E:UNHELPINERTIA ──
     * Impossible gates this on (char)ball[0x768] != 0; Up does not. */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_UNHELPINERTIA)) && my_stricmp(name, "E:UNHELPINERTIA") == 0) {
        if (level != 15 || (char)*(int *)((char *)ball + 0x768))
            ball[0xA9] = 0x40A00000;
    }

    /* ── Up: E:VACPOPOUT ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_VACPOPOUT)) && my_stricmp(name, "E:VACPOPOUT") == 0) {
        ball[0xA1] = 0x41D00000;
        if (g_SoundPlay3D && app) {
            DWORD snd = *(DWORD *)(app + 0x468);
            if (snd) g_SoundPlay3D((void *)snd, *(float *)((char *)ball + 0x164), *(float *)((char *)ball + 0x168), *(float *)((char *)ball + 0x16C));
        }
    }

    /* ── Up: N:SPEEDCYLINDER ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_SPEEDCYLINDER)) && my_stricmp(name, "N:SPEEDCYLINDER") == 0) {
        if (g_PendulumPlayCollisionSound && meshObj)
            g_PendulumPlayCollisionSound((void *)meshObj, (int)ball);
    }

    /* ── Up: N:EXTRATIME ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_EXTRATIME)) && my_stricmp(name, "N:EXTRATIME") == 0) {
        if (meshObj && *(char *)(meshObj + 0x10E4) == 0) {
            if (g_RotatorTriggerSound) g_RotatorTriggerSound(meshObj);
            if (app) {
                int gameMode = *(int *)(app + 0x220);
                if (gameMode && *(char *)(gameMode + 0x10) == 0 && *(char *)(gameMode + 0x11) == 0) {
                    *(DWORD *)(ball[6]*0xA0 + 0x5EC + app) = 500;
                    if (g_operatorNew && g_ScoreObjectCtor && g_TimerDecrement && g_AthenaListAppend) {
                        void *mem = g_operatorNew(0x30);
                        if (mem) {
                            int so = g_ScoreObjectCtor(mem, (int)board, ball[6]*0xA0 + 0x5CC + app, "EXTRA TIME:");
                            g_TimerDecrement(so);
                            g_AthenaListAppend((void *)((char *)board + UNI_SCORE_LIST), so);
                        }
                    }
                }
            }
        }
    }

    /* ── Neon: N:NEONPLATFORM ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_NEONPLATFORM)) && my_strnicmp(name, "N:NEONPLATFORM", 14) == 0) {
        if (g_NeonPlatformActivate && meshObj) g_NeonPlatformActivate(meshObj);
    }

    /* ── Neon: E:ZOOP ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_ZOOP)) && my_strnicmp(name, "E:ZOOP", 6) == 0) {
        if (*(int *)((char *)ball + 0x7F0) == 0) {
            if (g_SoundPlay3D && app) {
                DWORD snd = *(DWORD *)(app + 0x524);
                if (snd) g_SoundPlay3D((void *)snd, *(float *)((char *)ball + 0x164), *(float *)((char *)ball + 0x168), *(float *)((char *)ball + 0x16C));
            }
            *(int *)((char *)ball + 0x7F0) = 100;
        }
    }

    /* ── Neon: E:LIGHTSOFF ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_LIGHTSOFF)) && my_strnicmp(name, "E:LIGHTSOFF", 10) == 0) {
        if (*(int *)((char *)ball + 0x7B4) == 0) {
            if (g_SoundPlay3D && app) {
                DWORD snd = *(DWORD *)(app + 0x528);
                if (snd) g_SoundPlay3D((void *)snd, *(float *)((char *)ball + 0x164), *(float *)((char *)ball + 0x168), *(float *)((char *)ball + 0x16C));
            }
            /* Call vtable[+0x10](0) on player's render obj */
            void *lightExt = GetBoardExt(board); if (!lightExt) lightExt = ext;
            DWORD *renderObj = lightExt ? *(DWORD **)((char *)lightExt + ball[6]*4 + UNI_BONK_STORE) : NULL;
            if (!renderObj && lightExt) renderObj = *(DWORD **)((char *)lightExt + UNI_BONK_STORE);
            if (renderObj) {
                DWORD *vtbl = *(DWORD **)renderObj;
                if (vtbl) {
                    void (__thiscall *fn)(DWORD, int) = (void (__thiscall *)(DWORD, int))vtbl[4];
                    if (fn) fn((DWORD)renderObj, 0);
                }
                if (g_SceneRegisterObject && app) {
                    void *gfx = *(void **)(app + 0x174);
                    if (gfx) g_SceneRegisterObject(gfx, ball[6], (int *)renderObj);
                }
            }
            if (*(int *)((char *)ext + UNI_NEON_TRAPDOOR) == 0 && g_AthenaListAppend) {
                g_AthenaListAppend((void *)((char *)board + UNI_OBJ_LIST), *(int *)((char *)ext + UNI_NEON_DARK_COUNT));
            }
            *(int *)((char *)ext + UNI_NEON_TRAPDOOR) += 1;
            *(int *)((char *)ball + 0x7B4) = 100;
        }
    }

    /* ── Neon: E:LIGHTSON ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_LIGHTSON)) && my_strnicmp(name, "E:LIGHTSON", 10) == 0) {
        if (*(int *)((char *)ball + 0x7B8) == 0) {
            if (g_SoundPlay3D && app) {
                DWORD snd = *(DWORD *)(app + 0x528);
                if (snd) g_SoundPlay3D((void *)snd, *(float *)((char *)ball + 0x164), *(float *)((char *)ball + 0x168), *(float *)((char *)ball + 0x16C));
            }
            void *lightExt2 = GetBoardExt(board); if (!lightExt2) lightExt2 = ext;
            DWORD *renderObj = lightExt2 ? *(DWORD **)((char *)lightExt2 + ball[6]*4 + UNI_BONK_STORE) : NULL;
            if (!renderObj && lightExt2) renderObj = *(DWORD **)((char *)lightExt2 + UNI_BONK_STORE);
            if (renderObj) {
                DWORD *vtbl = *(DWORD **)renderObj;
                if (vtbl) {
                    void (__thiscall *fn)(DWORD, int) = (void (__thiscall *)(DWORD, int))vtbl[4];
                    if (fn) fn((DWORD)renderObj, 1);
                }
                if (g_SceneRegisterObject && app) {
                    void *gfx = *(void **)(app + 0x174);
                    if (gfx) g_SceneRegisterObject(gfx, ball[6], (int *)renderObj);
                }
            }
            *(int *)((char *)ball + 0x7B8) = 100;
            int n = *(int *)((char *)ext + UNI_NEON_TRAPDOOR) - 1;
            *(int *)((char *)ext + UNI_NEON_TRAPDOOR) = n;
            if (n < 1 && g_AthenaListRemoveByValue) {
                *(int *)((char *)ext + UNI_NEON_TRAPDOOR) = 0;
                g_AthenaListRemoveByValue((void *)((char *)board + UNI_OBJ_LIST), *(int *)((char *)ext + UNI_NEON_DARK_COUNT));
                DWORD trapObj = *(DWORD *)((char *)ext + UNI_NEON_DARK_COUNT);
                if (trapObj) {
                    *(DWORD *)(trapObj + 0x10DC) = 2;
                    *(DWORD *)(trapObj + 0x10E0) = 0;
                }
            }
        }
    }

    /* ── Expert/Master: E:CALLHAMMER ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_CALLHAMMER)) && my_stricmp(name, "E:CALLHAMMER") == 0) {
        if (difficulty != 0 && g_CreateBonkPopup) {
            void *be = GetBoardExt(board); if (!be) be = ext;
            int bonkObj = be ? *(int*)((char*)be + UNI_BONK_MESH) : *(int*)((char*)ext + UNI_BONK_MESH);
            if (!bonkObj) bonkObj = be ? *(int*)((char*)be + UNI_BONK_STORE) : *(int*)((char*)ext + UNI_BONK_STORE);
            g_CreateBonkPopup(bonkObj);
        }
    }

    /* ── Expert/Master: E:HAMMERCHASE ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_HAMMERCHASE)) && my_stricmp(name, "E:HAMMERCHASE") == 0) {
        if (difficulty != 0 && g_HammerChaseStart) {
            void *be2 = GetBoardExt(board); if (!be2) be2 = ext;
            int bonkObj2 = be2 ? *(int*)((char*)be2 + UNI_BONK_MESH) : *(int*)((char*)ext + UNI_BONK_MESH);
            if (!bonkObj2) bonkObj2 = be2 ? *(int*)((char*)be2 + UNI_BONK_STORE) : *(int*)((char*)ext + UNI_BONK_STORE);
            g_HammerChaseStart(bonkObj2);
        }
    }

    /* ── Expert: E:ALERTSAW1 ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_ALERTSAW1)) && my_stricmp(name, "E:ALERTSAW1") == 0) {
        if (difficulty != 0 && g_SawAlertActivate) {
            int saw1 = *(int *)((char *)ext + UNI_SAWBLADE1_OBJ);
            if (!saw1) saw1 = *(int *)((char *)ext + UNI_SAW1_OBJ);
            g_SawAlertActivate(saw1);
        }
    }

    /* ── Expert: E:ALERTSAW2 ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_ALERTSAW2)) && my_stricmp(name, "E:ALERTSAW2") == 0) {
        if (difficulty != 0 && g_SawAlertActivate) {
            int saw2 = *(int *)((char *)ext + UNI_SAWBLADE2_OBJ);
            if (!saw2) saw2 = *(int *)((char *)ext + UNI_SAW2_OBJ);
            g_SawAlertActivate(saw2);
        }
    }

    /* ── Toob: E:ALERTSAW2 vanilla + E:ALERTSAW3 renamed ── — vanilla Toob files use ALERTSAW2 */
    if ((((*(BYTE*)((char*)ext + COLL_FLAG_ALERTSAW3)) && my_stricmp(name, "E:ALERTSAW3") == 0)) || (GetCurrentLevel(board) == 10 && my_stricmp(name, "E:ALERTSAW2") == 0)) {
        if (difficulty != 0) {
            int saw2Obj = *(int *)((char *)ext + UNI_SAW2_TOOB_OBJ);
            if (saw2Obj) *(BYTE *)(saw2Obj + 0x110C) = 1;
        }
    }

    /* ── Expert: E:ACTIVATESAW1 (fallback to B8F4 dedup) ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_ACTIVATESAW1)) && my_stricmp(name, "E:ACTIVATESAW1") == 0) {
        if (difficulty != 0 && g_SawActivate) {
            int saw1 = *(int *)((char *)ext + UNI_SAWBLADE1_OBJ);
            if (!saw1) saw1 = *(int *)((char *)ext + UNI_SAW1_OBJ);
            g_SawActivate(saw1);
        }
    }

    /* ── Expert: E:ACTIVATESAW2 ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_ACTIVATESAW2)) && my_stricmp(name, "E:ACTIVATESAW2") == 0) {
        if (difficulty != 0 && g_SawActivate) {
            int saw2 = *(int *)((char *)ext + UNI_SAWBLADE2_OBJ);
            if (!saw2) saw2 = *(int *)((char *)ext + UNI_SAW2_OBJ);
            g_SawActivate(saw2);
        }
    }

    /* ── Expert: E:ALERTJUDGES ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_ALERTJUDGES)) && my_stricmp(name, "E:ALERTJUDGES") == 0) {
        int iter = g_AthenaListGetIterator((void *)((char *)ext + UNI_JUDGE_LIST));
        *(DWORD *)((char *)ext + UNI_JUDGE_LIST + 8 + iter * 4) = 0;
        int count = *(int *)((char *)ext + UNI_JUDGE_COUNT);
        int item = 0;
        if (count >= 1) {
            item = **(int **)((char *)ext + UNI_JUDGE_DATA);
            *(DWORD *)((char *)ext + UNI_JUDGE_LIST + 8 + iter * 4) = 1;
        }
        while (item) {
            if (g_JudgeReset) g_JudgeReset(item);
            int next = *(int *)((char *)ext + UNI_JUDGE_LIST + 8 + iter * 4);
            if (*(int *)((char *)ext + UNI_JUDGE_COUNT) <= next) break;
            item = *(int *)(*(int **)((char *)ext + UNI_JUDGE_DATA) + next * 4);
            *(int *)((char *)ext + UNI_JUDGE_LIST + 8 + iter * 4) = next + 1;
        }
    }

    /* ── Expert: E:SCORE ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_SCORE)) && my_strnicmp(name, "E:SCORE", 7) == 0) {
        int iter = g_AthenaListGetIterator((void *)((char *)ext + UNI_JUDGE_LIST));
        *(DWORD *)((char *)ext + UNI_JUDGE_LIST + 8 + iter * 4) = 0;
        int count = *(int *)((char *)ext + UNI_JUDGE_COUNT);
        void *item = NULL;
        if (count >= 1) {
            item = (void *)**(int **)((char *)ext + UNI_JUDGE_DATA);
            *(DWORD *)((char *)ext + UNI_JUDGE_LIST + 8 + iter * 4) = 1;
        }
        while (item) {
            long score = atol(name + 7);
            if (g_ScoreDisplaySetTime) g_ScoreDisplaySetTime(item, score);
            int next = *(int *)((char *)ext + UNI_JUDGE_LIST + 8 + iter * 4);
            if (*(int *)((char *)ext + UNI_JUDGE_COUNT) <= next) break;
            item = *(void **)(*(int **)((char *)ext + UNI_JUDGE_DATA) + next * 4);
            *(int *)((char *)ext + UNI_JUDGE_LIST + 8 + iter * 4) = next + 1;
        }
    }

    /* ── Expert: E:JUMP is identical to the global E:JUMP in DispatchCollisionEvents.
     *   No level-specific handler needed — the global handler at the end handles it. ── */

    /* ── Expert: E:BELL ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_BELL)) && my_strnicmp(name, "E:BELL", 6) == 0) {
        if (g_BellActivate) g_BellActivate(*(int *)((char *)ext + UNI_BELL_OBJ));
        if (app) {
            int gameMode = *(int *)(app + 0x220);
            if (gameMode && *(char *)(gameMode + 0x10) == 0 && *(char *)(gameMode + 0x11) == 0) {
                *(DWORD *)(ball[6]*0xA0 + 0x5EC + app) = 500;
                if (g_operatorNew && g_ScoreObjectCtor && g_TimerDecrement && g_AthenaListAppend) {
                    void *mem = g_operatorNew(0x30);
                    if (mem) {
                        int so = g_ScoreObjectCtor(mem, (int)board, ball[6]*0xA0 + 0x5CC + app, "EXTRA TIME:");
                        g_TimerDecrement(so);
                        g_AthenaListAppend((void *)((char *)board + UNI_SCORE_LIST), so);
                    }
                }
            }
        }
    }

    /* ── Odd: E:GRAVITY ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_GRAVITY)) && my_strnicmp(name, "E:GRAVITY", 9) == 0) {
        if (g_SceneObjectSub1Ctor && g_AthenaStringSet && g_MWParserReadTag && g_StreamReaderDtor) {
            char strObj[64];
            g_SceneObjectSub1Ctor(strObj);
            g_AthenaStringSet(strObj, name);
            DWORD *tag;
            while ((tag = (DWORD *)g_MWParserReadTag(strObj)) != NULL) {
                if (my_stricmp((char *)tag[1], "TYPE") == 0) {
                    if (my_stricmp((char *)tag[2], "NORMAL") == 0 && g_NormalGravityReset)
                        g_NormalGravityReset((int)ball);
                    if (my_stricmp((char *)tag[2], "X") == 0 && g_BallSetTiltedGravity)
                        g_BallSetTiltedGravity((int)ball);
                    if (my_stricmp((char *)tag[2], "Z") == 0 && g_BallSetFlatGravity)
                        g_BallSetFlatGravity((int)ball);
                }
                /* Original game: (**(code **)*puVar4)(1) — double dereference.
                 * tag[0] is a pointer to a vtable; vtable[0] is the destructor. */
                (*(void (__thiscall **)(DWORD))(*(DWORD*)tag[0]))(1);
            }
            g_StreamReaderDtor(strObj);
        }
    }

    /* ── Odd: N:JUMPFIRST / N:JUMPSECOND ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_JUMPFIRST)) && my_stricmp(name, "N:JUMPFIRST") == 0) {
        /* Uses AthenaHashTable_Lookup for "JUMPPIPE1" */
        float pos[3] = {0,0,0};
        if (g_AthenaHashTableLookup && app) {
            void *ht = *(void **)((char *)board + BOARD_MESHWORLD);
            if (ht) g_AthenaHashTableLookup(ht, pos, "JUMPPIPE1", NULL);
        }
        *(BYTE *)((char *)ball + 0xC3C) = 1;
        *(float *)((char *)ball + 0xC40) = pos[0];
        *(float *)((char *)ball + 0xC44) = pos[1];
        *(float *)((char *)ball + 0xC48) = pos[2];
        int phys = ball[0x69];
        if (phys && !IsBadReadPtr((void *)phys, 0xCB0)) {
            *(float *)(phys + 0xCA4) = 0.0f;
            *(DWORD *)(phys + 0xCA8) = 0x41800000;
            *(DWORD *)(phys + 0xCAC) = 0;
        }
    }
    if ((*(BYTE*)((char*)ext + COLL_FLAG_JUMPSECOND)) && my_stricmp(name, "N:JUMPSECOND") == 0) {
        float pos[3] = {0,0,0};
        if (g_AthenaHashTableLookup && app) {
            void *ht = *(void **)((char *)board + BOARD_MESHWORLD);
            if (ht) g_AthenaHashTableLookup(ht, pos, "JUMPPIPE2", NULL);
        }
        *(BYTE *)((char *)ball + 0xC3C) = 1;
        *(float *)((char *)ball + 0xC40) = pos[0];
        *(float *)((char *)ball + 0xC44) = pos[1];
        *(float *)((char *)ball + 0xC48) = pos[2];
        int phys = ball[0x69];
        if (phys && !IsBadReadPtr((void *)phys, 0xCB0)) {
            *(float *)(phys + 0xCA4) = 0.0f;
            *(DWORD *)(phys + 0xCA8) = 0x41800000;
            *(DWORD *)(phys + 0xCAC) = 0;
        }
    }

    /* ── Odd: E:SHRINK ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_SHRINK)) && my_stricmp(name, "E:SHRINK") == 0) {
        if (g_BallShrink) g_BallShrink((int)ball);
        float pos[3] = {0,0,0};
        if (g_AthenaHashTableLookup) {
            void *ht = *(void **)((char *)board + BOARD_MESHWORLD);
            if (ht) g_AthenaHashTableLookup(ht, pos, "SHRINKCENTER", NULL);
        }
        *(BYTE *)((char *)ball + 0xC3C) = 1;
        *(float *)((char *)ball + 0xC40) = pos[0];
        *(float *)((char *)ball + 0xC44) = pos[1];
        *(float *)((char *)ball + 0xC48) = pos[2];
        int phys = ball[0x69];
        if (phys && !IsBadReadPtr((void *)phys, 0xCB0)) {
            *(float *)(phys + 0xCA4) = 0.0f;
            *(DWORD *)(phys + 0xCA8) = 0xBF800000;
            *(DWORD *)(phys + 0xCAC) = 0;
        }
    }

    /* ── Odd: E:GROWSOUND ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_GROWSOUND)) && my_stricmp(name, "E:GROWSOUND") == 0) {
        if (*(int *)((char *)ball + 0x7F8) == 0) {
            if (g_SoundPlayChannel && app) {
                int ch = *(int *)(app + 0x4D8);
                if (ch) g_SoundPlayChannel(ch);
            }
        }
        *(int *)((char *)ball + 0x7F8) = 100;
    }

    /* ── Odd: E:GROW ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_GROW)) && my_stricmp(name, "E:GROW") == 0) {
        if (g_BallGrow) g_BallGrow((int)ball);
    }

    /* ── Odd: E:DROPLIFT ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_DROPLIFT)) && my_stricmp(name, "E:DROPLIFT") == 0) {
        if (g_DropLiftActivate) g_DropLiftActivate(*(int *)((char *)ext + UNI_BONK_STORE));
    }

    /* ── Odd: E:PIPERANDOM (complex — simplified to core behavior) ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_PIPERANDOM)) && my_stricmp(name, "E:PIPERANDOM") == 0) {
        if (g_BallGrow) g_BallGrow((int)ball);
        if (difficulty != 0) *(BYTE *)((char *)ext + UNI_SAW1_OBJ) = 1;
        /* Random pipe selection */
        if (g_CPUIDRNG && g_AthenaHashTableLookup) {
            void *ht = *(void **)((char *)board + BOARD_MESHWORLD);
            int rng = RNG_call((void *)0x4F7360, 0, 2, 0);
            float pos[3] = {0,0,0};
            if (rng == 0) {
                /* PIPERANDOM1: set byte 4 flag (checked by E:LIMITPIPE1) */
                const char *key = "PIPERANDOM1";
                if (ht) g_AthenaHashTableLookup(ht, pos, key, NULL);
                *(BYTE *)((char *)ball + 4) = 1;
            } else {
                /* PIPERANDOM2: set byte 5 flag (checked by E:LIMITPIPE2) */
                const char *key = "PIPERANDOM2";
                if (ht) g_AthenaHashTableLookup(ht, pos, key, NULL);
                *(BYTE *)((char *)ball + 5) = 1;
            }
            *(BYTE *)((char *)ball + 0xC3C) = 1;
            *(float *)((char *)ball + 0xC40) = pos[0];
            *(float *)((char *)ball + 0xC44) = pos[1];
            *(float *)((char *)ball + 0xC48) = pos[2];
            int phys = ball[0x69];
            if (phys && !IsBadReadPtr((void *)phys, 0xCB0)) {
                *(DWORD *)(phys + 0xCA4) = 0;
                *(DWORD *)(phys + 0xCA8) = 0;
                *(DWORD *)(phys + 0xCAC) = 0;
            }
            if (g_SoundPlay3D && app) {
                DWORD snd = *(DWORD *)(app + 0x468);
                if (snd) g_SoundPlay3D((void *)snd, *(float *)((char *)ball + 0x164), *(float *)((char *)ball + 0x168), *(float *)((char *)ball + 0x16C));
            }
        }
    }

    /* ── Odd: E:LIMIT / E:LIMITX / E:LIMITZ / E:LIMITPIPE1 / E:LIMITPIPE2 / E:SWALLOW ── */
    /* Odd's E:LIMIT checks ball+0x1D2 (axis selector) */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_LIMIT)) && my_stricmp(name, "E:LIMIT") == 0) {
        /* Odd's version: only activates if ball+0x1D2 == 0 */
        if (*(int *)((char *)ball + 0x748) == 0) {
            *(BYTE *)((char *)ball + 0x768) = 0;
            *(BYTE *)((char *)ball + 0x2E9) = 1;
        }
        /* Sky's version: removes from heat list if difficulty != 0 */
        if (difficulty != 0 && g_AthenaListRemoveByValue) {
            DWORD mgObj = *(DWORD *)((char *)ext + UNI_MAGNIFYING_GLASS);
            if (mgObj) g_AthenaListRemoveByValue((void *)(mgObj + 0x2C), (int)ball);
        }
    }
    if ((*(BYTE*)((char*)ext + COLL_FLAG_LIMITX)) && my_stricmp(name, "E:LIMITX") == 0) {
        if (*(int *)((char *)ball + 0x748) == 1) {
            *(BYTE *)((char *)ball + 0x768) = 0;
            *(BYTE *)((char *)ball + 0x2E9) = 1;
        }
    }
    if ((*(BYTE*)((char*)ext + COLL_FLAG_LIMITZ)) && my_stricmp(name, "E:LIMITZ") == 0) {
        if (*(int *)((char *)ball + 0x748) == 2) {
            *(BYTE *)((char *)ball + 0x768) = 0;
            *(BYTE *)((char *)ball + 0x2E9) = 1;
        }
    }
    if ((*(BYTE*)((char*)ext + COLL_FLAG_LIMITPIPE1)) && my_stricmp(name, "E:LIMITPIPE1") == 0) {
        if ((char)ball[1] != 0) {
            *(BYTE *)((char *)ball + 0x768) = 0;
            *(BYTE *)((char *)ball + 0x2E9) = 1;
        }
    }
    if ((*(BYTE*)((char*)ext + COLL_FLAG_SWALLOW)) && my_stricmp(name, "E:SWALLOW") == 0) {
        *(BYTE *)((char *)ball + 0x2E8) = 1;
    }
    if ((*(BYTE*)((char*)ext + COLL_FLAG_LIMITPIPE2)) && my_stricmp(name, "E:LIMITPIPE2") == 0) {
        /* VERIFIED via Ghidra decompilation of OddBoard_CollisionHandler (0x0040ED30):
         * Original: cVar1 = *(char *)((int)param_2 + 5);  — byte offset 5, NOT int* index.
         * This is correct as-is. E:LIMITPIPE1 uses (char)param_2[1] = byte 4 (int* arithmetic).
         * Both offsets match the original game exactly. */
        if (*(char *)((char *)ball + 5) != 0) {
            *(BYTE *)((char *)ball + 0x768) = 0;
            *(BYTE *)((char *)ball + 0x2E9) = 1;
        }
    }

    /* ── Toob: E:BRANCH (pipe branching system) ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_BRANCH)) && my_strnicmp(name, "E:BRANCH", 8) == 0) {
        if (g_AthenaHashTableLookup && g_CPUIDRNG && g_Vec3NormalizeAndScale &&
            g_AthenaListInit && g_AthenaListAppend && g_AthenaListGetSize &&
            g_AthenaListGetIterator && g_operatorNew) {
            void *ht = *(void **)((char *)board + BOARD_MESHWORLD);
            if (ht) {
                /* Local AthenaList for position+vector pairs */
                char listBuf[0x420]; /* AthenaList struct */
                g_AthenaListInit(listBuf, 0);

                char *branchName = name + 2; /* skip "E:" prefix */
                int isBranchB = (my_stricmp(name + 8, "(B)") == 0);

                /* Collect up to 5 POS/VECTOR pairs from hash table */
                int i;
                for (i = 1; i <= 5; i++) {
                    char posKey[64], vecKey[64];
                    sprintf(posKey, "%s-POS%.2d", branchName, i);
                    sprintf(vecKey, "%s-VECTOR%.2d", branchName, i);

                    float pos[3] = {0,0,0};
                    float vec[3] = {0,0,0};
                    char found1 = 0, found2 = 0;

                    g_AthenaHashTableLookup(ht, pos, posKey, &found1);
                    if (!found1) break;
                    g_AthenaHashTableLookup(ht, vec, vecKey, &found2);
                    if (!found2) break;

                    /* Allocate 0x18 bytes: pos[3] + vec[3] */
                    float *entry = (float *)g_operatorNew(0x18);
                    if (entry) {
                        entry[0] = pos[0]; entry[1] = pos[1]; entry[2] = pos[2];
                        entry[3] = vec[0]; entry[4] = vec[1]; entry[5] = vec[2];
                        g_AthenaListAppend(listBuf, (int)entry);
                    }
                }

                /* Pick a random entry and compute direction */
                int count = g_AthenaListGetSize(listBuf);
                if (count > 0) {
                    int rngIdx = RNG_call((void *)0x4F7360, 0, count, 0);
                    if (rngIdx < 0) rngIdx = 0;
                    if (rngIdx >= count) rngIdx = 0;

                    /* Get the entry at rngIdx */
                    int iter = g_AthenaListGetIterator(listBuf);
                    *(int *)(listBuf + 8 + iter * 4) = 0;
                    float *entry = NULL;
                    if (count >= 1) {
                        entry = *(float **)(*(int **)(listBuf + 0x40C));
                        *(int *)(listBuf + 8 + iter * 4) = 1;
                    }
                    /* Walk to the rngIdx-th entry */
                    int idx;
                    for (idx = 0; idx < rngIdx && entry; idx++) {
                        int next = *(int *)(listBuf + 8 + iter * 4);
                        if (count <= next) break;
                        entry = *(float **)(*(int **)(listBuf + 0x40C) + next * 4);
                        *(int *)(listBuf + 8 + iter * 4) = next + 1;
                    }

                    if (entry) {
                        /* direction = vec - pos */
                        float dx = entry[3] - entry[0];
                        float dy = entry[4] - entry[1];
                        float dz = entry[5] - entry[2];
                        float dir[3] = {dx, dy, dz};
                        g_Vec3NormalizeAndScale(dir, 1.0f);

                        /* Double the direction */
                        dx = dir[0] + dir[0];
                        dy = dir[1] + dir[1];
                        dz = dir[2] + dir[2];

                        /* Set trajectory target = entry pos */
                        *(BYTE *)((char *)ball + 0xC3C) = 1;
                        *(float *)((char *)ball + 0xC40) = entry[0];
                        *(float *)((char *)ball + 0xC44) = entry[1] + *(float *)((char *)ball + 0x284) + 0.0078125f;
                        *(float *)((char *)ball + 0xC48) = entry[2];

                        /* For (B) branches, double direction again (4x total) */
                        if (isBranchB) {
                            dx += dx; dy += dy; dz += dz;
                        }

                        /* Set ball velocity */
                        int phys = ball[0x69];
                        if (phys && !IsBadReadPtr((void *)phys, 0xCB0)) {
                            *(float *)(phys + 0xCA4) = dx;
                            *(float *)(phys + 0xCA8) = dy;
                            *(float *)(phys + 0xCAC) = dz;
                        }
                    }
                }

                /* Free all allocated entries */
                {
                    int iter2 = g_AthenaListGetIterator(listBuf);
                    int *iterSlot = (int *)(listBuf + 8 + iter2 * 4);
                    *iterSlot = 0;
                    if (g_AthenaListGetSize(listBuf) >= 1) {
                        void *p = *(void **)(*(int **)(listBuf + 0x40C));
                        *iterSlot = 1;
                        while (p) {
                            /* Free is at 0x4BA74D (free) — but we don't have a pointer.
                             * The original game calls _free directly. We can use our
                             * operator's free or just leak (the game does this too
                             * if Vec3List_Free fails). For safety, use the game's free. */
                            typedef void (__cdecl *free_fn)(void *);
                            static free_fn g_free = NULL;
                            if (!g_free) g_free = (free_fn)(g_moduleBase + 0x000BA74D);
                            if (g_free) g_free(p);
                            int next = *iterSlot;
                            if (g_AthenaListGetSize(listBuf) <= next) break;
                            p = *(void **)(*(int **)(listBuf + 0x40C) + next * 4);
                            *iterSlot = next + 1;
                        }
                    }
                }
                /* Vec3List_Free cleans up the AthenaList itself */
                {
                    typedef void (__thiscall *Vec3List_Free_t)(void *list);
                    Vec3List_Free_t freeFn = (Vec3List_Free_t)g_Vec3ListFree;
                    if (freeFn) freeFn(listBuf);
                }
            }
        }
    }

    /* ── Toob: N:SPINNY ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_SPINNY)) && my_strnicmp(name, "N:SPINNY", 8) == 0) {
        if (g_RotatorAddBall && meshObj) g_RotatorAddBall((void *)meshObj, (int)ball);
    }

    /* ── Toob: N:SAWTEETH ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_SAWTEETH)) && my_strnicmp(name, "N:SAWTEETH", 10) == 0) {
        if (meshObj && *(int *)(meshObj + 0x10F4) == 0 && *(int *)((char *)ball + 0x7DC) < 1) {
            *(int *)((char *)ball + 0x7DC) = 0x32;
            float vx = *(float *)(meshObj + 0x1100);
            float vy = *(float *)(meshObj + 0x1104);
            float vz = *(float *)(meshObj + 0x1108);
            float dir[3] = {vx, vy, vz};
            g_Vec3NormalizeAndScale(dir, 3.0f);
            int phys = ball[0x69];
            if (phys && !IsBadReadPtr((void *)phys, 0xCB0)) {
                *(float *)(phys + 0xCA4) = dir[0];
                *(float *)(phys + 0xCA8) = dir[1];
                *(float *)(phys + 0xCAC) = dir[2];
            }
            if (g_BallApplyTrajectory) g_BallApplyTrajectory((int)ball);
        }
    }

    /* ── Master: N:SPINNER ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_SPINNER)) && my_strnicmp(name, "N:SPINNER", 9) == 0) {
        if (g_SpinnerActivate && meshObj) g_SpinnerActivate((void *)meshObj, (int)ball);
    }

    /* ── Master: E:LAUNCH ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_LAUNCH)) && my_stricmp(name, "E:LAUNCH") == 0) {
        float launchPos[3] = {0,0,0};
        if (g_AthenaHashTableLookup) {
            void *ht = *(void **)((char *)board + BOARD_MESHWORLD);
            if (ht) g_AthenaHashTableLookup(ht, launchPos, "LAUNCHPOINT", NULL);
        }
        *(float *)((char *)ball + 0x164) = launchPos[0];
        *(float *)((char *)ball + 0x168) = launchPos[1];
        *(float *)((char *)ball + 0x16C) = launchPos[2];
        int phys = ball[0x69];
        if (phys && !IsBadReadPtr((void *)phys, 0xCB0)) {
            *(float *)(phys + 0xCA4) = 0.0f;
            *(DWORD *)(phys + 0xCA8) = 0x42200000;
            *(DWORD *)(phys + 0xCAC) = 0;
        }
        ball[200] = 0x19;
        ball[0xA7] = 0x3B03126F;
        ball[0xA8] = 5;
        *(int *)((char *)ball + 0x808) = 0x32;
        if (g_BallDizzyImmunity) g_BallDizzyImmunity(ball, 200);
        if (g_SoundPlay3D && app) {
            DWORD snd = *(DWORD *)(app + 0x500);
            if (snd) g_SoundPlay3D((void *)snd, launchPos[0], launchPos[1], launchPos[2]);
        }
        /* Create explosion particles */
        if (g_operatorNew && g_ArenaScoreParticleCtor && g_AthenaListAppend && g_WaveCos && g_WaveSin && g_CPUIDRNG) {
            float explodePos[3] = {0,0,0};
            if (g_AthenaHashTableLookup) {
                void *ht = *(void **)((char *)board + BOARD_MESHWORLD);
                if (ht) g_AthenaHashTableLookup(ht, explodePos, "EXPLODEHELPER", NULL);
            }
            int p;
            for (p = 0; p < 0x168; p += 5) {
                void *mem = g_operatorNew(0x28);
                if (!mem) break;
                DWORD *particle = (DWORD *)g_ArenaScoreParticleCtor(mem, app);
                if (!particle) continue;
                float angle = (float)p;
                float cosV = g_WaveCos((void *)0x4F7188, angle);
                float sinV = g_WaveSin((void *)0x4F7188, angle);
                *(float *)(particle + 2) = sinV * 0.375f + explodePos[0];
                *(float *)(particle + 3) = explodePos[1] + 0.4f;
                *(float *)(particle + 4) = cosV * 0.375f + explodePos[2];
                cosV = g_WaveCos((void *)0x4F7188, angle);
                sinV = g_WaveSin((void *)0x4F7188, angle);
                *(float *)(particle + 5) = sinV * 0.1f;
                *(DWORD *)(particle + 6) = 0;
                *(float *)(particle + 7) = cosV * 0.1f;
                int speedDiv = RNG_call((void *)0x4F7360, 0, 0x14, 0) + 0x14;
                float scale = 0.00390625f / (float)speedDiv;
                *(float *)(particle + 5) *= scale;
                *(float *)(particle + 6) *= scale;
                *(float *)(particle + 7) *= scale;
                g_AthenaListAppend((void *)((char *)board + UNI_PARTICLE_LIST), (int)particle);
            }
        }
    }

    /* ── Wobbly: N:SQUAREWOBBLY ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_SQUAREWOBBLY)) && my_strnicmp(name, "N:SQUAREWOBBLY", 14) == 0) {
        if ((char)*(int *)((char *)ball + 0x768) && g_SquareWobblyActivate && meshObj)
            g_SquareWobblyActivate((void *)meshObj, (int)ball);
    }

    /* ── Wobbly: N:WAVY ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_WAVY)) && my_strnicmp(name, "N:WAVY", 6) == 0) {
        if ((char)*(int *)((char *)ball + 0x768) && g_WavyActivate && meshObj)
            g_WavyActivate((void *)meshObj, (int)ball);
    }

    /* ── Glass: N:GLASS ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_GLASS)) && my_strnicmp(name, "N:GLASS", 7) == 0) {
        *(int *)((char *)ball + 0xC5C) = 0xF;
    }

    /* ── Glass: N:TENBONUS1 / N:TENBONUS2 ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_TENBONUS1)) && my_strnicmp(name, "N:TENBONUS1", 11) == 0) {
        int phys = ball[0x69];
        if (phys && !IsBadReadPtr((void *)phys, 0xCB0)) {
            float speed = sqrtf(*(float*)(phys+0xCA4)**(float*)(phys+0xCA4) + *(float*)(phys+0xCA8)**(float*)(phys+0xCA8) + *(float*)(phys+0xCAC)**(float*)(phys+0xCAC));
            if (speed >= 2.0f && *(char *)((char *)ext + REND_GLASS_FLAG1) == 0) {
                *(BYTE *)((char *)ext + REND_GLASS_FLAG1) = 1;
                if (g_SoundPlay3D && app) {
                    DWORD snd = *(DWORD *)(app + 0x52C);
                    if (snd) g_SoundPlay3D((void *)snd, *(float*)((char*)ext+REND_GLASS_S1_X), *(float*)((char*)ext+REND_GLASS_S1_Y), *(float*)((char*)ext+REND_GLASS_S1_Z));
                }
                if (app) {
                    int gameMode = *(int *)(app + 0x220);
                    if (gameMode && *(char *)(gameMode + 0x10) == 0 && *(char *)(gameMode + 0x11) == 0) {
                        *(DWORD *)(ball[6]*0xA0 + 0x5EC + app) = 1000;
                        if (g_operatorNew && g_ScoreObjectCtor && g_TimerDecrement && g_AthenaListAppend) {
                            void *mem = g_operatorNew(0x30);
                            if (mem) {
                                int so = g_ScoreObjectCtor(mem, (int)board, ball[6]*0xA0 + 0x5CC + app, "EXTRA TIME:");
                                g_TimerDecrement(so);
                                g_AthenaListAppend((void *)((char *)board + UNI_SCORE_LIST), so);
                            }
                        }
                    }
                }
            }
        }
    }
    if ((*(BYTE*)((char*)ext + COLL_FLAG_TENBONUS2)) && my_strnicmp(name, "N:TENBONUS2", 11) == 0) {
        int phys = ball[0x69];
        if (phys && !IsBadReadPtr((void *)phys, 0xCB0)) {
            float speed = sqrtf(*(float*)(phys+0xCA4)**(float*)(phys+0xCA4) + *(float*)(phys+0xCA8)**(float*)(phys+0xCA8) + *(float*)(phys+0xCAC)**(float*)(phys+0xCAC));
            if (speed >= 2.0f && *(char *)((char *)ext + REND_GLASS_FLAG2) == 0) {
                *(BYTE *)((char *)ext + REND_GLASS_FLAG2) = 1;
                if (g_SoundPlay3D && app) {
                    DWORD snd = *(DWORD *)(app + 0x52C);
                    if (snd) g_SoundPlay3D((void *)snd, *(float*)((char*)ext+REND_GLASS_S2_X), *(float*)((char*)ext+REND_GLASS_S2_Y), *(float*)((char*)ext+REND_GLASS_S2_Z));
                }
                if (app) {
                    int gameMode = *(int *)(app + 0x220);
                    if (gameMode && *(char *)(gameMode + 0x10) == 0 && *(char *)(gameMode + 0x11) == 0) {
                        *(DWORD *)(ball[6]*0xA0 + 0x5EC + app) = 1000;
                        if (g_operatorNew && g_ScoreObjectCtor && g_TimerDecrement && g_AthenaListAppend) {
                            void *mem = g_operatorNew(0x30);
                            if (mem) {
                                int so = g_ScoreObjectCtor(mem, (int)board, ball[6]*0xA0 + 0x5CC + app, "EXTRA TIME:");
                                g_TimerDecrement(so);
                                g_AthenaListAppend((void *)((char *)board + UNI_SCORE_LIST), so);
                            }
                        }
                    }
                }
            }
        }
    }

    /* ── Sky: E:PEGS ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_PEGS)) && my_stricmp(name, "E:PEGS") == 0) {
        if (*(int *)((char *)ball + 0x788) == 0) {
            *(int *)((char *)ext + UNI_PEG_COUNT) += 1;
            *(int *)((char *)ball + 0x788) = 1;
        }
    }

    /* ── Sky: E:TRAPPOP ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_TRAPPOP)) && my_stricmp(name, "E:TRAPPOP") == 0) {
        if (difficulty != 0 && g_RotatorStartSound) {
            int trapObj = *(int *)((char *)ext + UNI_NEON_TRAPDOOR);
            if (trapObj) g_RotatorStartSound(trapObj);
        }
    }

    /* ── Sky: E:NOPEGS ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_NOPEGS)) && my_stricmp(name, "E:NOPEGS") == 0) {
        if (*(int *)((char *)ball + 0x78C) == 0) {
            *(int *)((char *)ext + UNI_PEG_COUNT) -= 1;
            *(int *)((char *)ball + 0x78C) = 1;
        }
    }

    /* ── Sky: E:HEATON ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_HEATON)) && my_stricmp(name, "E:HEATON") == 0) {
        if (difficulty != 0 && g_AthenaListContainsValue && g_PendulumAddIndex) {
            DWORD mgObj = *(DWORD *)((char *)ext + UNI_MAGNIFYING_GLASS);
            if (mgObj) {
                if (!g_AthenaListContainsValue((void *)(mgObj + 0x2C), (int)ball))
                    g_PendulumAddIndex((void *)mgObj, (int)ball);
            }
        }
    }

    /* ── Sky: E:HEATOFF ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_HEATOFF)) && my_stricmp(name, "E:HEATOFF") == 0) {
        if (difficulty != 0 && g_AthenaListRemoveByValue) {
            DWORD mgObj = *(DWORD *)((char *)ext + UNI_MAGNIFYING_GLASS);
            if (mgObj) g_AthenaListRemoveByValue((void *)(mgObj + 0x2C), (int)ball);
        }
    }

    /* ── Impossible: N:BOUNCE ── */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_BOUNCE)) && my_strnicmp(name, "N:BOUNCE", 8) == 0) {
        if ((char)*(int *)((char *)ball + 0x768)) {
            int phys = ball[0x69];
            if (phys && !IsBadReadPtr((void *)phys, 0xCB0)) {
                float vx = *(float *)(phys + 0xCA4) * 2.0f;
                float vz = *(float *)(phys + 0xCAC) * 2.0f;
                float vy = 0.0f;
                float speed = sqrtf(vx*vx + vz*vz);
                if (speed < 1.25f) g_Vec3NormalizeAndScale((float[]){vx,vy,vz}, 1.25f);
                speed = sqrtf(vx*vx + vy*vy + vz*vz);
                if (speed > 3.0f) g_Vec3NormalizeAndScale((float[]){vx,vy,vz}, 3.0f);
                *(float *)(phys + 0xCA4) = vx;
                *(float *)(phys + 0xCA8) = vy;
                *(float *)(phys + 0xCAC) = vz;
            }
        }
    }

    /* ── Impossible: N:ONROTATOR ──
     * Original gates on (char)ball[0x768] != 0. */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_ONROTATOR)) && my_strnicmp(name, "N:ONROTATOR", 11) == 0) {
        if ((char)*(int *)((char *)ball + 0x768) && g_RotatorAddBall && meshObj)
            g_RotatorAddBall((void *)meshObj, (int)ball);
    }

    /* ── Impossible: N:ONGEAR ──
     * Original gates on (char)ball[0x768] != 0. */
    if ((*(BYTE*)((char*)ext + COLL_FLAG_ONGEAR)) && my_strnicmp(name, "N:ONGEAR", 8) == 0) {
        if ((char)*(int *)((char *)ball + 0x768) && g_GearAddBall && meshObj)
            g_GearAddBall((void *)meshObj, (int)ball);
    }

call_global:
    /* Call the original DispatchCollisionEvents (via trampoline) for global events */
    if (g_OriginalDispatch) g_OriginalDispatch(board, ball, collPair);
}
