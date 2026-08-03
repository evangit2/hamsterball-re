/*
 * mknp_sfx_path_MinGW.cpp — Custom SFX + Per-Level SFX Path (HB+ v2.1, MinGW)
 *
 * Features:
 *   1. Individual sound replacement via custom_sfx.txt config file.
 *      Replace any of 61 sound effects with custom .ogg/.wav files.
 *   2. Per-level SFX_PATH via RaceData.XML <SFX_PATH> tags.
 *      Redirect all sound loading to a subfolder for specific levels.
 *      Example: <GLASSRACE><SFX_PATH>Newsfx</SFX_PATH></GLASSRACE>
 *      → Glass Race loads sounds from Sounds\Newsfx\ instead of Sounds\
 *
 * Uses nocrt + manual 17-entry vtable for HB+ v2.1 compatibility.
 * Hooks Sound_LoadOggOrWav (0x459660) via code cave.
 */
#include "nocrt.h"
#include "HamsterballAPI.h"
#include "hbplus_api.h"

/* ═══════════════════════════════════════════════════════════════════════════
 * Constants
 * ═══════════════════════════════════════════════════════════════════════════ */

#define SOUND_LOAD_OGG_WAV_ADDR  0x459660
#define SOUND_LOAD_OGG_WAV_CONT  0x459666  /* address after patched instruction */
#define SOUNDLIST_DTOR_INNER     0x459700  /* clears DSound buffers in SoundList */
#define SOUNDS_OFFSET            0x43C     /* App+0x43C = first SoundList* */
#define NUM_SOUNDS               61

/* ═══════════════════════════════════════════════════════════════════════════
 * Sound Table — All 61 SFX in Hamsterball
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef struct {
    const char* name;
    int buffers;
    const char* entity;
    char replacement[64];
} sound_entry_t;

static sound_entry_t g_sounds[] = {
    {"collide",          10, "Ball-wall collision, Bumper hit", ""},
    {"roll",             10, "Ball rolling on surfaces", ""},
    {"whistle",          1,  "Race start whistle, menu select", ""},
    {"bumper",           10, "Bumper collision (Warm-Up, Beginner, Arena)", ""},
    {"ballbreak",        5,  "Ball shatter (full break)", ""},
    {"ballbreaksmall",   5,  "Ball partial break", ""},
    {"thwomp",           2,  "Bonkbash slam, heavy impact", ""},
    {"snap",             2,  "BreakBridge snap", ""},
    {"popup",            2,  "Bonk popup, UI popup", ""},
    {"dropin",           2,  "Ball drop-in at race start", ""},
    {"dropinshort",      2,  "Short drop-in", ""},
    {"popout",           2,  "Ball pop out (pipe exit)", ""},
    {"pipebump1",        10, "Pipe bump variant 1", ""},
    {"pipebump2",        10, "Pipe bump variant 2", ""},
    {"pipebump3",        10, "Pipe bump variant 3", ""},
    {"gearclank",        20, "Gear clank (Rotator collision)", ""},
    {"bridgeslam",       2,  "Bridge slam, Catapult launch", ""},
    {"platformtick",     5,  "Platform tick (moving platform)", ""},
    {"gluestuck",        5,  "Gluebie stuck sound", ""},
    {"bubble1",          5,  "Tar bubble 1", ""},
    {"bubble2",          5,  "Tar bubble 2", ""},
    {"wheelcreak",       2,  "Wheel creak (Spinner)", ""},
    {"catapult",         2,  "Catapult wind-up", ""},
    {"trapdoor",         2,  "Trapdoor activate, Rotator start", ""},
    {"fwing",            2,  "E:JUMP event, fan wing flap", ""},
    {"clink",            3,  "Breaker extend, metal clink", ""},
    {"whoosh",           3,  "Whoosh (air movement)", ""},
    {"chomp",            1,  "Chomp (Mousetrap)", ""},
    {"fan-start",        10, "Fan startup", ""},
    {"fan-blow",         10, "Fan blowing (continuous)", ""},
    {"crack",            2,  "Glass crack (Glass Race)", ""},
    {"crumble",          2,  "BreakBridge crumble", ""},
    {"sawstartup",       2,  "Saw startup (alert sound)", ""},
    {"sawcut",           2,  "Saw cutting (contact)", ""},
    {"minipop",          5,  "Mini pop (small collision)", ""},
    {"bell",             3,  "Bell ring (Bonus Bell)", ""},
    {"zip",              2,  "Zip (speed boost)", ""},
    {"ting",             20, "Ting (Rotator gear tick)", ""},
    {"shrink",           3,  "Ball shrink (power-down)", ""},
    {"grow",             3,  "Ball grow (power-up)", ""},
    {"tweet",            3,  "Tweet (bird/secret unlock)", ""},
    {"creakyplatform",   20, "Creaky platform (Tower)", ""},
    {"wubba",            5,  "Wubba (Wobbly Race platform)", ""},
    {"saw",              2,  "Saw idle hum", ""},
    {"sawspeedy",        2,  "Saw speedy (fast cutting)", ""},
    {"dawgstep1",        10, "Blockdawg footstep 1", ""},
    {"dawgstep2",        10, "Blockdawg footstep 2", ""},
    {"dawgsmash",        10, "Blockdawg smash (fall impact)", ""},
    {"sizzle",           2,  "Sizzle (Neon Race)", ""},
    {"explode",          3,  "Explode (Impossible Race)", ""},
    {"vac-o-sux",        3,  "Vac-O-Sux vacuum suction", ""},
    {"speedcylinder",    2,  "Speed Cylinder boost, Pendulum hit", ""},
    {"bonuspop",         5,  "Bonus pop (arena score)", ""},
    {"buzzbonus",        1,  "Buzz bonus (Rotator trigger)", ""},
    {"breakbridge",      1,  "Break bridge collapse", ""},
    {"unlock",           1,  "Level/feature unlock", ""},
    {"NeonRide",         1,  "Neon Ride (Neon Race enter)", ""},
    {"NeonFlicker",      50, "Neon Flicker (Neon Race ambient)", ""},
    {"ZoopDown",         2,  "Zoop Down (Glass Race)", ""},
    {"LightsOff",        2,  "Lights Off (Glass Race)", ""},
    {"GlassBonus",       2,  "Glass Bonus (Glass Race)", ""},
};

/* ═══════════════════════════════════════════════════════════════════════════
 * Level Tag Mapping (scene name → RaceData.XML tag)
 * ═══════════════════════════════════════════════════════════════════════════ */

#define NUM_RACES 15

static const char* RACE_SCENE_NAMES[NUM_RACES] = {
    "Board (Warm-Up)",
    "Board (Beginner)",
    "Board (Intermediate)",
    "Board (Dizzy)",
    "Board (Tower)",
    "Board (Up)",
    "Board (Dark)",
    "Board (Expert)",
    "Board (Odd)",
    "Board (Toob)",
    "Board (Wobbly)",
    "Board (Glass)",
    "Board (Sky)",
    "Board (Master)",
    "Board (Impossible)"
};

static const char* ARENA_SCENE_NAMES[NUM_RACES] = {
    "RumbleBoard (Warmup Arena)",
    "RumbleBoard (Beginner Arena)",
    "RumbleBoard (Intermediate Arena)",
    "RumbleBoard (Dizzy Arena)",
    "RumbleBoard (Tower Arena)",
    "RumbleBoard (Up Arena)",
    "RumbleBoard (Neon Arena)",
    "RumbleBoard (Expert Arena)",
    "RumbleBoard (Odd Arena)",
    "RumbleBoard (Toob Arena)",
    "RumbleBoard (Wobbly Arena)",
    "RumbleBoard (Sky Arena)",
    "RumbleBoard (Master Arena)",
    "RumbleBoard (Glass Arena)",
    "RumbleBoard (Impossible Arena)"
};

static const char* RACE_XML_TAGS[NUM_RACES] = {
    "CASCADERACE",
    "BEGINNERRACE",
    "INTERMEDIATERACE",
    "DIZZYRACE",
    "TOWERRACE",
    "UPRACE",
    "NEONRACE",
    "EXPERTRACE",
    "ODDRACE",
    "TOOBRACE",
    "WOBBLYRACE",
    "GLASSRACE",
    "SKYRACE",
    "MASTERRACE",
    "IMPOSSIBLERACE"
};

static const char* ARENA_XML_TAGS[NUM_RACES] = {
    "WARMUPARENA",
    "BEGINNERARENA",
    "INTERMEDIATEARENA",
    "DIZZYARENA",
    "TOWERARENA",
    "UPARENA",
    "NEONARENA",
    "EXPERTARENA",
    "ODDARENA",
    "TOOBARENA",
    "WOBBLYARENA",
    "SKYARENA",
    "MASTERARENA",
    "GLASSARENA",
    "IMPOSSIBLEARENA"
};

/* ═══════════════════════════════════════════════════════════════════════════
 * Global State
 * ═══════════════════════════════════════════════════════════════════════════ */

static void* g_storedApi = NULL;
static char g_configPath[MAX_PATH] = "";
static bool g_configPathReady = false;

/* Active SFX_PATH subfolder (empty = use default sounds folder) */
static char g_activeSfxPath[128] = "";

/* Parsed SFX_PATH values from RaceData.XML (one per level, empty = none) */
static char g_sfxPaths[NUM_RACES * 2][128]; /* 15 races + 15 arenas */

/* Flag: hook installed */
static bool g_hookInstalled = false;

/* ═══════════════════════════════════════════════════════════════════════════
 * Config File Path (next to DLL in Mods\ folder)
 * ═══════════════════════════════════════════════════════════════════════════ */

static void buildConfigPath(void) {
    MEMORY_BASIC_INFORMATION mbi;
    char dllPath[MAX_PATH];
    if (VirtualQuery((void*)buildConfigPath, &mbi, sizeof(mbi)) > 0) {
        HMODULE hMod = (HMODULE)mbi.AllocationBase;
        if (hMod && GetModuleFileNameA(hMod, dllPath, MAX_PATH) > 0) {
            char* last = NULL;
            char* p = dllPath;
            while (*p) {
                if (*p == '\\' || *p == '/') last = p;
                p++;
            }
            if (last) {
                *(last + 1) = '\0';
                nc_strncpy(g_configPath, dllPath, MAX_PATH - 1);
                nc_strncpy(g_configPath + nc_strlen(g_configPath),
                           "custom_sfx.txt",
                           MAX_PATH - nc_strlen(g_configPath) - 1);
                g_configPath[MAX_PATH - 1] = '\0';
                g_configPathReady = true;
                return;
            }
        }
    }
    nc_strncpy(g_configPath, "custom_sfx.txt", MAX_PATH - 1);
    g_configPath[MAX_PATH - 1] = '\0';
    g_configPathReady = true;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Config File Generator — writes custom_sfx.txt with all 61 sounds
 * ═══════════════════════════════════════════════════════════════════════════ */

static void generateConfig(const char* path) {
    HANDLE h = CreateFileA(path, GENERIC_WRITE, 0, NULL,
                           CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return;

    char buf[8192];
    int pos = 0;

    pos += nc_snprintf(buf + pos, sizeof(buf) - pos,
        "; =======================================================================\r\n"
        "; Custom SFX Configuration - Hamsterball (HB+ Mod)\r\n"
        "; =======================================================================\r\n"
        ";\r\n"
        "; This file lists all 61 sound effects in the game.\r\n"
        "; To replace a sound, change the filename after = to your custom file.\r\n"
        "; Your file must be .ogg or .wav format, placed in the Sounds\\ folder.\r\n"
        ";\r\n"
        "; Example: To replace the collide sound:\r\n"
        ";   1. Place your file as Sounds\\my_collision.ogg\r\n"
        ";   2. Change: collide = my_collision\r\n"
        ";\r\n"
        "; You can also use subfolders: collide = subfolder\\my_sound\r\n"
        ";\r\n"
        "; For per-level SFX_PATH, edit RaceData.XML:\r\n"
        ";   <GLASSRACE><SFX_PATH>Newsfx</SFX_PATH></GLASSRACE>\r\n"
        "; This loads all Glass Race sounds from Sounds\\Newsfx\\\r\n"
        ";\r\n"
        "; =======================================================================\r\n\r\n");

    int i;
    for (i = 0; i < NUM_SOUNDS; i++) {
        pos += nc_snprintf(buf + pos, sizeof(buf) - pos,
            "; --- %s ---\r\n"
            "; Buffers: %d | Entity: %s\r\n"
            "%s = %s\r\n\r\n",
            g_sounds[i].name, g_sounds[i].buffers, g_sounds[i].entity,
            g_sounds[i].name, g_sounds[i].name);
    }

    DWORD written = 0;
    WriteFile(h, buf, pos, &written, NULL);
    CloseHandle(h);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Config Parser — reads custom_sfx.txt for individual sound replacements
 * ═══════════════════════════════════════════════════════════════════════════ */

static void readConfig(void) {
    if (!g_configPathReady) return;

    HANDLE h = CreateFileA(g_configPath, GENERIC_READ, FILE_SHARE_READ, NULL,
                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return;

    char buf[8192];
    DWORD bytesRead = 0;
    ReadFile(h, buf, sizeof(buf) - 1, &bytesRead, NULL);
    CloseHandle(h);
    if (bytesRead == 0) return;
    buf[bytesRead] = '\0';

    char* p = buf;
    /* Skip BOM */
    if ((unsigned char)p[0] == 0xEF && (unsigned char)p[1] == 0xBB && (unsigned char)p[2] == 0xBF) {
        p += 3;
    }

    while (*p) {
        /* Skip whitespace */
        while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') p++;
        if (*p == '\0') break;
        /* Skip comments */
        if (*p == ';' || *p == '#') {
            while (*p && *p != '\n') p++;
            continue;
        }

        /* Parse name = value */
        char nameBuf[64];
        char valBuf[64];
        int ni = 0, vi = 0;
        while (*p && *p != '=' && *p != '\n' && *p != '\r' && ni < 63) {
            if (*p != ' ' && *p != '\t') nameBuf[ni++] = *p;
            p++;
        }
        nameBuf[ni] = '\0';
        if (*p != '=') { while (*p && *p != '\n') p++; continue; }
        p++; /* skip = */
        while (*p == ' ' || *p == '\t') p++;
        while (*p && *p != '\n' && *p != '\r' && vi < 63) {
            valBuf[vi++] = *p;
            p++;
        }
        valBuf[vi] = '\0';
        /* Trim trailing spaces */
        while (vi > 0 && (valBuf[vi-1] == ' ' || valBuf[vi-1] == '\t')) {
            valBuf[--vi] = '\0';
        }
        if (ni == 0 || vi == 0) continue;

        /* Look up in sound table */
        for (int i = 0; i < NUM_SOUNDS; i++) {
            if (nc_strcmp(nameBuf, g_sounds[i].name) == 0) {
                nc_strncpy(g_sounds[i].replacement, valBuf,
                           sizeof(g_sounds[i].replacement) - 1);
                g_sounds[i].replacement[sizeof(g_sounds[i].replacement) - 1] = '\0';
                break;
            }
        }
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * RaceData.XML Parser — reads <SFX_PATH> tags per level
 * ═══════════════════════════════════════════════════════════════════════════ */

static void parseRaceDataXML(void) {
    /* RaceData.XML is in the game root folder */
    char xmlPath[MAX_PATH];
    DWORD gameBase = (DWORD)GetModuleHandleA(NULL);
    nc_snprintf(xmlPath, MAX_PATH, "%s\\RaceData.xml", (char*)gameBase);

    /* Actually, game root is just the exe directory */
    char exePath[MAX_PATH];
    if (GetModuleFileNameA(NULL, exePath, MAX_PATH) > 0) {
        char* last = NULL;
        char* p = exePath;
        while (*p) {
            if (*p == '\\' || *p == '/') last = p;
            p++;
        }
        if (last) {
            *(last + 1) = '\0';
            nc_strncpy(xmlPath, exePath, MAX_PATH - 1);
            nc_strncpy(xmlPath + nc_strlen(xmlPath), "RaceData.xml",
                       MAX_PATH - nc_strlen(xmlPath) - 1);
        } else {
            nc_strncpy(xmlPath, "RaceData.xml", MAX_PATH - 1);
        }
    } else {
        nc_strncpy(xmlPath, "RaceData.xml", MAX_PATH - 1);
    }
    xmlPath[MAX_PATH - 1] = '\0';

    HANDLE h = CreateFileA(xmlPath, GENERIC_READ, FILE_SHARE_READ, NULL,
                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return;

    char buf[16384];
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

    /* Parse XML tags: <TAGNAME>...<SFX_PATH>value</SFX_PATH>...</TAGNAME> */
    int currentLevel = -1; /* -1 = not inside a level tag */
    bool isArena = false;

    while (*p) {
        /* Find next < */
        if (*p != '<') { p++; continue; }
        p++; /* skip < */

        /* Check for closing tag */
        if (*p == '/') {
            p++;
            /* Read tag name */
            char tagName[32];
            int ti = 0;
            while (*p && *p != '>' && *p != ' ' && ti < 31) {
                tagName[ti++] = *p++;
            }
            tagName[ti] = '\0';
            if (*p == '>') p++;

            /* Check if this closes a level tag */
            if (currentLevel >= 0) {
                const char* expectedTag = isArena ? ARENA_XML_TAGS[currentLevel] : RACE_XML_TAGS[currentLevel];
                if (nc_strcmp(tagName, expectedTag) == 0) {
                    currentLevel = -1;
                }
            }
            continue;
        }

        /* Read opening tag name */
        char tagName[32];
        int ti = 0;
        while (*p && *p != '>' && *p != ' ' && *p != '/' && ti < 31) {
            tagName[ti++] = *p++;
        }
        tagName[ti] = '\0';
        if (*p == '>') p++;
        else if (*p == '/') { p++; if (*p == '>') p++; continue; }

        /* Check if this is a level tag */
        if (currentLevel < 0) {
            for (int i = 0; i < NUM_RACES; i++) {
                if (nc_strcmp(tagName, RACE_XML_TAGS[i]) == 0) {
                    currentLevel = i;
                    isArena = false;
                    break;
                }
            }
            if (currentLevel < 0) {
                for (int i = 0; i < NUM_RACES; i++) {
                    if (nc_strcmp(tagName, ARENA_XML_TAGS[i]) == 0) {
                        currentLevel = i;
                        isArena = true;
                        break;
                    }
                }
            }
            continue;
        }

        /* Inside a level tag — check for SFX_PATH */
        if (nc_strcmp(tagName, "SFX_PATH") == 0 && currentLevel >= 0) {
            /* Read text content until </SFX_PATH> */
            char value[128];
            int vi = 0;
            while (*p && *p != '<' && vi < 127) {
                if (*p != '\r' && *p != '\n' && *p != '\t' && *p != ' ') {
                    value[vi++] = *p;
                } else if (*p == ' ' && vi > 0) {
                    value[vi++] = *p;
                }
                p++;
            }
            value[vi] = '\0';
            /* Trim trailing spaces */
            while (vi > 0 && value[vi-1] == ' ') value[--vi] = '\0';

            /* Store in the appropriate slot */
            int slot = isArena ? (NUM_RACES + currentLevel) : currentLevel;
            nc_strncpy(g_sfxPaths[slot], value, 127);
            g_sfxPaths[slot][127] = '\0';

            /* Skip closing </SFX_PATH> */
            if (*p == '<') {
                p++;
                if (*p == '/') p++;
                while (*p && *p != '>') p++;
                if (*p == '>') p++;
            }
            continue;
        }
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Sound Path Modification — called from assembly cave
 *
 * Receives the original name string (e.g. "sounds\collide").
 * Returns a modified path if SFX_PATH is active or individual replacement exists.
 * Returns NULL if no modification needed.
 * ═══════════════════════════════════════════════════════════════════════════ */

static char g_resultBuf[256];

static const char* __cdecl find_sound_replacement(const char* original) {
    if (!original || !*original) return NULL;

    /* Strip "sounds\" or "Sounds\" prefix (case-insensitive) */
    const char* name = original;
    if (nc_strlen(original) > 7) {
        if ((original[0] == 's' || original[0] == 'S') &&
            (original[1] == 'o' || original[1] == 'O') &&
            (original[2] == 'u' || original[2] == 'U') &&
            (original[3] == 'n' || original[3] == 'N') &&
            (original[4] == 'd' || original[4] == 'D') &&
            (original[5] == 's' || original[5] == 'S') &&
            (original[6] == '\\' || original[6] == '/')) {
            name = original + 7;
        }
    }

    /* Check individual sound replacement first (higher priority) */
    for (int i = 0; i < NUM_SOUNDS; i++) {
        if (nc_strcmp(name, g_sounds[i].name) == 0) {
            if (g_sounds[i].replacement[0] != '\0' &&
                nc_strcmp(g_sounds[i].replacement, g_sounds[i].name) != 0) {
                /* Individual replacement takes priority */
                nc_snprintf(g_resultBuf, sizeof(g_resultBuf),
                            "sounds\\%s", g_sounds[i].replacement);
                g_resultBuf[sizeof(g_resultBuf) - 1] = '\0';
                return g_resultBuf;
            }
            break;
        }
    }

    /* Check SFX_PATH — prepend subfolder */
    if (g_activeSfxPath[0] != '\0') {
        /* Build: sounds\<sfx_path>\<name> */
        nc_snprintf(g_resultBuf, sizeof(g_resultBuf),
                    "sounds\\%s\\%s", g_activeSfxPath, name);
        g_resultBuf[sizeof(g_resultBuf) - 1] = '\0';
        return g_resultBuf;
    }

    return NULL; /* no modification */
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Hook Installation — Assembly Cave on Sound_LoadOggOrWav (0x459660)
 *
 * Original entry: 81 EC 00 01 00 00  (SUB ESP, 0x100 — 6 bytes)
 * Patched to:     E9 xx xx xx xx 90  (JMP cave + NOP — 6 bytes)
 *
 * Cave:
 *   PUSH EAX, ECX, EDX        ; save registers
 *   PUSH [ESP+0x10]           ; push name param (first stack param at entry)
 *   CALL find_sound_replacement
 *   ADD ESP, 4                ; clean param
 *   TEST EAX, EAX
 *   JZ skip                   ; no replacement → skip
 *   MOV [ESP+0x10], EAX       ; replace name on stack
 * skip:
 *   POP EDX, ECX, EAX        ; restore registers
 *   SUB ESP, 0x100            ; original instruction
 *   JMP 0x459666              ; continue original function
 * ═══════════════════════════════════════════════════════════════════════════ */

static BYTE* g_cave = NULL;

static void install_hook(void) {
    DWORD target = SOUND_LOAD_OGG_WAV_ADDR;
    DWORD old_prot;
    int pos = 0;

    /* Allocate executable cave (64 bytes) */
    g_cave = (BYTE*)VirtualAlloc(NULL, 64, MEM_COMMIT | MEM_RESERVE,
                                 PAGE_EXECUTE_READWRITE);
    if (!g_cave) return;

    /* PUSH EAX */      g_cave[pos++] = 0x50;
    /* PUSH ECX */      g_cave[pos++] = 0x51;
    /* PUSH EDX */      g_cave[pos++] = 0x52;
    /* PUSH DWORD [ESP+0x10]  (name — 3 pushes + ret_addr = 16 bytes offset) */
    g_cave[pos++] = 0xFF;
    g_cave[pos++] = 0x74;
    g_cave[pos++] = 0x24;
    g_cave[pos++] = 0x10;
    /* CALL find_sound_replacement (rel32) */
    g_cave[pos++] = 0xE8;
    {
        DWORD call_rel = (DWORD)&find_sound_replacement - (DWORD)(g_cave + pos + 4);
        *(DWORD*)(g_cave + pos) = call_rel;
        pos += 4;
    }
    /* ADD ESP, 4 */
    g_cave[pos++] = 0x83;
    g_cave[pos++] = 0xC4;
    g_cave[pos++] = 0x04;
    /* TEST EAX, EAX */
    g_cave[pos++] = 0x85;
    g_cave[pos++] = 0xC0;
    /* JZ +4 (skip MOV) */
    g_cave[pos++] = 0x74;
    g_cave[pos++] = 0x04;
    /* MOV [ESP+0x10], EAX  (replace name) */
    g_cave[pos++] = 0x89;
    g_cave[pos++] = 0x44;
    g_cave[pos++] = 0x24;
    g_cave[pos++] = 0x10;
    /* POP EDX */       g_cave[pos++] = 0x5A;
    /* POP ECX */       g_cave[pos++] = 0x59;
    /* POP EAX */       g_cave[pos++] = 0x58;
    /* SUB ESP, 0x100  (original instruction: 81 EC 00 01 00 00) */
    g_cave[pos++] = 0x81;
    g_cave[pos++] = 0xEC;
    g_cave[pos++] = 0x00;
    g_cave[pos++] = 0x01;
    g_cave[pos++] = 0x00;
    g_cave[pos++] = 0x00;
    /* JMP 0x459666 (rel32) */
    g_cave[pos++] = 0xE9;
    {
        DWORD jmp_rel = (DWORD)SOUND_LOAD_OGG_WAV_CONT - (DWORD)(g_cave + pos + 4);
        *(DWORD*)(g_cave + pos) = jmp_rel;
        pos += 4;
    }

    /* Patch Sound_LoadOggOrWav entry: JMP cave + NOP */
    VirtualProtect((void*)target, 6, PAGE_EXECUTE_READWRITE, &old_prot);
    *(BYTE*)(target)     = 0xE9;  /* JMP rel32 */
    *(DWORD*)(target+1)  = (DWORD)g_cave - (target + 5);
    *(BYTE*)(target+5)   = 0x90;  /* NOP (fill remaining byte) */
    VirtualProtect((void*)target, 6, old_prot, &old_prot);
    FlushInstructionCache(GetCurrentProcess(), (void*)target, 6);

    g_hookInstalled = true;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Sound Reloading — reload all 61 sounds with current g_activeSfxPath
 *
 * Uses inline asm to call:
 *   1. SoundList_DtorInner (0x459700) — __fastcall(ECX = SoundList*)
 *   2. Sound_LoadOggOrWav (0x459660) — __thiscall(ECX=SoundList*, stack=name, stack2=buffers)
 * ═══════════════════════════════════════════════════════════════════════════ */

static void reloadAllSounds(void) {
    if (!g_storedApi) return;

    HBPlusAPI hb = { g_storedApi };
    App* app = hb.GetApp();
    if (!app) return;

    /* App+0x43C = Sounds struct (61 void* pointers to SoundList) */
    DWORD appAddr = (DWORD)app;
    DWORD gameBase = (DWORD)GetModuleHandleA(NULL);

    for (int i = 0; i < NUM_SOUNDS; i++) {
        /* Get SoundList* from App+0x43C + i*4 */
        DWORD soundListAddr = *(DWORD*)(appAddr + SOUNDS_OFFSET + i * 4);
        if (!soundListAddr) continue;

        /* Clear old DSound buffers: call SoundList_DtorInner(soundList) */
        /* __fastcall: ECX = soundList */
        {
            typedef void (__attribute__((fastcall)) *dtor_fn_t)(DWORD);
            dtor_fn_t dtor_fn = (dtor_fn_t)(gameBase + SOUNDLIST_DTOR_INNER);
            dtor_fn(soundListAddr);
        }

        /* Build the sound path */
        char pathBuf[256];
        const char* soundName = g_sounds[i].name;

        if (g_activeSfxPath[0] != '\0') {
            nc_snprintf(pathBuf, sizeof(pathBuf), "sounds\\%s\\%s", g_activeSfxPath, soundName);
        } else {
            nc_snprintf(pathBuf, sizeof(pathBuf), "sounds\\%s", soundName);
        }
        pathBuf[sizeof(pathBuf) - 1] = '\0';

        /* Call Sound_LoadOggOrWav(soundList, name, bufferCount) */
        /* __thiscall: ECX = soundList, stack = [name, bufferCount], RET 8 */
        {
            /* Use a raw function pointer with manual stack management.
               Sound_LoadOggOrWav is __thiscall: ECX=this, 2 stack params, callee cleans (RET 8).
               We declare it as __fastcall(ECX=param1, EDX=param2, stack=param3) with RET 8.
               But GCC __fastcall puts 2 params in ECX/EDX and cleans remaining stack.
               Actually, __thiscall = ECX this + stack params + RET N.
               GCC doesn't have __thiscall, so we use a trick:
               Push stack params, set ECX, CALL, and the callee will RET 8 (clean 2 stack params).
               We use naked inline asm for this. */
            int bufCount = g_sounds[i].buffers;
            DWORD funcAddr = gameBase + SOUND_LOAD_OGG_WAV_ADDR;
            DWORD sl = soundListAddr;
            const char* nm = pathBuf;
            int bc = bufCount;
            __asm__ __volatile__(
                "push %3\n"       /* push bufferCount */
                "push %2\n"       /* push name string */
                "movl %1, %%ecx\n"  /* ECX = soundList* */
                "call *%0\n"      /* call Sound_LoadOggOrWav */
                : /* no output */
                : "r" (funcAddr), "r" (sl), "r" (nm), "r" (bc)
                : "eax", "ecx", "edx", "memory"
            );
        }
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Level Detection
 * ═══════════════════════════════════════════════════════════════════════════ */

static int identifyLevel(void) {
    if (!g_storedApi) return -1;
    HBPlusAPI hb = { g_storedApi };
    Scene* scene = hb.GetScene();
    if (!scene || !scene->name) return -1;
    if (IsBadReadPtr((void*)scene->name, 2)) return -1;
    if ((unsigned char)scene->name[0] < 0x20 || (unsigned char)scene->name[0] > 0x7E) return -1;

    const char* sceneName = scene->name;

    for (int i = 0; i < NUM_RACES; i++) {
        if (nc_strcmp(sceneName, RACE_SCENE_NAMES[i]) == 0) return i;
    }
    for (int i = 0; i < NUM_RACES; i++) {
        if (nc_strcmp(sceneName, ARENA_SCENE_NAMES[i]) == 0) return NUM_RACES + i;
    }
    return -1;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * HB+ Vtable Implementation
 * ═══════════════════════════════════════════════════════════════════════════ */

static void* __thiscall sc_dtor(void* thisptr, int flags) {
    if (flags & 1) nc_free(thisptr);
    return thisptr;
}

static const char* __thiscall get_mod_name(void*) { return "Custom SFX + SFX Path"; }
static const char* __thiscall get_author(void*) { return "MAKYUNI"; }
static int __thiscall get_version(void*) { return HAMSTERBALL_API_VERSION; }
static const char* __thiscall get_contributors(void*) { return "Hamsterbot"; }

static void __thiscall init_impl(void* thisptr, void* modApi) {
    *(void**)((char*)thisptr + 4) = modApi;
    g_storedApi = modApi;

    /* Initialize SFX_PATH storage */
    for (int i = 0; i < NUM_RACES * 2; i++) {
        g_sfxPaths[i][0] = '\0';
    }

    /* Build config path and generate/read custom_sfx.txt */
    buildConfigPath();

    /* Generate config if it doesn't exist */
    if (g_configPathReady) {
        HANDLE h = CreateFileA(g_configPath, GENERIC_READ, FILE_SHARE_READ, NULL,
                               OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (h == INVALID_HANDLE_VALUE) {
            generateConfig(g_configPath);
        } else {
            CloseHandle(h);
        }
    }

    /* Read individual sound replacements */
    readConfig();

    /* Parse RaceData.XML for <SFX_PATH> tags */
    parseRaceDataXML();

    /* Install hook BEFORE game loads sounds */
    install_hook();
}

static void __thiscall level_start_impl(void*) {
    /* Detect current level */
    int levelIdx = identifyLevel();
    if (levelIdx < 0) return;

    /* Check if this level has a SFX_PATH configured */
    const char* sfxPath = g_sfxPaths[levelIdx];
    if (sfxPath[0] != '\0') {
        /* Set active SFX path */
        nc_strncpy(g_activeSfxPath, sfxPath, 127);
        g_activeSfxPath[127] = '\0';
    } else {
        /* Clear active SFX path */
        g_activeSfxPath[0] = '\0';
    }

    /* Reload sounds with new path */
    reloadAllSounds();
}

static void __thiscall scene_end_impl(void*) {
    /* Clear SFX path and reload default sounds */
    if (g_activeSfxPath[0] != '\0') {
        g_activeSfxPath[0] = '\0';
        reloadAllSounds();
    }
}

static void __thiscall game_update_impl(void*) {
    /* Re-read config periodically for live editing (every 120 ticks ~2s) */
    static int tickCounter = 0;
    tickCounter++;
    if (tickCounter >= 120) {
        tickCounter = 0;
        readConfig();
    }
}

/* No-op implementations for unused callbacks */
static void __thiscall ball_update_impl(void*, void*) {}
static void __thiscall render_apply_impl(void*, void*, float*) {}
static void __thiscall button_toggle_impl(void*, const char*, bool) {}
static void __thiscall slider_change_impl(void*, const char*, float) {}
static void __thiscall cycle_change_impl(void*, const char*, const char*) {}
static void __thiscall event_collide_impl(void*, void*, char*) {}
static void __thiscall text_render_impl(void*) {}
static void __thiscall ball_bump_impl(void*, void*, void*) {}

/* ═══════════════════════════════════════════════════════════════════════════
 * 17-Entry Vtable (HB+ v2.0/v2.1)
 * ═══════════════════════════════════════════════════════════════════════════ */

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
