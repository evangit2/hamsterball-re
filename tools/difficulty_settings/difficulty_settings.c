/*
 * difficulty_settings.c — Difficulty-based entity replacement mod
 *
 * Reads difficulty_settings.txt next to bass.dll. Maps entity names to
 * replacements depending on tournament difficulty (Pipsqueak/Normal/Frenzied!).
 *
 * Config format:
 *
 *   EASY          ; Pipsqueak
 *   8ball = NOTHING
 *   Mousetrap = NOTHING
 *
 *   NORMAL        ; Normal
 *   8ball = 8ball
 *   Mousetrap = Mousetrap
 *
 *   HARD          ; Frenzied!
 *   8ball = Bonk
 *   Mousetrap = Mousetrap
 *
 * "NOTHING" = skip spawning the entity entirely.
 * Entity names are case-insensitive. "8ball" is an alias for "BadBall".
 *
 * HOW IT WORKS:
 *   Hooks the Scene_CreateEntities dispatch function (0x0041C5B0) which
 *   runs before ALL entity factories (CreateBadBall, CreateMouseTrap,
 *   CreateLevelObjects, CreateExpertLevelObjects). Before the original
 *   runs, we iterate the MeshWorld entity list and replace entity name
 *   pointers with replacement names from the config. This ensures ALL
 *   factories see the modified names.
 *
 *   For "NOTHING": entity name is replaced with "REF:NOTHING" which no
 *   factory matches, so the entity is silently skipped.
 *   For replacements (e.g. "8ball" → "BONK"): entity name is replaced
 *   with "BONK". CreateBadBall won't match it, but CreateLevelObjects
 *   will create a Bonk object at the original 8ball's position.
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll difficulty_settings.c \
 *     -I../shared -lwinmm -Wl,--enable-stdcall-fixup -O2 -static \
 *     -static-libgcc -Wl,--add-stdcall-alias -msse2 -mfpmath=sse
 */

#include "bass_proxy.h"

/* ═══════════════════════════════════════════════════════════════════════════
 * Game addresses
 * ═══════════════════════════════════════════════════════════════════════════ */

#define APP_DIFFICULTY_OFFSET 0x23C

/* Scene_CreateEntities — dispatch function that calls ALL entity factories.
 * __fastcall, ECX=board/scene, plain RET.
 * Calls: CreateBadBall, CreateMouseTrap, CreateSecretObjects,
 *        Scene_CreateFlags, Scene_CreateSigns, Scene_CreateDynamicObjects
 * (which calls CreateLevelObjects / CreateExpertLevelObjects via vtable). */
#define SCENE_CREATE_ENTITIES_ADDR 0x0041C5B0

/* Board/Scene layout */
#define BOARD_MESHWORLD_OFFSET  0x8AC   /* board+0x8AC → MeshWorld ptr */

/* MeshWorld entity list */
#define MW_ENTITYLIST_OFFSET 0x480      /* MW+0x480 → entity_list struct */

/* Entity list struct layout */
#define ELIST_COUNT  0x898   /* entity_list+0x898 → count (int) */
#define ELIST_DATA   0xCA0   /* entity_list+0xCA0 → *(ptr) → array of entity ptrs */

/* Entity struct: [0]=name(char*), [1]=x, [2]=y, [3]=z, ... */
#define ENTITY_NAME_IDX 0

/* ═══════════════════════════════════════════════════════════════════════════
 * Config structures
 * ═══════════════════════════════════════════════════════════════════════════ */

#define MAX_REPLACEMENTS 64
#define MAX_NAME_LEN 64

typedef struct {
    char original[MAX_NAME_LEN];
    char replacement[MAX_NAME_LEN];  /* "NOTHING" = skip spawn */
} Replacement;

typedef struct {
    Replacement entries[MAX_REPLACEMENTS];
    int count;
} DifficultyTable;

static DifficultyTable g_tables[3];  /* 0=EASY, 1=NORMAL, 2=HARD */
static char g_configPath[MAX_PATH];
static CRITICAL_SECTION g_configLock;

/* ═══════════════════════════════════════════════════════════════════════════
 * String helpers
 * ═══════════════════════════════════════════════════════════════════════════ */

static int ci_strcmp(const char *a, const char *b) {
    while (*a && *b) {
        int ca = (*a >= 'a' && *a <= 'z') ? *a - 32 : *a;
        int cb = (*b >= 'a' && *b <= 'z') ? *b - 32 : *b;
        if (ca != cb) return ca - cb;
        a++; b++;
    }
    return *a - *b;
}

static void normalize_name(char *name) {
    /* Convert to uppercase */
    for (char *p = name; *p; p++) {
        if (*p >= 'a' && *p <= 'z') *p -= 32;
    }
    /* Map user-friendly aliases to internal game names */
    if (strcmp(name, "8BALL") == 0) {
        strcpy(name, "BADBALL");
    }
}

static char* trim(char *s) {
    while (*s == ' ' || *s == '\t' || *s == '\r' || *s == '\n') s++;
    if (*s == '\0') return s;
    char *end = s + strlen(s) - 1;
    while (end > s && (*end == ' ' || *end == '\t' || *end == '\r' || *end == '\n'))
        *end-- = '\0';
    return s;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Config file parsing
 * ═══════════════════════════════════════════════════════════════════════════ */

static void parse_config(const char *path) {
    HANDLE hFile = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL,
                               OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return;

    DWORD fileSize = GetFileSize(hFile, NULL);
    if (fileSize == 0 || fileSize > 65536) { CloseHandle(hFile); return; }

    char *buf = (char*)malloc(fileSize + 1);
    if (!buf) { CloseHandle(hFile); return; }

    DWORD read = 0;
    ReadFile(hFile, buf, fileSize, &read, NULL);
    buf[read] = '\0';
    CloseHandle(hFile);

    for (int i = 0; i < 3; i++) g_tables[i].count = 0;

    int currentTable = -1;
    char *line = buf;

    while (line && *line) {
        char *next = line;
        while (*next && *next != '\n') next++;
        if (*next) { *next = '\0'; next++; }

        char *trimmed = trim(line);
        if (*trimmed == '\0' || *trimmed == ';' || *trimmed == '#') {
            line = next; continue;
        }

        if (ci_strcmp(trimmed, "EASY") == 0)   { currentTable = 0; line = next; continue; }
        if (ci_strcmp(trimmed, "NORMAL") == 0)  { currentTable = 1; line = next; continue; }
        if (ci_strcmp(trimmed, "HARD") == 0)    { currentTable = 2; line = next; continue; }

        char *eq = strchr(trimmed, '=');
        if (eq && currentTable >= 0 && currentTable < 3) {
            *eq = '\0';
            char *orig = trim(trimmed);
            char *repl = trim(eq + 1);

            DifficultyTable *tbl = &g_tables[currentTable];
            if (tbl->count < MAX_REPLACEMENTS &&
                strlen(orig) < MAX_NAME_LEN && strlen(repl) < MAX_NAME_LEN) {
                strncpy(tbl->entries[tbl->count].original, orig, MAX_NAME_LEN - 1);
                tbl->entries[tbl->count].original[MAX_NAME_LEN - 1] = '\0';
                strncpy(tbl->entries[tbl->count].replacement, repl, MAX_NAME_LEN - 1);
                tbl->entries[tbl->count].replacement[MAX_NAME_LEN - 1] = '\0';
                normalize_name(tbl->entries[tbl->count].original);
                normalize_name(tbl->entries[tbl->count].replacement);
                tbl->count++;
            }
        }
        line = next;
    }
    free(buf);
}

static void generate_default_config(const char *path) {
    HANDLE hFile = CreateFileA(path, GENERIC_WRITE, 0, NULL,
                               CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return;

    const char *defaults =
        "; difficulty_settings.txt\n"
        "; Map entity spawns to replacements based on difficulty.\n"
        "; NOTHING = skip spawning entirely.\n"
        "; Entity names are case-insensitive. \"8ball\" = BadBall.\n"
        ";\n"
        "; Difficulty: EASY=Pipsqueak, NORMAL=Normal, HARD=Frenzied!\n"
        ";\n"
        "; Entities (CreateLevelObjects): Bridge, Tipper, Bonk, Bbridge1,\n"
        ";   Bbridge2, PopCylinder, Blockdawg1, Blockdawg2, Catapult, Gluebie\n"
        "; Entities (CreateExpertLevelObjects): Bonk, Fan, Sawblade, Bridge,\n"
        ";   Judge, Bell\n"
        "; Entities (iterating): 8ball (BadBall), Mousetrap\n\n"
        "EASY\n"
        "8ball = 8ball\n"
        "Mousetrap = Mousetrap\n\n"
        "NORMAL\n"
        "8ball = 8ball\n"
        "Mousetrap = Mousetrap\n\n"
        "HARD\n"
        "8ball = 8ball\n"
        "Mousetrap = Mousetrap\n";

    DWORD written;
    WriteFile(hFile, defaults, strlen(defaults), &written, NULL);
    CloseHandle(hFile);
}

static void init_config_path(void) {
    HMODULE hSelf = NULL;
    GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                      | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                      (LPCSTR)&init_config_path, &hSelf);
    GetModuleFileNameA(hSelf, g_configPath, MAX_PATH);
    char *p = strrchr(g_configPath, '\\');
    if (p) strcpy(p + 1, "difficulty_settings.txt");
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Replacement lookup
 * ═══════════════════════════════════════════════════════════════════════════ */

static int get_difficulty(void) {
    if (IsBadReadPtr((void*)GLOBAL_APP_PTR, 4)) return 1;
    DWORD app = *(DWORD*)GLOBAL_APP_PTR;
    if (!app || app < 0x10000) return 1;
    if (IsBadReadPtr((void*)(app + APP_DIFFICULTY_OFFSET), 4)) return 1;
    int diff = *(int*)(app + APP_DIFFICULTY_OFFSET);
    if (diff < 0 || diff > 2) return 1;
    return diff;
}

/*
 * Look up entity name in current difficulty table.
 * Returns: -1=NOTHING(skip), 1=replacement(name in outBuf), 0=no match
 */
static int lookup_replacement(const char *entityName, char *outBuf, int bufSize) {
    char normalized[MAX_NAME_LEN];
    strncpy(normalized, entityName, MAX_NAME_LEN - 1);
    normalized[MAX_NAME_LEN - 1] = '\0';
    normalize_name(normalized);

    int diff = get_difficulty();
    DifficultyTable *tbl = &g_tables[diff];

    for (int i = 0; i < tbl->count; i++) {
        if (ci_strcmp(normalized, tbl->entries[i].original) == 0) {
            if (ci_strcmp(tbl->entries[i].replacement, "NOTHING") == 0) {
                return -1;
            }
            strncpy(outBuf, tbl->entries[i].replacement, bufSize - 1);
            outBuf[bufSize - 1] = '\0';
            return 1;
        }
    }
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Entity list modification
 *
 * Iterates the MeshWorld entity list and modifies entity name pointers
 * based on the config. This ensures ALL factories see the modified names.
 *
 * Entity list access (verified from CreateBadBall decompilation at 0x40BCA0):
 *   board+0x8AC → MeshWorld
 *   MeshWorld+0x480 → entity_list struct
 *   entity_list+0x898 → count (int)
 *   *(entity_list+0xCA0) → data_array (DWORD* array of entity pointers)
 *   entity[0] → name (char*)
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Stable replacement name buffers (entity[0] will point to these) */
#define MAX_NAME_SLOTS 256
static char g_nameSlots[MAX_NAME_SLOTS][MAX_NAME_LEN];
static int g_nameSlotCount = 0;

/* Track which board we've already modified (avoids re-modifying) */
static DWORD g_lastBoard = 0;

static void modify_entity_names(DWORD board) {
    if (!board || board == g_lastBoard) return;
    g_lastBoard = board;
    g_nameSlotCount = 0;  /* reset for new board */

    /* board → MeshWorld */
    if (IsBadReadPtr((void*)(board + BOARD_MESHWORLD_OFFSET), 4)) return;
    DWORD meshWorld = *(DWORD*)(board + BOARD_MESHWORLD_OFFSET);
    if (!meshWorld || meshWorld < 0x10000) return;

    /* MeshWorld → entity_list */
    if (IsBadReadPtr((void*)(meshWorld + MW_ENTITYLIST_OFFSET), 4)) return;
    DWORD elist = *(DWORD*)(meshWorld + MW_ENTITYLIST_OFFSET);
    if (!elist || elist < 0x10000) return;

    /* entity_list → count */
    if (IsBadReadPtr((void*)(elist + ELIST_COUNT), 4)) return;
    int count = *(int*)(elist + ELIST_COUNT);
    if (count <= 0 || count > 10000) return;

    /* entity_list → data array pointer */
    if (IsBadReadPtr((void*)(elist + ELIST_DATA), 4)) return;
    DWORD dataArr = *(DWORD*)(elist + ELIST_DATA);
    if (!dataArr || dataArr < 0x10000) return;
    if (IsBadReadPtr((void*)dataArr, count * 4)) return;

    /* Iterate entities */
    for (int i = 0; i < count; i++) {
        DWORD entityPtr = *(DWORD*)(dataArr + i * 4);
        if (!entityPtr || entityPtr < 0x10000) continue;
        if (IsBadReadPtr((void*)entityPtr, 4)) continue;

        /* Read entity name pointer (entity[0]) */
        DWORD namePtr = *(DWORD*)entityPtr;
        if (!namePtr || namePtr < 0x10000) continue;
        if (IsBadReadPtr((void*)namePtr, 1)) continue;

        char *name = (char*)namePtr;

        /* Look up replacement for this entity */
        char replacement[MAX_NAME_LEN];
        int result = lookup_replacement(name, replacement, MAX_NAME_LEN);

        if (result != 0 && g_nameSlotCount < MAX_NAME_SLOTS) {
            /* Replace entity name pointer with our stable buffer */
            if (result == -1) {
                /* NOTHING — use a name no factory will match */
                strcpy(g_nameSlots[g_nameSlotCount], "REF:NOTHING");
            } else {
                /* Replacement entity name */
                strcpy(g_nameSlots[g_nameSlotCount], replacement);
            }
            *(DWORD*)entityPtr = (DWORD)g_nameSlots[g_nameSlotCount];
            g_nameSlotCount++;
        }
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Detour hook
 *
 * Target: Scene_CreateEntities (0x0041C5B0)
 * Calling convention: __fastcall, ECX=board, plain RET
 * Prologue: PUSH -1 (2 bytes) + PUSH handler_addr (5 bytes) = 7 bytes
 *
 * We copy 7 bytes to trampoline, patch 5-byte JMP + 2 NOPs.
 * The trampoline executes the original 7 bytes then jumps to target+7.
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef void (__fastcall *SceneCreateEntities_t)(void *board);
static SceneCreateEntities_t g_origFunc = NULL;

/* C wrapper — modifies entity names before calling original */
void __fastcall hook_SceneCreateEntities(void *board) {
    /* Modify entity names in the MeshWorld before any factory runs.
     * This reads the ORIGINAL difficulty (App+0x23C) to select the
     * correct replacement table. Must happen before we override it. */
    EnterCriticalSection(&g_configLock);
    modify_entity_names((DWORD)board);
    LeaveCriticalSection(&g_configLock);

    /* ── Difficulty override ──────────────────────────────────────
     * The game's entity factories (CreateBadBall, CreateMouseTrap,
     * CreateLevelObjects, CreateExpertLevelObjects) all check
     * App+0x23C != 0 before spawning entities. On Pipsqueak (0),
     * factories are skipped entirely — so the mod's name replacements
     * have no effect because the factories never run.
     *
     * Fix: temporarily set difficulty to 1 (Normal) so ALL factories
     * execute. The mod's name replacement logic then controls which
     * entities actually spawn (NOTHING = skip, replacement = different
     * entity, unchanged = normal spawn). Restore the original value
     * after entity creation completes.
     * ──────────────────────────────────────────────────────────── */
    DWORD app = 0;
    int origDifficulty = 1;
    BOOL overridden = FALSE;

    if (!IsBadReadPtr((void*)GLOBAL_APP_PTR, 4)) {
        app = *(DWORD*)GLOBAL_APP_PTR;
        if (app && app >= 0x10000 &&
            !IsBadReadPtr((void*)(app + APP_DIFFICULTY_OFFSET), 4)) {
            origDifficulty = *(int*)(app + APP_DIFFICULTY_OFFSET);
            if (origDifficulty == 0) {
                /* Force Normal so factory difficulty gates pass */
                *(int*)(app + APP_DIFFICULTY_OFFSET) = 1;
                overridden = TRUE;
            }
        }
    }

    /* Call original function via trampoline */
    g_origFunc(board);

    /* Restore original difficulty */
    if (overridden && app) {
        *(int*)(app + APP_DIFFICULTY_OFFSET) = origDifficulty;
    }
}

static void install_hook(void) {
    DWORD target = SCENE_CREATE_ENTITIES_ADDR;
    unsigned char *src = (unsigned char*)target;

    /* Allocate executable page for trampoline */
    void *trampMem = VirtualAlloc(NULL, 4096, MEM_COMMIT | MEM_RESERVE,
                                  PAGE_EXECUTE_READWRITE);
    if (!trampMem) return;
    unsigned char *tramp = (unsigned char*)trampMem;

    /* All Hamsterball functions start with:
     *   6A FF           PUSH -1           (2 bytes)
     *   68 xx xx xx xx  PUSH handler      (5 bytes)
     * Total: 7 bytes for 2 complete instructions.
     *
     * Copy 7 bytes to trampoline, then append JMP back to target+7.
     */
    DWORD old;
    VirtualProtect(src, 16, PAGE_EXECUTE_READWRITE, &old);
    memcpy(tramp, src, 7);               /* copy original 7 bytes */
    tramp[7] = 0xE9;                      /* JMP rel32 */
    DWORD back = (target + 7) - ((DWORD)(tramp + 7) + 5);
    memcpy(tramp + 8, &back, 4);
    VirtualProtect(src, 16, old, &old);
    FlushInstructionCache(GetCurrentProcess(), src, 16);

    /* Set up function pointer to trampoline */
    g_origFunc = (SceneCreateEntities_t)tramp;

    /* Install 5-byte JMP + 2 NOPs at target */
    VirtualProtect(src, 7, PAGE_EXECUTE_READWRITE, &old);
    src[0] = 0xE9;                        /* JMP rel32 */
    DWORD hookAddr = (DWORD)hook_SceneCreateEntities;
    DWORD offset = hookAddr - (target + 5);
    memcpy(src + 1, &offset, 4);
    src[5] = 0x90;                        /* NOP */
    src[6] = 0x90;                        /* NOP */
    VirtualProtect(src, 7, old, &old);
    FlushInstructionCache(GetCurrentProcess(), src, 7);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Config reload thread
 * ═══════════════════════════════════════════════════════════════════════════ */

static DWORD WINAPI config_thread(LPVOID param) {
    while (1) {
        EnterCriticalSection(&g_configLock);
        parse_config(g_configPath);
        LeaveCriticalSection(&g_configLock);
        Sleep(2000);
    }
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DllMain
 * ═══════════════════════════════════════════════════════════════════════════ */

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        InitializeCriticalSection(&g_configLock);

        load_real_bass();
        init_config_path();

        /* Generate default config if it doesn't exist */
        if (GetFileAttributesA(g_configPath) == INVALID_FILE_ATTRIBUTES) {
            generate_default_config(g_configPath);
        }

        EnterCriticalSection(&g_configLock);
        parse_config(g_configPath);
        LeaveCriticalSection(&g_configLock);

        CreateThread(NULL, 0, config_thread, NULL, 0, NULL);
        install_hook();
    }
    return TRUE;
}
