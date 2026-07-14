/*
 * LevelSpecials_Loader v4 — Universal Level Handler
 *
 * 1. ALLOCATION PATCH: Patches all 15 level allocation sites in
 *    Tournament_AdvanceRace (0x00427080) to use the union size 0xA2F8.
 *
 * 2. BOARD CONSTRUCTOR HOOK: Patches all 15 CALL LevelBoard_*_ctor
 *    instructions to call a single UniversalBoardCtor driven by
 *    LevelData.txt config file.
 *
 * 3. SCENE LOADING HOOK: Hooks vtable[0x48] (CALL [EDX+0x48]) to
 *    replace all per-level Scene_LoadLevel* functions with a universal
 *    4-step scene loader (MeshWorldCtor, RenderCtor, InitScene, Board_Setup).
 *
 * 4. COLLISION HOOK: Hooks DispatchCollisionEvents (0x0040C5D0) to
 *    handle N:BUMPER collisions on any level, driven by LevelSpecials.txt.
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef unsigned long long QWORD;

/* ═══════════════════════════════════════════════════════════════════════════
 * Debug logging (forward declaration — defined later, used by hook functions)
 * ═══════════════════════════════════════════════════════════════════════════ */
void DebugLog(const char *msg);

/* ═══════════════════════════════════════════════════════════════════════════
 * Game addresses (RVAs)
 * ═══════════════════════════════════════════════════════════════════════════ */

#define RVA_DispatchCollisionEvents  0x000CC5D0
#define RVA_Sound_Play3D             0x00059860
#define RVA_Scene_CollectByNameFilter 0x000602F0
#define RVA_AthenaList_Init          0x00053210
#define RVA_operator_new              0x000BA57B
#define RVA_Level_MeshWorldCtor       0x00061510
#define RVA_Level_RenderCtor          0x00065080
#define RVA_Level_InitScene           0x0000B090
#define RVA_Board_ctor                0x00019030
#define RVA_LoadRaceData              0x0000A120
#define RVA_Vec3_Init                 0x00053180
#define RVA_Matrix_Identity           0x00053200
#define RVA_MeshNode_ctor              0x00071C20
#define RVA_Sprite_ctor                0x0005D0C0
#define RVA_TipperVisual_Attach       0x00065200
#define RVA_Level_AssignTexAndScales  0x00011BA0

/* Bumper physics constants */
#define BUMPER_VEL_SCALE  4.0f
#define BUMPER_MIN_SPEED  5.0f
#define BUMPER_MAX_SPEED  10.0f

/* Struct offsets */
#define BALL_POS_X        0x164
#define BALL_POS_Y        0x168
#define BALL_POS_Z        0x16C
#define BALL_PHYS_PTR     0x1A4
#define PHYS_VEL_X        0xCA4
#define PHYS_VEL_Y        0xCA8
#define PHYS_VEL_Z        0xCAC
#define APP_SOUNDFX_LIST  0x448
#define COLL_MESHBUF      0x4
#define MESHBUF_NAME      0x864
#define BOARD_APP_PTR     0x878
#define BOARD_MESHWORLD   0x8AC
#define BOARD_RENDEROBJ   0x8B0

/* Bumper slot layout */
#define BUMPER_SLOT_BASE   0x436C
#define BUMPER_SLOT_STRIDE 0x418
#define BUMPER_LIT_BASE    0x642C
#define BUMPER_LIT_STRIDE  4
#define BUMPER_LIT_COLL     0x6428

/* Bridge slot layout (board+0x436C..0x4388) */
#define BRIDGE_MESHWORLD   0x436C
#define BRIDGE_RENDEROBJ   0x4370
#define BRIDGE_PARAM1      0x4380  /* float 45.0 in Intermediate */
#define BRIDGE_PARAM2      0x4384  /* 0 */
#define BRIDGE_PARAM3      0x4388  /* 0x32 = 50 */

#define UNION_SIZE  0xA2F8

/* ═══════════════════════════════════════════════════════════════════════════
 * Object type system — extensible per-level feature toggles
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef enum {
    OBJ_BUMPERS = 0,
    OBJ_BRIDGE,
    OBJ_COUNT
} ObjectType;

static const char *g_objectNames[OBJ_COUNT] = {
    "BUMPERS",
    "BRIDGE",
};

/* ═══════════════════════════════════════════════════════════════════════════
 * Level vtable addresses (absolute — module base 0x00400000)
 * ═══════════════════════════════════════════════════════════════════════════ */

static DWORD g_levelVtables[16] = {
    0,              /* index 0 unused */
    0x004D04A8,     /* 1=WarmUp */
    0x004D1098,     /* 2=Beginner */
    0x004D05A0,     /* 3=Intermediate */
    0x004D0890,     /* 4=Dizzy */
    0x004D0A08,     /* 5=Tower */
    0x004D11A0,     /* 6=Up */
    0x004D1DF0,     /* 7=Neon */
    0x004D0B00,     /* 8=Expert */
    0x004D0BC0,     /* 9=Odd */
    0x004D0E78,     /* 10=Toob */
    0x004D0D38,     /* 11=Wobbly */
    0x004D1F90,     /* 12=Glass */
    0x004D0FC8,     /* 13=Sky */
    0x004D12B0,     /* 14=Master */
    0x004D21C0,     /* 15=Impossible */
};

/* ═══════════════════════════════════════════════════════════════════════════
 * Per-level data defaults (extracted from Ghidra decompilation)
 * ═══════════════════════════════════════════════════════════════════════════ */

#define MAX_MESHES_PER_LEVEL 16
#define MAX_STR_LEN 128

typedef struct {
    char name[MAX_STR_LEN];
    DWORD vtable;
    char boardName[MAX_STR_LEN];
    char raceTitle[MAX_STR_LEN];
    char raceData[MAX_STR_LEN];
    char musicName[MAX_STR_LEN];
    float color[3];
    char meshPath[MAX_STR_LEN];
    /* Extra meshes: offset:path pairs. Path prefixes:
       bare = Level_MeshWorldCtor, RENDER = Level_RenderCtor(prev),
       MESH: = MeshNode_ctor, SPRITE: = Sprite_ctor */
    char meshes[MAX_MESHES_PER_LEVEL][MAX_STR_LEN];
    int meshCount;
    DWORD unlockFlagOffset;  /* 0 = none */
} LevelData;

static LevelData g_levelData[16] = {
    {{0}}, /* index 0 unused */
    /* 1=WarmUp */
    {"WarmUp",0x004D04A8,"Board (Warm-Up)","WARM-UP RACE","BEGINNERRACE","Hamster Nation",{1.0f,0.0f,1.0f},"levels\\level1",{},0,0},
    /* 2=Beginner */
    {"Beginner",0x004D1098,"Board (Beginner)","BEGINNER RACE","CASCADERACE","Cascade Race",{1.0f,0.75f,0.25f},"levels\\levelcascade",{},0,0},
    /* 3=Intermediate */
    {"Intermediate",0x004D05A0,"Board (Intermediate)","INTERMEDIATE RACE","INTERMEDIATERACE","Gerbil Groove",{0.0f,0.0f,1.0f},"levels\\level2",{},0,0},
    /* 4=Dizzy */
    {"Dizzy",0x004D0890,"Board (Dizzy)","DIZZY RACE","DIZZYRACE","Dizzy!",{0.0f,1.0f,0.0f},"levels\\level3",{"0x436C:Levels\\Level3-Tipper","0x4370:RENDER","0x4BA8:Levels\\Level3-WaterWheel","0x4BAC:RENDER","0x4BC4:Levels\\Level3-Swirl","0x4BC8:RENDER","0x4374:Levels\\Level3-Gluebie"},7,0x851},
    /* 5=Tower */
    {"Tower",0x004D0A08,"Board (Tower)","TOWER RACE","TOWERRACE","Happy Rush",{1.0f,0.75f,0.0f},"levels\\level4",{"0x436C:Levels\\Level4-Catapult","0x4370:Levels\\Level4-Drawbridge","0x4374:MESH:Meshes\\YellowLink","0x4378:Levels\\Level4-Mace","0x437C:Levels\\Level4-Windmill","0x4390:MESH:Meshes\\Chomper","0x43B4:Levels\\Level4-Turret"},7,0},
    /* 6=Up */
    {"Up",0x004D11A0,"Board (Up)","UP RACE","UPRACE","Up Race",{1.0f,0.0f,1.0f},"levels\\levelup",{"0x4784:levels\\levelup-lifter","0x4788:levels\\levelup-speedcylinder","0x478C:levels\\levelup-button"},3,0x853},
    /* 7=Neon */
    {"Neon",0x004D1DF0,"Board (Dark)","NEON RACE","NEONRACE","Neon Theme",{1.0f,1.0f,0.0f},"levels\\leveldark",{"0x4374:Levels\\LevelDark-NeonPlatform","0x4378:Levels\\LevelDark-DFloor1","0x437C:Levels\\LevelDark-DFloor2","0x4380:Levels\\LevelDark-DFloor3","0x4384:Levels\\LevelDark-DFloor4","0x4388:Levels\\LevelDark-Trode"},6,0},
    /* 8=Expert */
    {"Expert",0x004D0B00,"Board (Expert)","EXPERT RACE","EXPERTRACE","Fight!",{1.0f,0.0f,0.0f},"levels\\level5",{"0x4378:Levels\\Level5-Bridge","0x437C:RENDER","0x4BB0:MESH:meshes\\hammyjudge","0x4BB4:MESH:meshes\\hammyjudge","0x4BB8:MESH:meshes\\hammyjudge"},5,0x854},
    /* 9=Odd */
    {"Odd",0x004D0BC0,"Board (Odd)","ODD RACE","ODDRACE","Ninja Hamster",{1.0f,0.5f,0.0f},"levels\\level6",{},0,0x855},
    /* 10=Toob */
    {"Toob",0x004D0E78,"Board (Toob)","TOOB RACE","TOOBRACE","Rodenthood",{0.5f,0.5f,1.0f},"levels\\level8",{"0x436C:Levels\\Level8-Spinny","0x4370:Levels\\Level8-Saw","0x4374:Levels\\Level8-Fallout","0x4378:Levels\\Level8-Blockdawg1","0x437C:Levels\\Level8-Blockdawg2"},5,0x856},
    /* 11=Wobbly */
    {"Wobbly",0x004D0D38,"Board (Wobbly)","WOBBLY RACE","WOBBLYRACE","Hamster Chase",{0.62f,0.84f,0.30f},"levels\\level7",{"0x436C:Levels\\Level7-Wobbly1","0x4370:Levels\\Level7-Wobbly2","0x4374:Levels\\Level7-Wobbly3","0x4378:Levels\\Level7-Wobbly4","0x437C:Levels\\Level7-Wobbly5","0x4380:Levels\\Level7-Wobbly6","0x4384:Levels\\Level7-Wobbly7"},7,0x857},
    /* 12=Glass */
    {"Glass",0x004D1F90,"Board (Glass)","GLASS RACE","GLASSRACE","Glass Theme",{1.0f,0.0f,1.0f},"levels\\levelglass",{},0,0},
    /* 13=Sky */
    {"Sky",0x004D0FC8,"Board (Sky)","SKY RACE","SKYRACE","Bucky Break",{0.0f,0.5f,1.0f},"levels\\level9",{"0x436C:MESH:meshes\\skypillar","0x4370:MESH:meshes\\magnifyingglass","0x4384:levels\\level9-popcylinder1","0x4388:levels\\level9-popcylinder2","0x438C:levels\\level9-trapdoor","0x4374:SPRITE:textures\\clouds.png"},6,0x858},
    /* 14=Master */
    {"Master",0x004D12B0,"Board (Master)","MASTER RACE","MASTERRACE","Master Theme",{0.5f,0.5f,0.5f},"levels\\level10",{"0x4374:Levels\\Level10-2PBridge","0x4378:RENDER","0x4394:Levels\\Level3-Tipper","0x4398:RENDER","0x5410:Levels\\Level10-Bridge1","0x5414:Levels\\Level10-Bridge2","0x5420:levels\\level9-popcylinder1","0x5424:levels\\level9-popcylinder2","0x5840:Levels\\Level8-Blockdawg1","0x5844:Levels\\Level8-Blockdawg2","0x5848:Levels\\Level4-Catapult","0x607C:Levels\\Level3-Gluebie"},12,0x859},
    /* 15=Impossible */
    {"Impossible",0x004D21C0,"Board (Impossible)","IMPOSSIBLE RACE","IMPOSSIBLERACE","Impossible Theme",{1.0f,0.0f,0.0f},"levels\\levelimpossible",{"0x436C:Levels\\LevelImpossible-Looper","0x4370:Levels\\LevelImpossible-Gear","0x4374:Levels\\LevelImpossible-BigGear","0x4378:Levels\\LevelImpossible-Rotator","0x437C:Levels\\LevelImpossible-Pendulum"},5,0},
};

/* ═══════════════════════════════════════════════════════════════════════════
 * Config: per-level feature flags
 * g_objectEnabled[objType][level] = 1 if enabled
 * ═══════════════════════════════════════════════════════════════════════════ */

static int g_objectEnabled[OBJ_COUNT][16] = {{0}};

/* ═══════════════════════════════════════════════════════════════════════════
 * Globals
 * ═══════════════════════════════════════════════════════════════════════════ */

static DWORD g_moduleBase = 0;
typedef void (__thiscall *Sound_Play3D_t)(void *soundChannel, float x, float y, float z);
static Sound_Play3D_t g_SoundPlay3D = NULL;

/* Must be non-static for asm reference */
typedef void (__thiscall *DispatchCollisionEvents_t)(void *board, void *ball, void *collPair);
DispatchCollisionEvents_t g_OriginalDispatch = NULL;

static unsigned char *g_trampoline = NULL;
static char g_configPath[MAX_PATH] = "";
static char g_levelDataPath[MAX_PATH] = "";

/* Pending race index for board constructor thunks */
/* Must be non-static for asm reference */
int g_pendingRaceIndex = 0;

/* ═══════════════════════════════════════════════════════════════════════════
 * String helpers
 * ═══════════════════════════════════════════════════════════════════════════ */

static int my_strnicmp(const char *a, const char *b, int n) {
    int i;
    for (i = 0; i < n; i++) {
        char ca = a[i], cb = b[i];
        if (ca >= 'a' && ca <= 'z') ca -= 32;
        if (cb >= 'a' && cb <= 'z') cb -= 32;
        if (ca != cb) return (unsigned char)ca - (unsigned char)cb;
        if (ca == 0) return 0;
    }
    return 0;
}

static void my_strncpy(char *dst, const char *src, int max) {
    int i;
    for (i = 0; i < max - 1 && src[i]; i++) dst[i] = src[i];
    dst[i] = '\0';
}

static void trim_str(char *s) {
    int len;
    while (*s == ' ' || *s == '\t' || *s == '\r') {
        char *p = s;
        while (*p) { *p = p[1]; p++; }
    }
    len = strlen(s);
    while (len > 0 && (s[len-1] == ' ' || s[len-1] == '\t' || s[len-1] == '\r')) {
        s[--len] = '\0';
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Level identification
 * ═══════════════════════════════════════════════════════════════════════════ */

static int GetCurrentLevel(void *board) {
    DWORD vtable = *(DWORD *)board;
    int i;
    for (i = 1; i <= 15; i++) {
        if (vtable == g_levelVtables[i])
            return i;
    }
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Config parser (LevelSpecials.txt)
 * Format:
 *   [OBJECTS]
 *   BUMPERS = 2 5 8
 *   BRIDGE = 3 14
 *
 * Object name = level numbers (1-15). Empty () = disabled.
 * ═══════════════════════════════════════════════════════════════════════════ */

static void LoadConfig(void) {
    memset(g_objectEnabled, 0, sizeof(g_objectEnabled));
    HANDLE hFile = CreateFileA(g_configPath, GENERIC_READ, FILE_SHARE_READ,
                               NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return;
    DWORD fileSize = GetFileSize(hFile, NULL);
    if (fileSize > 8192) fileSize = 8192;
    char buf[8192];
    DWORD bytesRead = 0;
    ReadFile(hFile, buf, fileSize, &bytesRead, NULL);
    CloseHandle(hFile);
    buf[bytesRead] = '\0';

    char *line = buf;
    int inObjectsSection = 0;
    while (line < buf + bytesRead) {
        char *eol = line;
        while (*eol && *eol != '\n' && *eol != '\r') eol++;
        char saved = *eol;
        *eol = '\0';
        char *p = line;
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '\0' || *p == '#' || *p == ';') goto next_line;
        if (p[0] == '[') {
            inObjectsSection = (my_strnicmp(p, "[OBJECTS", 8) == 0);
            goto next_line;
        }
        if (inObjectsSection) {
            /* Find object name match */
            int objType = -1;
            int i;
            for (i = 0; i < OBJ_COUNT; i++) {
                int nameLen = strlen(g_objectNames[i]);
                if (my_strnicmp(p, g_objectNames[i], nameLen) == 0) {
                    /* Make sure next char is '=' or whitespace (not a prefix) */
                    char after = p[nameLen];
                    if (after == '=' || after == ' ' || after == '\t') {
                        objType = i;
                        break;
                    }
                }
            }
            if (objType < 0) goto next_line;

            char *eq = p;
            while (*eq && *eq != '=') eq++;
            if (*eq == '=') {
                eq++;
                while (*eq) {
                    while (*eq == ' ' || *eq == '\t') eq++;
                    if (!*eq) break;
                    if (*eq == '(') {
                        eq++;
                        while (*eq && *eq != ')') eq++;
                        if (*eq == ')') eq++;
                        continue;
                    }
                    if (*eq >= '0' && *eq <= '9') {
                        int levelNum = atoi(eq);
                        if (levelNum >= 1 && levelNum <= 15)
                            g_objectEnabled[objType][levelNum] = 1;
                        while (*eq && *eq >= '0' && *eq <= '9') eq++;
                    } else {
                        eq++;
                    }
                }
            }
        }
    next_line:
        *eol = saved;
        if (*eol == '\r') eol++;
        if (*eol == '\n') eol++;
        line = eol;
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * LevelData.txt parser
 * ═══════════════════════════════════════════════════════════════════════════ */

static void LoadLevelData(void) {
    HANDLE hFile = CreateFileA(g_levelDataPath, GENERIC_READ, FILE_SHARE_READ,
                               NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return;
    DWORD fileSize = GetFileSize(hFile, NULL);
    if (fileSize > 32768) fileSize = 32768;
    char buf[32768];
    DWORD bytesRead = 0;
    ReadFile(hFile, buf, fileSize, &bytesRead, NULL);
    CloseHandle(hFile);
    buf[bytesRead] = '\0';

    /* Skip BOM */
    char *start = buf;
    if (bytesRead >= 3 && (unsigned char)start[0] == 0xEF &&
        (unsigned char)start[1] == 0xBB && (unsigned char)start[2] == 0xBF)
        start += 3;

    int currentLevel = 0;
    char *line = start;
    while (line < buf + bytesRead) {
        char *eol = line;
        while (*eol && *eol != '\n' && *eol != '\r') eol++;
        char saved = *eol;
        *eol = '\0';
        trim_str(line);
        char *p = line;

        if (*p == '\0' || *p == '#' || *p == ';') goto next_ld_line;

        if (p[0] == '[') {
            /* Parse [N] section header */
            char *cl = p + 1;
            while (*cl && *cl != ']') cl++;
            if (*cl == ']') {
                *cl = '\0';
                currentLevel = atoi(p + 1);
                if (currentLevel < 1 || currentLevel > 15) currentLevel = 0;
            }
            goto next_ld_line;
        }

        if (currentLevel >= 1 && currentLevel <= 15) {
            LevelData *ld = &g_levelData[currentLevel];
            char *eq = p;
            while (*eq && *eq != '=') eq++;
            if (*eq != '=') goto next_ld_line;
            *eq = '\0';
            char *key = p;
            char *val = eq + 1;
            trim_str(key);
            trim_str(val);

            if (my_strnicmp(key, "Name", 4) == 0) {
                my_strncpy(ld->name, val, MAX_STR_LEN);
            } else if (my_strnicmp(key, "Vtable", 6) == 0) {
                ld->vtable = (DWORD)strtoul(val, NULL, 0);
                g_levelVtables[currentLevel] = ld->vtable;
            } else if (my_strnicmp(key, "BoardName", 9) == 0) {
                my_strncpy(ld->boardName, val, MAX_STR_LEN);
            } else if (my_strnicmp(key, "RaceTitle", 9) == 0) {
                my_strncpy(ld->raceTitle, val, MAX_STR_LEN);
            } else if (my_strnicmp(key, "RaceData", 8) == 0) {
                my_strncpy(ld->raceData, val, MAX_STR_LEN);
            } else if (my_strnicmp(key, "Music", 5) == 0) {
                my_strncpy(ld->musicName, val, MAX_STR_LEN);
            } else if (my_strnicmp(key, "Color", 5) == 0) {
                float r = 0, g = 0, b = 0;
                if (sscanf(val, "%f %f %f", &r, &g, &b) == 3) {
                    ld->color[0] = r; ld->color[1] = g; ld->color[2] = b;
                }
            } else if (my_strnicmp(key, "MeshPath", 8) == 0) {
                my_strncpy(ld->meshPath, val, MAX_STR_LEN);
            } else if (my_strnicmp(key, "Meshes", 6) == 0) {
                ld->meshCount = 0;
                char *tok = val;
                while (*tok && ld->meshCount < MAX_MESHES_PER_LEVEL) {
                    char *semi = tok;
                    while (*semi && *semi != ';') semi++;
                    char sv = *semi;
                    *semi = '\0';
                    trim_str(tok);
                    if (*tok) {
                        my_strncpy(ld->meshes[ld->meshCount], tok, MAX_STR_LEN);
                        ld->meshCount++;
                    }
                    *semi = sv;
                    if (!sv) break;
                    tok = semi + 1;
                }
            }
        }
    next_ld_line:
        *eol = saved;
        if (*eol == '\r') eol++;
        if (*eol == '\n') eol++;
        line = eol;
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Auto-generate LevelData.txt if missing
 * ═══════════════════════════════════════════════════════════════════════════ */

static void GenerateLevelData(void) {
    HANDLE hFile = CreateFileA(g_levelDataPath, GENERIC_WRITE, 0,
                               NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return;

    const char *header =
        "# LevelData.txt — Per-level configuration for LevelSpecials_Loader\n"
        "# Level numbers: 1=WarmUp 2=Beginner 3=Intermediate 4=Dizzy 5=Tower\n"
        "#   6=Up 7=Neon 8=Expert 9=Odd 10=Toob 11=Wobbly 12=Glass 13=Sky\n"
        "#   14=Master 15=Impossible\n"
        "# Meshes format: 0xOFFSET:PATH;0xOFFSET:PATH;...\n"
        "# Path types: bare=Level_MeshWorldCtor, RENDER=Level_RenderCtor(prev),\n"
        "#   MESH:path=MeshNode_ctor, SPRITE:path=Sprite_ctor\n\n";
    DWORD written;
    WriteFile(hFile, header, strlen(header), &written, NULL);

    int i;
    for (i = 1; i <= 15; i++) {
        LevelData *ld = &g_levelData[i];
        char line[4096];
        int pos = 0;

        pos += sprintf(line + pos, "[%d]\n", i);
        pos += sprintf(line + pos, "Name=%s\n", ld->name);
        pos += sprintf(line + pos, "Vtable=0x%08lX\n", (unsigned long)ld->vtable);
        pos += sprintf(line + pos, "BoardName=%s\n", ld->boardName);
        pos += sprintf(line + pos, "RaceTitle=%s\n", ld->raceTitle);
        pos += sprintf(line + pos, "RaceData=%s\n", ld->raceData);
        pos += sprintf(line + pos, "Music=%s\n", ld->musicName);
        pos += sprintf(line + pos, "Color=%.3f %.3f %.3f\n", ld->color[0], ld->color[1], ld->color[2]);
        pos += sprintf(line + pos, "MeshPath=%s\n", ld->meshPath);

        pos += sprintf(line + pos, "Meshes=");
        int j;
        for (j = 0; j < ld->meshCount; j++) {
            pos += sprintf(line + pos, "%s%s", j > 0 ? ";" : "", ld->meshes[j]);
        }
        pos += sprintf(line + pos, "\n\n");

        WriteFile(hFile, line, pos, &written, NULL);
    }

    CloseHandle(hFile);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Get config paths (next to this DLL)
 * ═══════════════════════════════════════════════════════════════════════════ */

static void GetConfigPath(void) {
    HMODULE hSelf = NULL;
    GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                       | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                       (LPCSTR)&GetConfigPath, &hSelf);
    if (hSelf && GetModuleFileNameA(hSelf, g_configPath, MAX_PATH)) {
        char *p = strrchr(g_configPath, '\\');
        if (p) {
            strcpy(p + 1, "LevelSpecials.txt");
            strcpy(g_levelDataPath, g_configPath);
            p = strrchr(g_levelDataPath, '\\');
            if (p) strcpy(p + 1, "LevelData.txt");
        }
    }
}

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

    if (IsBadReadPtr(ball, 0x1A8)) return;
    float posX = *(float *)((char *)ball + BALL_POS_X);
    float posY = *(float *)((char *)ball + BALL_POS_Y);
    float posZ = *(float *)((char *)ball + BALL_POS_Z);

    DWORD *app = *(DWORD **)((char *)board + BOARD_APP_PTR);
    if (app && !IsBadReadPtr(app, 0x500)) {
        DWORD soundList = app[APP_SOUNDFX_LIST / 4];
        if (soundList && g_SoundPlay3D) {
            g_SoundPlay3D((void *)soundList, posX, posY, posZ);
        }
    }

    DWORD phys = *(DWORD *)((char *)ball + BALL_PHYS_PTR);
    if (!phys || IsBadReadPtr((void *)phys, 0xCB0)) return;

    float *velX = (float *)(phys + PHYS_VEL_X);
    float *velY = (float *)(phys + PHYS_VEL_Y);
    float *velZ = (float *)(phys + PHYS_VEL_Z);

    float vx = *velX * BUMPER_VEL_SCALE;
    float vz = *velZ * BUMPER_VEL_SCALE;
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
    if (speedSq > BUMPER_MAX_SPEED * BUMPER_MAX_SPEED) {
        float speed = sqrtf(speedSq);
        if (speed > 0.0001f) {
            float scale = BUMPER_MAX_SPEED / speed;
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
    if (!g_objectEnabled[OBJ_BUMPERS][level]) return;
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
typedef int (__cdecl *Vec3_Init_t)(void *out, float x, float y, float z);
typedef void (__cdecl *Matrix_Identity_t)(void *out);
typedef void *(__thiscall *MeshNode_ctor_t)(void *mem, void *gfx, const char *path);
typedef void *(__thiscall *Sprite_ctor_t)(void *mem, void *gfx, const char *path);
typedef void (__thiscall *TipperVisual_Attach_t)(void *renderObj, void *meshWorld);
typedef void (__thiscall *Level_AssignTex_t)(void *board, void *meshWorld);

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

/* ═══════════════════════════════════════════════════════════════════════════
 * Universal Board Constructor — REPLACES all 15 per-level board constructors
 *
 * Called via 15 naked thunks that set g_pendingRaceIndex.
 * Does the same steps every LevelBoard_*_ctor does:
 *   1. Board_ctor(mem, app)
 *   2. Set vtable
 *   3. Set board name (+0x868), race title (+0x29B4)
 *   4. Set +0x870 from app+0x1DC
 *   5. Vec3_Init with per-level color, store at +0x1508
 *   6. LoadRaceData(mem, raceName)
 *   7. Set music name (+0x4344)
 *   8. Load extra meshes (per-level)
 *   9. Set unlock flags (per-level)
 * ═══════════════════════════════════════════════════════════════════════════ */

static void LoadExtraMeshes(void *board, LevelData *ld) {
    DWORD app = *(DWORD *)((char *)board + BOARD_APP_PTR);
    if (!app || IsBadReadPtr((void *)app, 0x200)) return;
    void *gfx = *(void **)((char *)app + 0x174);
    if (!gfx) return;

    void *prevMesh = NULL;
    int i;
    for (i = 0; i < ld->meshCount; i++) {
        char *entry = ld->meshes[i];
        if (!*entry) continue;

        /* Parse: 0xOFFSET:PATH */
        DWORD offset = (DWORD)strtoul(entry, NULL, 0);
        char *colon = strchr(entry, ':');
        if (!colon) continue;
        char *path = colon + 1;
        if (!*path) continue;

        void *result = NULL;

        if (my_strnicmp(path, "RENDER", 6) == 0) {
            /* Level_RenderCtor on previous mesh */
            void *mem = g_operatorNew(0x10D0);
            if (mem) result = g_LevelRenderCtor(mem, prevMesh);
            /* TipperVisual_Attach for bridge levels */
            if (result && prevMesh)
                g_TipperVisualAttach(result, prevMesh);
        } else if (my_strnicmp(path, "MESH:", 5) == 0) {
            /* MeshNode_ctor */
            void *mem = g_operatorNew(0x18);
            if (mem) result = g_MeshNodeCtor(mem, gfx, path + 5);
        } else if (my_strnicmp(path, "SPRITE:", 7) == 0) {
            /* Sprite_ctor */
            void *mem = g_operatorNew(0xD4);
            if (mem) result = g_SpriteCtor(mem, gfx, path + 7);
        } else {
            /* Level_MeshWorldCtor */
            void *mem = g_operatorNew(0x10D0);
            if (mem) result = g_LevelMeshWorldCtor(mem, gfx, path);
            prevMesh = result;
        }

        if (offset > 0 && offset < UNION_SIZE) {
            *(void **)((char *)board + offset) = result;
        }
    }
}

void __cdecl UniversalBoardCtorLogic(void *mem, int app) {
    int raceIndex = g_pendingRaceIndex;
    char buf[256];
    wsprintfA(buf, "UniversalBoardCtorLogic called: mem=%p app=0x%X raceIndex=%d", mem, app, raceIndex);
    DebugLog(buf);
    if (!mem || raceIndex < 1 || raceIndex > 15) {
        DebugLog("UniversalBoardCtorLogic: invalid params, returning");
        return;
    }
    if (!g_BoardCtor || !g_LoadRaceData || !g_Vec3Init || !g_MatrixIdentity) {
        DebugLog("UniversalBoardCtorLogic: function pointers not resolved");
        return;
    }

    LevelData *ld = &g_levelData[raceIndex];

    /* Step 1: Board_ctor(mem, app) — base setup */
    g_BoardCtor(mem, app);

    /* Step 2: Set vtable */
    *(DWORD *)mem = g_levelVtables[raceIndex];

    /* Step 3: Set board name and race title */
    *(char **)((char *)mem + 0x868) = ld->boardName;
    *(char **)((char *)mem + 0x29B4) = ld->raceTitle;

    /* Step 4: Set +0x870 from app+0x1DC */
    *(DWORD *)((char *)mem + 0x870) = *(DWORD *)(app + 0x1DC);

    /* Step 5: Vec3_Init with per-level color, store at +0x1508 */
    float vec3buf[5];
    g_Vec3Init(vec3buf, ld->color[0], ld->color[1], ld->color[2]);
    memcpy((char *)mem + 0x1508, (char *)vec3buf + 4, 16);
    g_MatrixIdentity(vec3buf);

    /* Step 6: LoadRaceData(mem, raceName) */
    g_LoadRaceData(mem, ld->raceData);

    /* Step 7: Set music name */
    *(char **)((char *)mem + 0x4344) = ld->musicName;

    /* Step 8: Load extra meshes */
    if (g_operatorNew && g_LevelMeshWorldCtor) {
        LoadExtraMeshes(mem, ld);
    }

    /* Step 9: Set unlock flags */
    if (ld->unlockFlagOffset) {
        DWORD appVal = *(DWORD *)((char *)mem + BOARD_APP_PTR);
        if (appVal && !IsBadReadPtr((void *)appVal, 0x860)) {
            DWORD diff = *(DWORD *)(appVal + 0x23C);
            if (diff != 0) {
                DWORD gameMode = *(DWORD *)(appVal + 0x220);
                if (gameMode && !IsBadReadPtr((void *)gameMode, 0x20)) {
                    if (*(char *)(gameMode + 0x10) == 0) {
                        *(char *)(appVal + ld->unlockFlagOffset) = 1;
                    }
                }
            }
        }
    }

    /* Special: Impossible sets board+0x4348=1 */
    if (raceIndex == 15) {
        *(char *)((char *)mem + 0x4348) = 1;
    }

    /* Special: Master sets board+0x29C0=0x449C4000 */
    if (raceIndex == 14) {
        *(DWORD *)((char *)mem + 0x29C0) = 0x449C4000;
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Universal Board Constructor Entry — naked, handles __thiscall convention
 * ECX=mem, [ESP+4]=app, returns board ptr in EAX
 * ═══════════════════════════════════════════════════════════════════════════ */

__attribute__((naked)) void UniversalBoardCtorEntry(void) {
    __asm__ __volatile__(
        "pushl %%ebp\n\t"
        "movl  %%esp, %%ebp\n\t"
        "pushl %%ecx\n\t"          /* save mem */
        "pushl 8(%%ebp)\n\t"       /* app */
        "pushl %%ecx\n\t"          /* mem (from saved) */
        "call  _UniversalBoardCtorLogic\n\t"
        "addl  $8, %%esp\n\t"
        "popl  %%ecx\n\t"          /* restore mem to ECX */
        "movl  %%ecx, %%eax\n\t"   /* return mem in EAX */
        "popl  %%ebp\n\t"
        "ret   $4\n\t"             /* __thiscall: callee cleans 4 bytes */
        :: : "eax", "ecx", "edx", "memory"
    );
}

/* ═══════════════════════════════════════════════════════════════════════════
 * 15 naked thunks — each sets race index then JMPs to UniversalBoardCtorEntry
 * ═══════════════════════════════════════════════════════════════════════════ */

#define DEFINE_LEVEL_THUNK(NUM) \
__attribute__((naked)) static void Thunk_Level##NUM(void) { \
    __asm__ __volatile__( \
        "movl $" #NUM ", _g_pendingRaceIndex\n\t" \
        "jmp _UniversalBoardCtorEntry\n\t" \
    ); \
}

DEFINE_LEVEL_THUNK(1)
DEFINE_LEVEL_THUNK(2)
DEFINE_LEVEL_THUNK(3)
DEFINE_LEVEL_THUNK(4)
DEFINE_LEVEL_THUNK(5)
DEFINE_LEVEL_THUNK(6)
DEFINE_LEVEL_THUNK(7)
DEFINE_LEVEL_THUNK(8)
DEFINE_LEVEL_THUNK(9)
DEFINE_LEVEL_THUNK(10)
DEFINE_LEVEL_THUNK(11)
DEFINE_LEVEL_THUNK(12)
DEFINE_LEVEL_THUNK(13)
DEFINE_LEVEL_THUNK(14)
DEFINE_LEVEL_THUNK(15)

/* ═══════════════════════════════════════════════════════════════════════════
 * Patch all 15 CALL LevelBoard_*_ctor instructions
 * ═══════════════════════════════════════════════════════════════════════════ */

static const DWORD g_ctorCallRVAs[15] = {
    0x0002712C,  /* 1=WarmUp */
    0x0002715D,  /* 2=Beginner */
    0x0002718E,  /* 3=Intermediate */
    0x000271BF,  /* 4=Dizzy */
    0x000271F0,  /* 5=Tower */
    0x00027221,  /* 6=Up */
    0x00027252,  /* 7=Neon */
    0x00027283,  /* 8=Expert */
    0x000272B4,  /* 9=Odd */
    0x000272E5,  /* 10=Toob */
    0x00027316,  /* 11=Wobbly */
    0x00027347,  /* 12=Glass */
    0x00027374,  /* 13=Sky */
    0x0002739E,  /* 14=Master */
    0x000273C8,  /* 15=Impossible */
};

static void *g_thunkPtrs[16] = {
    NULL,
    Thunk_Level1, Thunk_Level2, Thunk_Level3, Thunk_Level4,
    Thunk_Level5, Thunk_Level6, Thunk_Level7, Thunk_Level8,
    Thunk_Level9, Thunk_Level10, Thunk_Level11, Thunk_Level12,
    Thunk_Level13, Thunk_Level14, Thunk_Level15,
};

static void InstallBoardCtorHooks(void) {
    int i;
    for (i = 0; i < 15; i++) {
        unsigned char *site = (unsigned char *)(g_moduleBase + g_ctorCallRVAs[i]);
        if (IsBadReadPtr(site, 5)) continue;
        if (site[0] != 0xE8) continue;  /* verify CALL opcode */

        DWORD target = (DWORD)g_thunkPtrs[i + 1];
        DWORD rel32 = target - ((DWORD)site + 5);

        DWORD oldProtect;
        VirtualProtect(site, 5, PAGE_EXECUTE_READWRITE, &oldProtect);
        *(DWORD *)(site + 1) = rel32;
        VirtualProtect(site, 5, oldProtect, &oldProtect);
        FlushInstructionCache(GetCurrentProcess(), site, 5);
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * InitBridge — replicates LevelBoard_Intermediate_ctor bridge setup
 *
 * Steps (from Ghidra decompilation of 0x0041cb20):
 *   1. operator_new(0x10d0) → Level_MeshWorldCtor(mem, gfx, "Levels\\Level2-Bridge") → board+0x436C
 *   2. operator_new(0x10d0) → Level_RenderCtor(mem, meshWorld) → board+0x4370
 *   3. TipperVisual_Attach(renderObj, meshWorld)
 *   4. board+0x4380 = 0x42340000 (float 45.0)
 *   5. board+0x4384 = 0
 *   6. board+0x4388 = 0x32 (50)
 * ═══════════════════════════════════════════════════════════════════════════ */

static void InitBridge(void *board) {
    if (!g_operatorNew || !g_LevelMeshWorldCtor || !g_LevelRenderCtor ||
        !g_TipperVisualAttach) return;

    DWORD app = *(DWORD *)((char *)board + BOARD_APP_PTR);
    if (!app || IsBadReadPtr((void *)app, 0x200)) return;
    void *gfx = *(void **)((char *)app + 0x174);
    if (!gfx) return;

    /* Step 1: MeshWorld */
    void *meshMem = g_operatorNew(0x10D0);
    if (!meshMem) return;
    void *meshWorld = g_LevelMeshWorldCtor(meshMem, gfx, "Levels\\Level2-Bridge");
    *(void **)((char *)board + BRIDGE_MESHWORLD) = meshWorld;

    /* Step 2: RenderObj */
    void *renderMem = g_operatorNew(0x10D0);
    void *renderObj = NULL;
    if (renderMem) {
        renderObj = g_LevelRenderCtor(renderMem, meshWorld);
    }
    *(void **)((char *)board + BRIDGE_RENDEROBJ) = renderObj;

    /* Step 3: TipperVisual_Attach */
    if (renderObj && meshWorld) {
        g_TipperVisualAttach(renderObj, meshWorld);
    }

    /* Steps 4-6: Bridge config values */
    *(DWORD *)((char *)board + BRIDGE_PARAM1) = 0x42340000;  /* 45.0f */
    *(DWORD *)((char *)board + BRIDGE_PARAM2) = 0;
    *(DWORD *)((char *)board + BRIDGE_PARAM3) = 0x32;       /* 50 */
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Universal Post-Setup — config-driven feature initialization
 * ═══════════════════════════════════════════════════════════════════════════ */

static void UniversalPostSetup(void *board) {
    int level = GetCurrentLevel(board);
    if (level == 0) return;

    LoadConfig();

    /* Bridge */
    if (g_objectEnabled[OBJ_BRIDGE][level]) {
        InitBridge(board);
    }

    /* Bumpers */
    if (g_objectEnabled[OBJ_BUMPERS][level]) {
        DWORD meshWorld = *(DWORD *)((char *)board + BOARD_MESHWORLD);
        if (!meshWorld || IsBadReadPtr((void *)meshWorld, 0x430)) return;

        int i;
        for (i = 0; i < 8; i++) {
            char nameBuf[16];
            const char *prefix = "N:BUMPER";
            int p = 0, j;
            for (j = 0; prefix[j]; j++) nameBuf[p++] = prefix[j];
            int num = i + 1;
            if (num >= 10) { nameBuf[p++] = '0' + (num / 10); num %= 10; }
            nameBuf[p++] = '0' + num;
            nameBuf[p] = '\0';

            void *dest = (char *)board + BUMPER_SLOT_BASE + i * BUMPER_SLOT_STRIDE;

            if (g_AthenaListInit) {
                g_AthenaListInit(dest, 0);
            }
            if (g_CollectByNameFilter) {
                g_CollectByNameFilter((void *)meshWorld, nameBuf, dest);
            }
            *(DWORD *)((char *)board + BUMPER_LIT_BASE + i * BUMPER_LIT_STRIDE) = 0;
        }
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Universal Scene Constructor — REPLACES vtable[0x48] (Scene_LoadLevel*)
 * ═══════════════════════════════════════════════════════════════════════════ */

static void UniversalConstructor(void *board, int raceIndex) {
    char buf[256];
    wsprintfA(buf, "UniversalConstructor called: board=%p raceIndex=%d", board, raceIndex);
    DebugLog(buf);
    if (!board || raceIndex < 1 || raceIndex > 15) {
        DebugLog("UniversalConstructor: invalid params");
        return;
    }
    if (!g_operatorNew || !g_LevelMeshWorldCtor || !g_LevelRenderCtor ||
        !g_LevelInitScene) {
        DebugLog("UniversalConstructor: function pointers not resolved");
        return;
    }

    /* Use meshPath from LevelData if available, fallback to g_meshPaths */
    const char *meshPath = g_levelData[raceIndex].meshPath;
    if (!meshPath || !*meshPath) meshPath = g_meshPaths[raceIndex];
    if (!meshPath) return;

    DWORD app = *(DWORD *)((char *)board + BOARD_APP_PTR);
    if (!app || IsBadReadPtr((void *)app, 0x200)) return;
    void *gfx = *(void **)((char *)app + 0x174);
    if (!gfx) return;

    /* Step 1: MeshWorld */
    void *meshMem = g_operatorNew(0x10D0);
    if (!meshMem) return;
    void *meshWorld = g_LevelMeshWorldCtor(meshMem, gfx, meshPath);
    *(DWORD *)((char *)board + BOARD_MESHWORLD) = (DWORD)meshWorld;

    /* Step 2: RenderObj */
    void *renderMem = g_operatorNew(0x10D0);
    void *renderObj = NULL;
    if (renderMem) {
        renderObj = g_LevelRenderCtor(renderMem, meshWorld);
    }
    *(DWORD *)((char *)board + BOARD_RENDEROBJ) = (DWORD)renderObj;

    /* Step 3: InitScene */
    g_LevelInitScene(board);

    /* Step 4: Board_Setup via vtable[0x80] */
    DWORD vtable = *(DWORD *)board;
    if (vtable && !IsBadReadPtr((void *)vtable, 0x84)) {
        void (__thiscall *boardSetup)(void *) = *(void (__thiscall **)(void *))((char *)vtable + 0x80);
        if (boardSetup) boardSetup(board);
    }

    /* Step 5: Config-driven features */
    UniversalPostSetup(board);
}

/* Must be non-static for asm reference */
void __cdecl UniversalConstructorLogic(void *board, int raceIndex) {
    UniversalConstructor(board, raceIndex);
}

__attribute__((naked)) static void Hook_UniversalConstructor(void) {
    __asm__ __volatile__(
        "pushl %%ebp\n\t"
        "movl  %%esp, %%ebp\n\t"
        "pushl %%edx\n\t"
        "pushl %%ecx\n\t"

        "movl  0x08(%%esi), %%eax\n\t"
        "pushl %%eax\n\t"
        "pushl %%ecx\n\t"

        "call  _UniversalConstructorLogic\n\t"
        "addl  $8, %%esp\n\t"

        "popl  %%ecx\n\t"
        "popl  %%edx\n\t"
        "popl  %%ebp\n\t"

        "jmpl  *_g_ctorTrampoline\n\t"
        :: : "eax", "memory"
    );
}

/* Must be non-static for asm reference */
unsigned char *g_ctorTrampoline = NULL;

static void InstallUniversalConstructorHook(void) {
    DWORD targetAddr = g_moduleBase + 0x000273E0;
    unsigned char *orig = (unsigned char *)targetAddr;

    if (orig[0] != 0xFF || orig[1] != 0x52 || orig[2] != 0x48 ||
        orig[3] != 0x8B || orig[4] != 0x4E || orig[5] != 0x04) return;

    g_ctorTrampoline = VirtualAlloc(NULL, 16,
                        MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!g_ctorTrampoline) return;

    g_ctorTrampoline[0] = 0x8B;  /* MOV ECX, [ESI+4] */
    g_ctorTrampoline[1] = 0x4E;
    g_ctorTrampoline[2] = 0x04;
    g_ctorTrampoline[3] = 0xE9;  /* JMP back to original+6 */
    *(DWORD *)(g_ctorTrampoline + 4) = (targetAddr + 6) - ((DWORD)g_ctorTrampoline + 8);

    DWORD oldProtect;
    VirtualProtect(orig, 6, PAGE_EXECUTE_READWRITE, &oldProtect);
    orig[0] = 0xE9;
    *(DWORD *)(orig + 1) = (DWORD)&Hook_UniversalConstructor - (targetAddr + 5);
    orig[5] = 0x90;
    VirtualProtect(orig, 6, oldProtect, &oldProtect);
    FlushInstructionCache(GetCurrentProcess(), orig, 6);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Allocation size patch
 * ═══════════════════════════════════════════════════════════════════════════ */

static const DWORD g_allocPatchRVAs[15] = {
    0x00027109, 0x00027136, 0x00027167, 0x00027198, 0x000271C9,
    0x000271FA, 0x0002722B, 0x0002725C, 0x0002728D, 0x000272BE,
    0x000272EF, 0x00027320, 0x00027351, 0x0002737B, 0x000273A5,
};

static void PatchAllocSizes(void) {
    int i;
    for (i = 0; i < 15; i++) {
        unsigned char *site = (unsigned char *)(g_moduleBase + g_allocPatchRVAs[i]);
        if (IsBadReadPtr(site, 5)) continue;
        if (site[0] != 0x68) continue;

        DWORD oldProtect;
        VirtualProtect(site, 5, PAGE_EXECUTE_READWRITE, &oldProtect);
        *(DWORD *)(site + 1) = UNION_SIZE;
        VirtualProtect(site, 5, oldProtect, &oldProtect);
        FlushInstructionCache(GetCurrentProcess(), site, 5);
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Install collision hook
 * ═══════════════════════════════════════════════════════════════════════════ */

static void InstallHook(void) {
    DWORD targetAddr = g_moduleBase + RVA_DispatchCollisionEvents;
    unsigned char *orig = (unsigned char *)targetAddr;

    if (orig[0] != 0x6A || orig[1] != 0xFF ||
        orig[2] != 0x64 || orig[3] != 0xA1) return;

    g_trampoline = VirtualAlloc(NULL, 16,
                    MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!g_trampoline) return;

    memcpy(g_trampoline, orig, 8);
    g_trampoline[8] = 0xE9;
    *(DWORD *)(g_trampoline + 9) = (targetAddr + 8) - ((DWORD)g_trampoline + 13);

    g_OriginalDispatch = (DispatchCollisionEvents_t)g_trampoline;

    DWORD oldProtect;
    VirtualProtect(orig, 16, PAGE_EXECUTE_READWRITE, &oldProtect);
    orig[0] = 0xE9;
    *(DWORD *)(orig + 1) = (DWORD)&Hook_DispatchCollisionEvents - (targetAddr + 5);
    orig[5] = 0x90;
    orig[6] = 0x90;
    orig[7] = 0x90;
    VirtualProtect(orig, 16, oldProtect, &oldProtect);
    FlushInstructionCache(GetCurrentProcess(), orig, 16);
}

/* BASS proxy exports handled by bass.def - DLL forwarding to bass_real.dll */

/* ═══════════════════════════════════════════════════════════════════════════
 * Debug logging
 * ═══════════════════════════════════════════════════════════════════════════ */

void DebugLog(const char *msg) {
    HANDLE hFile = CreateFileA("C:\\lsdebug.log", GENERIC_WRITE,
                               FILE_SHARE_READ, NULL, OPEN_ALWAYS,
                               FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return;
    SetFilePointer(hFile, 0, NULL, FILE_END);
    DWORD written;
    WriteFile(hFile, msg, strlen(msg), &written, NULL);
    WriteFile(hFile, "\r\n", 2, &written, NULL);
    CloseHandle(hFile);
}

static DWORD WINAPI PatchThread(LPVOID param) {
    DebugLog("=== PatchThread started ===");
    Sleep(2000);
    DebugLog("Sleep done, resolving module base");
    g_moduleBase = (DWORD)GetModuleHandleA("Hamsterball.exe");
    if (!g_moduleBase) g_moduleBase = 0x00400000;

    g_operatorNew = (operator_new_t)(g_moduleBase + RVA_operator_new);
    g_LevelMeshWorldCtor = (Level_MeshWorldCtor_t)(g_moduleBase + RVA_Level_MeshWorldCtor);
    g_LevelRenderCtor = (Level_RenderCtor_t)(g_moduleBase + RVA_Level_RenderCtor);
    g_LevelInitScene = (Level_InitScene_t)(g_moduleBase + RVA_Level_InitScene);
    g_SoundPlay3D = (Sound_Play3D_t)(g_moduleBase + RVA_Sound_Play3D);
    g_CollectByNameFilter = (Scene_CollectByNameFilter_t)(g_moduleBase + RVA_Scene_CollectByNameFilter);
    g_AthenaListInit = (AthenaList_Init_t)(g_moduleBase + RVA_AthenaList_Init);
    g_BoardCtor = (Board_ctor_t)(g_moduleBase + RVA_Board_ctor);
    g_LoadRaceData = (LoadRaceData_t)(g_moduleBase + RVA_LoadRaceData);
    g_Vec3Init = (Vec3_Init_t)(g_moduleBase + RVA_Vec3_Init);
    g_MatrixIdentity = (Matrix_Identity_t)(g_moduleBase + RVA_Matrix_Identity);
    g_MeshNodeCtor = (MeshNode_ctor_t)(g_moduleBase + RVA_MeshNode_ctor);
    g_SpriteCtor = (Sprite_ctor_t)(g_moduleBase + RVA_Sprite_ctor);
    g_TipperVisualAttach = (TipperVisual_Attach_t)(g_moduleBase + RVA_TipperVisual_Attach);
    g_LevelAssignTex = (Level_AssignTex_t)(g_moduleBase + RVA_Level_AssignTexAndScales);
    DebugLog("Function pointers resolved");

    GetConfigPath();
    LoadConfig();

    if (GetFileAttributesA(g_levelDataPath) == INVALID_FILE_ATTRIBUTES) {
        GenerateLevelData();
    }
    LoadLevelData();

    PatchAllocSizes();
    InstallBoardCtorHooks();
    InstallUniversalConstructorHook();
    InstallHook();
    DebugLog("=== PatchThread complete ===");
    return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    if (fdwReason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hinstDLL);
        CreateThread(NULL, 0, PatchThread, NULL, 0, NULL);
    }
    return TRUE;
}
