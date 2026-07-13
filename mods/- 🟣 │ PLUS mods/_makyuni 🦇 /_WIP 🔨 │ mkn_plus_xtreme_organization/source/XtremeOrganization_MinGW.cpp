/*
 * XtremeOrganization_MinGW.cpp — Xtreme File Organization (HB+ v2.1, MinGW)
 *
 * A comprehensive file redirection mod that lets users customize:
 *   - Level MESHWORLD files (15 races + 15 arenas)
 *   - Mesh files (8Ball, FunBall, Sphere, hammyjudge, etc.)
 *   - Level object files (Swirl, Bridge, Windmill, etc.)
 *   - Tournament menu thumbnails (15)
 *   - Practice menu thumbnails (15)
 *   - Cache file generation (ignore_cache)
 *
 * The mod works by patching PUSH imm32 instructions in Hamsterball.exe.
 * Each PUSH loads a string pointer (e.g. "levels\\level1"). We overwrite
 * the 4-byte immediate to point to our own string allocated in DLL memory.
 *
 * Config file: mkn_plus_xtreme_organization.txt (next to DLL in Mods\ folder)
 * Format: key = value pairs, one per line. Comments start with #.
 *
 * Uses nocrt + manual 17-entry vtable for HB+ v2.0/v2.1.
 */
#include "nocrt.h"
#include "HamsterballAPI.h"
#include "hbplus_api.h"

#define MAX_STRING_LEN 128
#define MAX_LINE_LEN 256

/* ── Default file paths ──────────────────────────────────────────────── */

static const char* DEFAULT_RACE_PATHS[15] = {
    "levels\\level1",       /* 1  Warm-Up       */
    "levels\\levelcascade", /* 2  Beginner      */
    "levels\\level2",       /* 3  Intermediate  */
    "levels\\level3",       /* 4  Dizzy         */
    "levels\\level4",       /* 5  Tower         */
    "levels\\levelup",      /* 6  Up            */
    "levels\\leveldark",    /* 7  Neon          */
    "levels\\level5",       /* 8  Expert        */
    "levels\\level6",       /* 9  Odd           */
    "levels\\level8",       /* 10 Toob          */
    "levels\\level7",       /* 11 Wobbly        */
    "levels\\levelglass",   /* 12 Glass         */
    "levels\\level9",       /* 13 Sky           */
    "levels\\level10",      /* 14 Master        */
    "levels\\levelimpossible" /* 15 Impossible   */
};

static const char* DEFAULT_ARENA_PATHS[15] = {
    "levels\\arena-WarmUp",        "levels\\arena-beginner",
    "levels\\arena-intermediate",   "levels\\arena-dizzy",
    "levels\\arena-tower",          "levels\\arena-up",
    "levels\\arena-neon",           "levels\\arena-expert",
    "levels\\arena-Odd",            "levels\\arena-Toob",
    "levels\\arena-Wobbly",         "levels\\arena-Sky",
    "levels\\arena-Master",         "levels\\arena-glass",
    "levels\\arena-impossible"
};

/* ── PUSH instruction RVAs for level/arena paths ─────────────────────── */
/* From mkn_custom_filenames mod — verified against binary */

static const DWORD RACE_PUSH_RVAS[15] = {
    0x0d202, 0x11115, 0x0d2c2, 0x0d3d4, 0x0d712,
    0x1158b, 0x162b5, 0x0e1d2, 0x0ead3, 0x0fa65,
    0x0f3a2, 0x17683, 0x10874, 0x11fa2, 0x17f62
};

static const DWORD ARENA_PUSH_RVAS[15] = {
    0x13c62, 0x13d25, 0x141c2, 0x14282, 0x144f2,
    0x149a2, 0x16f83, 0x14b52, 0x14d22, 0x14f45,
    0x153e2, 0x17e32, 0x15904, 0x160c2, 0x18582
};

/* ── Tournament thumbnail PUSH RVAs ──────────────────────────────────── */
/* Tournament thumbnails are in reverse order (15→1) in the binary */

static const DWORD TOURNEY_THUMB_RVAS[15] = {
    0x2a35a, /* 1  Warm-Up       tourney-beginner.png     */
    0x2a371, /* 2  Beginner      tourney-cascade.png      */
    0x2a388, /* 3  Intermediate  tourney-intermediate.png */
    0x2a39f, /* 4  Dizzy         tourney-dizzy.png        */
    0x2a3b6, /* 5  Tower         tourney-Tower.png        */
    0x2a3cd, /* 6  Up            tourney-Up.png           */
    0x2a3e4, /* 7  Neon          tourney-Neon.png         */
    0x2a3fb, /* 8  Expert        tourney-Expert.png       */
    0x2a412, /* 9  Odd           tourney-Odd.png          */
    0x2a429, /* 10 Toob          tourney-Toob.png         */
    0x2a440, /* 11 Wobbly        tourney-Wobbly.png       */
    0x2a457, /* 12 Glass         tourney-Glass.png        */
    0x2a46e, /* 13 Sky           tourney-Sky.png          */
    0x2a485, /* 14 Master        tourney-Master.png       */
    0x2a49c  /* 15 Impossible    tourney-Impossible.png   */
};

/* ── Practice thumbnail PUSH RVAs ────────────────────────────────────── */

static const DWORD PRACTICE_THUMB_RVAS[15] = {
    0x2eac1, /* 1  Warm-Up       practice-level1.png       */
    0x2eb03, /* 2  Beginner      practice-cascade.png      */
    0x2eb45, /* 3  Intermediate  practice-level2.png       */
    0x2eb87, /* 4  Dizzy         practice-level3.png       */
    0x2ebc9, /* 5  Tower         practice-level4.png       */
    0x2ec0b, /* 6  Up            practice-up.png           */
    0x2ec4d, /* 7  Neon          practice-level5.png       */
    0x2ec8f, /* 8  Expert        practice-level6.png       */
    0x2ecd1, /* 9  Odd           practice-level7.png       */
    0x2ed13, /* 10 Toob          practice-level8.png       */
    0x2ed55, /* 11 Wobbly        practice-level9.png       */
    0x2ed97, /* 12 Glass         practice-level10.png      */
    0x2edd9, /* 13 Sky           practice-neon.png         */
    0x2ee1b, /* 14 Master        practice-glass.png        */
    0x2ee5d  /* 15 Impossible    practice-impossible.png   */
};

/* ── Mesh file PUSH RVAs ────────────────────────────────────────────── */
/* All "Meshes\*" and "meshes\*" strings with their PUSH instruction RVAs */

typedef struct {
    const char* key;         /* config key (lowercase) */
    const char* default_path; /* default game path */
    DWORD push_rva;          /* RVA of PUSH instruction */
} MeshEntry;

static const MeshEntry MESH_ENTRIES[] = {
    /* Ball meshes */
    {"mesh_file_8ball",          "Meshes\\8Ball",            0x29ef1},
    {"mesh_file_funball",        "Meshes\\FunBall",          0x29f08},
    {"mesh_file_sphere",         "Meshes\\Sphere",           0x29e1a},
    {"mesh_file_spherebreak1",   "Meshes\\SphereBreak1",     0x29e39},
    {"mesh_file_spherebreak2",   "Meshes\\SphereBreak2",     0x29e50},
    {"mesh_file_sphere_tar",     "Meshes\\Sphere+Tar",       0x29e67},
    {"mesh_file_rbsglare",       "Meshes\\RBGlare",          0x29e7e},
    {"mesh_file_dizzy",          "Meshes\\Dizzy",            0x29f36},
    {"mesh_file_bell",           "Meshes\\Bell",             0x29f1f},
    {"mesh_file_hammyjudge",     "meshes\\hammyjudge",       0x1ebdb},
    /* Hamster animation meshes */
    {"mesh_file_hamster_trot1",  "Meshes\\Hamster-trot1",    0x29eac},
    {"mesh_file_hamster_trot2",  "Meshes\\Hamster-trot2",    0x29ec3},
    {"mesh_file_hamster_trot3",  "Meshes\\Hamster-trot3",    0x29eda},
    {"mesh_file_hamster_waiting","Meshes\\Hamster-Waiting",  0x29e95},
    /* Object meshes */
    {"mesh_file_glassbonus",         "Meshes\\GlassBonus",           0x2a32c},
    {"mesh_file_glassbonus_smashed", "Meshes\\GlassBonus-Smashed",   0x2a343},
    {"mesh_file_mousetrapshadow",    "Meshes\\mousetrapshadow",      0x2a0f8},
    {"mesh_file_sawface2",           "Meshes\\sawface2",             0x2a0e1},
    {"mesh_file_sawface",            "Meshes\\sawface",              0x2a0ca},
    {"mesh_file_dawgshadow",         "Meshes\\dawgshadow",           0x2a0b3},
    {"mesh_file_dawgshoe2",          "Meshes\\dawgshoe2",            0x2a09c},
    {"mesh_file_dawgshoe",           "Meshes\\dawgshoe",             0x2a085},
    {"mesh_file_sawblade",           "Meshes\\sawblade",             0x2a06e},
    {"mesh_file_fanbody",            "Meshes\\fanbody",              0x2a057},
    {"mesh_file_fanblades",          "Meshes\\fanblades",            0x2a040},
    {"mesh_file_tarbubble",          "Meshes\\tarbubble",            0x2a029},
    {"mesh_file_chomper",            "Meshes\\Chomper",              0x1e5d2},
    {"mesh_file_yellowlink",         "Meshes\\YellowLink",           0x1e50f},
    {"mesh_file_magnifyingglass",    "meshes\\magnifyingglass",      0x1fa6b},
    {"mesh_file_skypillar",          "meshes\\skypillar",            0x1fa2d},
};

#define MESH_COUNT (sizeof(MESH_ENTRIES) / sizeof(MeshEntry))

/* ── Level object file PUSH RVAs ────────────────────────────────────── */
/* "Levels\LevelX-ObjectName" strings, used for LoadMeshWorld calls */

typedef struct {
    const char* key;
    const char* default_path;
    DWORD push_rva;
} ObjectEntry;

static const ObjectEntry OBJ_ENTRIES[] = {
    /* Level 2 (Intermediate) */
    {"obj_file_bridge",          "Levels\\Level2-Bridge",          0x1cb90},
    /* Level 3 (Dizzy) */
    {"obj_file_gluebie",         "Levels\\Level3-Gluebie",         0x1d314},
    {"obj_file_waterwheel",      "Levels\\Level3-WaterWheel",      0x1d1f2},
    {"obj_file_tipper",          "Levels\\Level3-Tipper",          0x1d159},
    {"obj_file_swirl",           "Levels\\Level3-Swirl",          0x1d27b},
    /* Level 4 (Tower) */
    {"obj_file_turret",          "Levels\\Level4-Turret",          0x1e614},
    {"obj_file_windmill",        "Levels\\Level4-Windmill",        0x1e593},
    {"obj_file_mace",            "Levels\\Level4-Mace",            0x1e551},
    {"obj_file_drawbridge",      "Levels\\Level4-Drawbridge",      0x1e4d0},
    {"obj_file_catapult",        "Levels\\Level4-Catapult",        0x1e48e},
    {"obj_file_trapdoor1",       "Levels\\Level4-Trapdoor1",       0x29fab},
    {"obj_file_trapdoor2",       "Levels\\Level4-Trapdoor2",       0x29fc2},
    /* Level 5 (Expert) */
    {"obj_file_expert_bridge",   "Levels\\Level5-Bridge",          0x1eb65},
    {"obj_file_bonk",            "levels\\level5-bonk",            0x3887d},
    /* Level 6 (Odd) */
    {"obj_file_lifter_odd",      "Levels\\Level6-Lifter",          0x2a10f},
    /* Level 7 (Wobbly) */
    {"obj_file_wavy1",           "Levels\\Level7-Wavy1",           0x0f5bb},
    {"obj_file_wobbly1",         "Levels\\Level7-Wobbly1",         0x1f200},
    {"obj_file_wobbly2",         "Levels\\Level7-Wobbly2",         0x1f241},
    {"obj_file_wobbly3",         "Levels\\Level7-Wobbly3",         0x1f282},
    {"obj_file_wobbly4",         "Levels\\Level7-Wobbly4",         0x1f2c3},
    {"obj_file_wobbly5",         "Levels\\Level7-Wobbly5",         0x1f304},
    {"obj_file_wobbly6",         "Levels\\Level7-Wobbly6",         0x1f345},
    {"obj_file_wobbly7",         "Levels\\Level7-Wobbly7",         0x1f386},
    {"obj_file_wobbly8",         "Levels\\Level7-Wobbly8",         0x23757},
    /* Level 8 (Toob) */
    {"obj_file_blockdawg1",      "Levels\\Level8-Blockdawg1",      0x1f688},
    {"obj_file_blockdawg2",      "Levels\\Level8-Blockdawg2",      0x1f6c9},
    {"obj_file_fallout",         "Levels\\Level8-Fallout",         0x1f647},
    {"obj_file_saw",             "Levels\\Level8-Saw",             0x1f606},
    {"obj_file_spinny",          "Levels\\Level8-Spinny",          0x1f5c5},
    /* Level 9 (Sky) */
    {"obj_file_sky_trapdoor",    "levels\\level9-trapdoor",       0x1fb2e},
    {"obj_file_popcylinder1",    "levels\\level9-popcylinder1",   0x1faac},
    {"obj_file_popcylinder2",    "levels\\level9-popcylinder2",   0x1faed},
    /* Level Up */
    {"obj_file_up_button",       "levels\\levelup-button",        0x20521},
    {"obj_file_up_speedcyl",     "levels\\levelup-speedcylinder",  0x204e0},
    {"obj_file_up_lifter",       "levels\\levelup-lifter",        0x2049f},
    /* Level 10 (Master) */
    {"obj_file_master_bridge1",  "Levels\\Level10-Bridge1",       0x20960},
    {"obj_file_master_bridge2",  "Levels\\Level10-Bridge2",       0x209a1},
    {"obj_file_2pbridge",        "Levels\\Level10-2PBridge",      0x20856},
    /* Neon (Dark) */
    {"obj_file_trode",           "Levels\\LevelDark-Trode",       0x24658},
    {"obj_file_dfloor1",         "Levels\\LevelDark-DFloor1",     0x24554},
    {"obj_file_dfloor2",         "Levels\\LevelDark-DFloor2",     0x24595},
    {"obj_file_dfloor3",         "Levels\\LevelDark-DFloor3",     0x245d6},
    {"obj_file_dfloor4",         "Levels\\LevelDark-DFloor4",     0x24617},
    {"obj_file_neonplatform",    "Levels\\LevelDark-NeonPlatform",0x24513},
    {"obj_file_flickring",       "Levels\\LevelDark-Flickning",   0x24924},
    /* Impossible */
    {"obj_file_pendulum",        "Levels\\LevelImpossible-Pendulum", 0x24dee},
    {"obj_file_rotator",         "Levels\\LevelImpossible-Rotator", 0x24dad},
    {"obj_file_biggear",         "Levels\\LevelImpossible-BigGear",  0x24d6c},
    {"obj_file_gear",            "Levels\\LevelImpossible-Gear",     0x24d2b},
    {"obj_file_looper",          "Levels\\LevelImpossible-Looper",   0x24cea},
    /* Shared objects */
    {"obj_file_popupsign",       "Levels\\PopupSign",             0x29fff},
    {"obj_file_secret_unlock",   "Levels\\Secret-Unlock",         0x29f81},
    {"obj_file_secret",          "Levels\\Secret",                0x29f6a},
    {"obj_file_mousetrap",       "Levels\\MouseTrap",             0x29f4b},
};

#define OBJ_COUNT (sizeof(OBJ_ENTRIES) / sizeof(ObjectEntry))

/* ── String storage (kept alive for game lifetime) ───────────────────── */

static char g_race_strings[15][MAX_STRING_LEN];
static char g_arena_strings[15][MAX_STRING_LEN];
static char g_tourney_thumb_strings[15][MAX_STRING_LEN];
static char g_practice_thumb_strings[15][MAX_STRING_LEN];
static char g_mesh_strings[MESH_COUNT][MAX_STRING_LEN];
static char g_obj_strings[OBJ_COUNT][MAX_STRING_LEN];

static bool g_ignoreCache = true;
static bool g_configLoaded = false;
static char g_configPath[MAX_PATH] = "";
static bool g_pathReady = false;
static void* g_storedApi = NULL;

/* ── Config path: find THIS DLL's folder ─────────────────────────────── */

static void buildConfigPath(void) {
    MEMORY_BASIC_INFORMATION mbi;
    char dllPath[MAX_PATH];
    if (VirtualQuery((void*)buildConfigPath, &mbi, sizeof(mbi)) > 0) {
        HMODULE hMod = (HMODULE)mbi.AllocationBase;
        if (hMod && GetModuleFileNameA(hMod, dllPath, MAX_PATH) > 0) {
            char* last = nc_strrchr(dllPath, '\\');
            if (!last) last = nc_strrchr(dllPath, '/');
            if (last) {
                *(last + 1) = '\0';
                nc_strncpy(g_configPath, dllPath, MAX_PATH - 1);
                nc_strncpy(g_configPath + nc_strlen(g_configPath),
                           "mkn_plus_xtreme_organization.txt",
                           MAX_PATH - nc_strlen(g_configPath) - 1);
                g_configPath[MAX_PATH - 1] = '\0';
                g_pathReady = true;
                return;
            }
        }
    }
    nc_strncpy(g_configPath, "mkn_plus_xtreme_organization.txt", MAX_PATH - 1);
    g_configPath[MAX_PATH - 1] = '\0';
    g_pathReady = true;
}

/* ── Create default config file if it doesn't exist ──────────────────── */

static void createDefaultConfig(void) {
    if (!g_pathReady) return;
    HANDLE h = CreateFileA(g_configPath, GENERIC_READ, FILE_SHARE_READ, NULL,
                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h != INVALID_HANDLE_VALUE) {
        CloseHandle(h);
        return; /* File exists */
    }

    h = CreateFileA(g_configPath, GENERIC_WRITE, 0, NULL,
                    CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return;

    static char buf[16384];
    int pos = 0;

    pos += nc_snprintf(buf + pos, sizeof(buf) - pos,
        "# mkn_plus_xtreme_organization.txt\r\n"
        "# Xtreme File Organization config file\r\n"
        "# Lines starting with # are comments. Format: key = value\r\n"
        "\r\n");

    pos += nc_snprintf(buf + pos, sizeof(buf) - pos,
        "# Ignore cache: if true, the game will not read or write .cached files\r\n"
        "ignore_cache = true\r\n"
        "\r\n");

    pos += nc_snprintf(buf + pos, sizeof(buf) - pos,
        "# ── Level MESHWORLD files (Tournament races 1-15) ──\r\n");
    for (int i = 0; i < 15; i++) {
        pos += nc_snprintf(buf + pos, sizeof(buf) - pos,
            "level%02d_file = %s\r\n", i + 1, DEFAULT_RACE_PATHS[i]);
    }

    pos += nc_snprintf(buf + pos, sizeof(buf) - pos,
        "\r\n# ── Arena MESHWORLD files (Rumble mode 1-15) ──\r\n");
    for (int i = 0; i < 15; i++) {
        pos += nc_snprintf(buf + pos, sizeof(buf) - pos,
            "arena%02d_file = %s\r\n", i + 1, DEFAULT_ARENA_PATHS[i]);
    }

    pos += nc_snprintf(buf + pos, sizeof(buf) - pos,
        "\r\n# ── Mesh files (.MESH) ──\r\n");
    for (int i = 0; i < (int)MESH_COUNT; i++) {
        pos += nc_snprintf(buf + pos, sizeof(buf) - pos,
            "%s = %s\r\n", MESH_ENTRIES[i].key, MESH_ENTRIES[i].default_path);
    }

    pos += nc_snprintf(buf + pos, sizeof(buf) - pos,
        "\r\n# ── Level object files (MESHWORLD sub-objects) ──\r\n");
    for (int i = 0; i < (int)OBJ_COUNT; i++) {
        pos += nc_snprintf(buf + pos, sizeof(buf) - pos,
            "%s = %s\r\n", OBJ_ENTRIES[i].key, OBJ_ENTRIES[i].default_path);
    }

    pos += nc_snprintf(buf + pos, sizeof(buf) - pos,
        "\r\n# ── Tournament menu thumbnails (1-15) ──\r\n");
    static const char* tourney_defaults[15] = {
        "tourney-beginner.png", "tourney-cascade.png",
        "tourney-intermediate.png", "tourney-dizzy.png",
        "tourney-Tower.png", "tourney-Up.png",
        "tourney-Neon.png", "tourney-Expert.png",
        "tourney-Odd.png", "tourney-Toob.png",
        "tourney-Wobbly.png", "tourney-Glass.png",
        "tourney-Sky.png", "tourney-Master.png",
        "tourney-Impossible.png"
    };
    for (int i = 0; i < 15; i++) {
        pos += nc_snprintf(buf + pos, sizeof(buf) - pos,
            "img_file_tourneythumb%02d = %s\r\n", i + 1, tourney_defaults[i]);
    }

    pos += nc_snprintf(buf + pos, sizeof(buf) - pos,
        "\r\n# ── Practice (Time Trials) menu thumbnails (1-15) ──\r\n");
    static const char* practice_defaults[15] = {
        "Textures\\practice-level1.png",
        "Textures\\practice-cascade.png",
        "Textures\\practice-level2.png",
        "Textures\\practice-level3.png",
        "Textures\\practice-level4.png",
        "Textures\\practice-up.png",
        "Textures\\practice-level5.png",
        "Textures\\practice-level6.png",
        "Textures\\practice-level7.png",
        "Textures\\practice-level8.png",
        "Textures\\practice-level9.png",
        "Textures\\practice-level10.png",
        "Textures\\practice-neon.png",
        "Textures\\practice-glass.png",
        "Textures\\practice-impossible.png"
    };
    for (int i = 0; i < 15; i++) {
        pos += nc_snprintf(buf + pos, sizeof(buf) - pos,
            "img_file_practicethumb%02d = %s\r\n", i + 1, practice_defaults[i]);
    }

    DWORD written = 0;
    WriteFile(h, buf, pos, &written, NULL);
    CloseHandle(h);
}

/* ── Parse config file ───────────────────────────────────────────────── */

static void trim(char* s) {
    /* Trim leading whitespace */
    char* p = s;
    while (*p == ' ' || *p == '\t') p++;
    if (p != s) {
        size_t len = nc_strlen(p);
        nc_memcpy(s, p, len + 1);
    }
    /* Trim trailing whitespace/CR/LF */
    size_t len = nc_strlen(s);
    while (len > 0 && (s[len-1] == '\n' || s[len-1] == '\r' ||
                       s[len-1] == ' '  || s[len-1] == '\t')) {
        s[--len] = '\0';
    }
}

static void parseKeyValue(char* key, char* value) {
    trim(key);
    trim(value);
    if (key[0] == '\0' || value[0] == '\0') return;

    /* ignore_cache */
    if (nc_stricmp(key, "ignore_cache") == 0) {
        g_ignoreCache = (nc_stricmp(value, "true") == 0 ||
                         nc_stricmp(value, "1") == 0 ||
                         nc_stricmp(value, "yes") == 0);
        return;
    }

    /* Level files: level01_file through level15_file */
    if (nc_stricmp(key, "level01_file") >= 0 && nc_stricmp(key, "level15_file") <= 0) {
        /* Extract number from "levelXX_file" */
        int slot = 0;
        char* p = key + 5; /* skip "level" */
        while (*p >= '0' && *p <= '9') { slot = slot * 10 + (*p - '0'); p++; }
        if (slot >= 1 && slot <= 15) {
            nc_strncpy(g_race_strings[slot-1], value, MAX_STRING_LEN - 1);
            g_race_strings[slot-1][MAX_STRING_LEN-1] = '\0';
        }
        return;
    }

    /* Arena files: arena01_file through arena15_file */
    if (nc_stricmp(key, "arena01_file") >= 0 && nc_stricmp(key, "arena15_file") <= 0) {
        int slot = 0;
        char* p = key + 5; /* skip "arena" */
        while (*p >= '0' && *p <= '9') { slot = slot * 10 + (*p - '0'); p++; }
        if (slot >= 1 && slot <= 15) {
            nc_strncpy(g_arena_strings[slot-1], value, MAX_STRING_LEN - 1);
            g_arena_strings[slot-1][MAX_STRING_LEN-1] = '\0';
        }
        return;
    }

    /* Tournament thumbnails: img_file_tourneythumb01 through 15 */
    if (nc_stricmp(key, "img_file_tourneythumb01") >= 0 &&
        nc_stricmp(key, "img_file_tourneythumb15") <= 0) {
        int slot = 0;
        char* p = key + 21; /* skip "img_file_tourneythumb" */
        while (*p >= '0' && *p <= '9') { slot = slot * 10 + (*p - '0'); p++; }
        if (slot >= 1 && slot <= 15) {
            nc_strncpy(g_tourney_thumb_strings[slot-1], value, MAX_STRING_LEN - 1);
            g_tourney_thumb_strings[slot-1][MAX_STRING_LEN-1] = '\0';
        }
        return;
    }

    /* Practice thumbnails: img_file_practicethumb01 through 15 */
    if (nc_stricmp(key, "img_file_practicethumb01") >= 0 &&
        nc_stricmp(key, "img_file_practicethumb15") <= 0) {
        int slot = 0;
        char* p = key + 22; /* skip "img_file_practicethumb" */
        while (*p >= '0' && *p <= '9') { slot = slot * 10 + (*p - '0'); p++; }
        if (slot >= 1 && slot <= 15) {
            nc_strncpy(g_practice_thumb_strings[slot-1], value, MAX_STRING_LEN - 1);
            g_practice_thumb_strings[slot-1][MAX_STRING_LEN-1] = '\0';
        }
        return;
    }

    /* Mesh files */
    for (int i = 0; i < (int)MESH_COUNT; i++) {
        if (nc_stricmp(key, MESH_ENTRIES[i].key) == 0) {
            nc_strncpy(g_mesh_strings[i], value, MAX_STRING_LEN - 1);
            g_mesh_strings[i][MAX_STRING_LEN-1] = '\0';
            return;
        }
    }

    /* Object files */
    for (int i = 0; i < (int)OBJ_COUNT; i++) {
        if (nc_stricmp(key, OBJ_ENTRIES[i].key) == 0) {
            nc_strncpy(g_obj_strings[i], value, MAX_STRING_LEN - 1);
            g_obj_strings[i][MAX_STRING_LEN-1] = '\0';
            return;
        }
    }
}

static void loadConfig(void) {
    if (!g_pathReady) return;

    HANDLE h = CreateFileA(g_configPath, GENERIC_READ, FILE_SHARE_READ, NULL,
                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return;

    char buf[32768];
    DWORD bytesRead = 0;
    ReadFile(h, buf, sizeof(buf) - 1, &bytesRead, NULL);
    CloseHandle(h);
    if (bytesRead == 0) return;
    buf[bytesRead] = '\0';

    /* Skip BOM */
    char* p = buf;
    if ((unsigned char)p[0] == 0xEF && (unsigned char)p[1] == 0xBB && (unsigned char)p[2] == 0xBF) {
        p += 3;
    }

    /* Parse line by line */
    while (*p) {
        char line[MAX_LINE_LEN];
        int linePos = 0;

        /* Read one line */
        while (*p && *p != '\n' && linePos < (int)sizeof(line) - 1) {
            line[linePos++] = *p++;
        }
        line[linePos] = '\0';
        if (*p == '\n') p++;

        /* Skip empty lines and comments */
        char* trimmed = line;
        while (*trimmed == ' ' || *trimmed == '\t') trimmed++;
        if (*trimmed == '\0' || *trimmed == '#') continue;

        /* Find '=' */
        char* eq = line;
        while (*eq && *eq != '=') eq++;
        if (*eq != '=') continue;
        *eq = '\0';
        char* key = line;
        char* value = eq + 1;

        parseKeyValue(key, value);
    }

    g_configLoaded = true;
}

/* ── Initialize string storage with defaults ─────────────────────────── */

static void initDefaults(void) {
    for (int i = 0; i < 15; i++) {
        nc_strncpy(g_race_strings[i], DEFAULT_RACE_PATHS[i], MAX_STRING_LEN - 1);
        g_race_strings[i][MAX_STRING_LEN-1] = '\0';
        nc_strncpy(g_arena_strings[i], DEFAULT_ARENA_PATHS[i], MAX_STRING_LEN - 1);
        g_arena_strings[i][MAX_STRING_LEN-1] = '\0';
    }

    static const char* tourney_defaults[15] = {
        "tourney-beginner.png", "tourney-cascade.png",
        "tourney-intermediate.png", "tourney-dizzy.png",
        "tourney-Tower.png", "tourney-Up.png",
        "tourney-Neon.png", "tourney-Expert.png",
        "tourney-Odd.png", "tourney-Toob.png",
        "tourney-Wobbly.png", "tourney-Glass.png",
        "tourney-Sky.png", "tourney-Master.png",
        "tourney-Impossible.png"
    };
    static const char* practice_defaults[15] = {
        "Textures\\practice-level1.png",
        "Textures\\practice-cascade.png",
        "Textures\\practice-level2.png",
        "Textures\\practice-level3.png",
        "Textures\\practice-level4.png",
        "Textures\\practice-up.png",
        "Textures\\practice-level5.png",
        "Textures\\practice-level6.png",
        "Textures\\practice-level7.png",
        "Textures\\practice-level8.png",
        "Textures\\practice-level9.png",
        "Textures\\practice-level10.png",
        "Textures\\practice-neon.png",
        "Textures\\practice-glass.png",
        "Textures\\practice-impossible.png"
    };
    for (int i = 0; i < 15; i++) {
        nc_strncpy(g_tourney_thumb_strings[i], tourney_defaults[i], MAX_STRING_LEN - 1);
        g_tourney_thumb_strings[i][MAX_STRING_LEN-1] = '\0';
        nc_strncpy(g_practice_thumb_strings[i], practice_defaults[i], MAX_STRING_LEN - 1);
        g_practice_thumb_strings[i][MAX_STRING_LEN-1] = '\0';
    }

    for (int i = 0; i < (int)MESH_COUNT; i++) {
        nc_strncpy(g_mesh_strings[i], MESH_ENTRIES[i].default_path, MAX_STRING_LEN - 1);
        g_mesh_strings[i][MAX_STRING_LEN-1] = '\0';
    }

    for (int i = 0; i < (int)OBJ_COUNT; i++) {
        nc_strncpy(g_obj_strings[i], OBJ_ENTRIES[i].default_path, MAX_STRING_LEN - 1);
        g_obj_strings[i][MAX_STRING_LEN-1] = '\0';
    }
}

/* ── Patch PUSH instruction to point to new string ──────────────────── */

static void patchPush(DWORD pushRva, const char* newString) {
    HMODULE hExe = GetModuleHandleA("Hamsterball.exe");
    if (!hExe) hExe = GetModuleHandleA(NULL);
    DWORD base = (DWORD)hExe;
    DWORD pushAddr = base + pushRva;

    /* Verify it's a PUSH imm32 (0x68) */
    if (*((BYTE*)pushAddr) != 0x68) return;

    DWORD* immPtr = (DWORD*)(pushAddr + 1);
    DWORD oldProt;
    if (VirtualProtect(immPtr, 4, PAGE_READWRITE, &oldProt)) {
        *immPtr = (DWORD)newString;
        VirtualProtect(immPtr, 4, oldProt, &oldProt);
    }
}

/* ── Patch cache system: prevent reading and writing .cached files ───── */

static void patchCacheSystem(DWORD base) {
    if (!g_ignoreCache) return;

    /*
     * MeshWorld_ctor (0x0046f3d0):
     *   At 0x0046f439 there's a JZ (74 70) that skips cache reading
     *   when the .cached file doesn't exist. We patch it to JMP (EB 70)
     *   so it ALWAYS skips reading the cache file.
     *
     *   Address: base + 0x6f439
     *   Original: 74 70  (JZ +0x70)
     *   Patched:  EB 70  (JMP +0x70)
     */
    DWORD skipCacheRead = base + 0x6f439;
    DWORD oldProt;
    if (VirtualProtect((void*)skipCacheRead, 1, PAGE_READWRITE, &oldProt)) {
        *((BYTE*)skipCacheRead) = 0xEB;
        VirtualProtect((void*)skipCacheRead, 1, oldProt, &oldProt);
    }

    /*
     * Mesh_SaveAndFree (0x0046f670):
     *   At 0x0046f67e there's a JE (0f 84 cf 00 00 00) that skips
     *   the cache write block if filename is NULL. We patch it to
     *   always skip writing cache files.
     *
     *   Address: base + 0x6f67e
     *   Original: 0f 84 cf 00 00 00  (JE +0xcf)
     *   Patched:  e9 d0 00 00 00 90  (JMP +0xd0, NOP)
     *   This is a 6-byte conditional jump → 5-byte JMP + 1 NOP
     */
    DWORD skipCacheWrite = base + 0x6f67e;
    if (VirtualProtect((void*)skipCacheWrite, 6, PAGE_READWRITE, &oldProt)) {
        *((BYTE*)skipCacheWrite) = 0xE9;
        *((DWORD*)(skipCacheWrite + 1)) = 0x000000d0;
        *((BYTE*)(skipCacheWrite + 5)) = 0x90;
        VirtualProtect((void*)skipCacheWrite, 6, oldProt, &oldProt);
    }

    /*
     * Also patch the graphics flag check that gates cache writing.
     * At 0x0046f684 there's another conditional (0f 85 c6 00 00 00)
     * that checks cache_flag==0. Patch to always jump (skip write).
     *
     *   Address: base + 0x6f684
     *   Original: 0f 85 c6 00 00 00  (JNZ +0xc6)
     *   Patched:  e9 c7 00 00 00 90  (JMP +0xc7, NOP)
     */
    DWORD skipCacheFlag = base + 0x6f684;
    if (VirtualProtect((void*)skipCacheFlag, 6, PAGE_READWRITE, &oldProt)) {
        *((BYTE*)skipCacheFlag) = 0xE9;
        *((DWORD*)(skipCacheFlag + 1)) = 0x000000c7;
        *((BYTE*)(skipCacheFlag + 5)) = 0x90;
        VirtualProtect((void*)skipCacheFlag, 6, oldProt, &oldProt);
    }
}

/* ── Apply all patches ───────────────────────────────────────────────── */

static void applyAllPatches(void) {
    HMODULE hExe = GetModuleHandleA("Hamsterball.exe");
    if (!hExe) hExe = GetModuleHandleA(NULL);
    DWORD base = (DWORD)hExe;

    /* Patch cache system first */
    patchCacheSystem(base);

    /* Patch level MESHWORLD paths */
    for (int i = 0; i < 15; i++) {
        patchPush(RACE_PUSH_RVAS[i], g_race_strings[i]);
        patchPush(ARENA_PUSH_RVAS[i], g_arena_strings[i]);
    }

    /* Patch tournament thumbnails */
    for (int i = 0; i < 15; i++) {
        patchPush(TOURNEY_THUMB_RVAS[i], g_tourney_thumb_strings[i]);
    }

    /* Patch practice thumbnails */
    for (int i = 0; i < 15; i++) {
        patchPush(PRACTICE_THUMB_RVAS[i], g_practice_thumb_strings[i]);
    }

    /* Patch mesh files */
    for (int i = 0; i < (int)MESH_COUNT; i++) {
        patchPush(MESH_ENTRIES[i].push_rva, g_mesh_strings[i]);
    }

    /* Patch object files */
    for (int i = 0; i < (int)OBJ_COUNT; i++) {
        patchPush(OBJ_ENTRIES[i].push_rva, g_obj_strings[i]);
    }
}

/* ── HB+ vtable (17 entries for v2.0/v2.1) ──────────────────────────── */

static void* __thiscall sc_dtor(void* thisptr, int flags) {
    if (flags & 1) nc_free(thisptr);
    return thisptr;
}

static const char* __thiscall get_mod_name(void*) { return "Xtreme Organization"; }
static const char* __thiscall get_author(void*) { return "MAKYUNI"; }
static int __thiscall get_version(void*) { return HAMSTERBALL_API_VERSION; }
static const char* __thiscall get_contributors(void*) { return "Hamsterbot"; }

static void __thiscall init_impl(void* thisptr, void* modApi) {
    *(void**)((char*)thisptr + 4) = modApi;
    g_storedApi = modApi;

    /* Initialize defaults */
    initDefaults();

    /* Build config path and load config */
    buildConfigPath();
    createDefaultConfig();
    loadConfig();

    /* Apply all patches */
    applyAllPatches();

    /* Create toggle button for cache */
    CustomButton btn("xtreme_org_ignore_cache", "Ignore Cache Files");
    btn.defaultState = true;
    btn.submenuID = "XTREME_ORG";
    HBPlusAPI hb = { modApi };
    hb.CreateToggleButton(btn, thisptr);

    /* Create submenu */
    CustomSubmenu sub("XTREME_ORG", "Xtreme Organization");
    hb.CreateSubmenu(sub);
}

static void __thiscall ball_update_impl(void* thisptr, void* ball) {
    /* No per-frame logic needed — patches are applied once at init */
}

static void __thiscall game_update_impl(void*) {
    /* Could re-apply patches here if needed, but for now patches are persistent */
}

static void __thiscall button_toggle_impl(void* thisptr, const char* id, bool state) {
    if (nc_stricmp(id, "xtreme_org_ignore_cache") == 0) {
        g_ignoreCache = state;
        /* Re-apply cache patches */
        HMODULE hExe = GetModuleHandleA("Hamsterball.exe");
        if (!hExe) hExe = GetModuleHandleA(NULL);
        if (hExe) {
            DWORD base = (DWORD)hExe;
            DWORD oldProt;

            if (state) {
                /* Enable cache skipping */
                DWORD skipCacheRead = base + 0x6f439;
                if (VirtualProtect((void*)skipCacheRead, 1, PAGE_READWRITE, &oldProt)) {
                    *((BYTE*)skipCacheRead) = 0xEB;
                    VirtualProtect((void*)skipCacheRead, 1, oldProt, &oldProt);
                }
            } else {
                /* Disable cache skipping (restore original) */
                DWORD skipCacheRead = base + 0x6f439;
                if (VirtualProtect((void*)skipCacheRead, 1, PAGE_READWRITE, &oldProt)) {
                    *((BYTE*)skipCacheRead) = 0x74;
                    VirtualProtect((void*)skipCacheRead, 1, oldProt, &oldProt);
                }
            }
        }
    }
}

/* No-op implementations for unused callbacks */
static void __thiscall render_apply_impl(void*, void*, float*) {}
static void __thiscall slider_change_impl(void*, const char*, float) {}
static void __thiscall cycle_change_impl(void*, const char*, const char*) {}
static void __thiscall event_collide_impl(void*, void*, char*) {}
static void __thiscall text_render_impl(void*) {}
static void __thiscall ball_bump_impl(void*, void*, void*) {}
static void __thiscall level_start_impl(void*) {}
static void __thiscall scene_end_impl(void*) {}

static void* g_vtable[17] = {
    (void*)sc_dtor,              // [0]
    (void*)get_mod_name,         // [1]
    (void*)get_author,           // [2]
    (void*)get_version,          // [3]
    (void*)get_contributors,     // [4]
    (void*)init_impl,            // [5]
    (void*)ball_update_impl,     // [6]
    (void*)render_apply_impl,    // [7]
    (void*)button_toggle_impl,   // [8]
    (void*)slider_change_impl,   // [9]
    (void*)cycle_change_impl,    // [10]
    (void*)game_update_impl,     // [11]
    (void*)event_collide_impl,   // [12]
    (void*)text_render_impl,     // [13]
    (void*)ball_bump_impl,       // [14]
    (void*)scene_end_impl,       // [15]
    (void*)level_start_impl,     // [16]
};

extern "C" __declspec(dllexport) HamsterballAPI* CreateModInstance() {
    void* obj = nc_malloc(8);
    if (!obj) return NULL;
    *(void**)obj = g_vtable;
    *(void**)((char*)obj + 4) = NULL;
    return (HamsterballAPI*)obj;
}
