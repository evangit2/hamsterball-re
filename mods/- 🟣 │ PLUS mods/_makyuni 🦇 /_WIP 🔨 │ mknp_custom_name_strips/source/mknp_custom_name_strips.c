/*
 * mknp_custom_name_strips.c — BASS.dll proxy mod
 *
 * Custom per-level RGBA colors for the horizontal strip behind the
 * Race/Arena name shown at the beginning of each race.
 *
 * In the original game, these values are global (same yellow strip
 * for all races/arenas):
 *   R=1.0, G=1.0, B=0.0, A=0.75
 *   Pushed at 0x41B763-0x41B76B in FUN_0041B710 (Scene_DrawNameStrip)
 *
 * This mod replaces the hardcoded push sequence with a code cave
 * containing patchable push-imm32 instructions. A background thread
 * reads the level name from board+0x29B4 and patches the cave's
 * float operands with per-level custom values from a config file.
 *
 * Config file: mknp_custom_name_strips.txt (next to bass.dll)
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll mknp_custom_name_strips.c -lwinmm \
 *     -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
 *     -Wl,--add-stdcall-alias
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string.h>
#include <stdlib.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * BASS Proxy Exports
 * ═══════════════════════════════════════════════════════════════════════════ */

static HMODULE g_hRealBass = NULL;

typedef int  (__stdcall *BASS_ChannelSetAttributes_t)(DWORD, float, int, int);
static BASS_ChannelSetAttributes_t real_BASS_ChannelSetAttributes = NULL;
__declspec(dllexport) int __stdcall BASS_ChannelSetAttributes(DWORD a, float b, int c, int d) {
    if (real_BASS_ChannelSetAttributes) return real_BASS_ChannelSetAttributes(a, b, c, d);
    return 1;
}
typedef int  (__stdcall *BASS_MusicPlayEx_t)(DWORD, DWORD, BOOL);
static BASS_MusicPlayEx_t real_BASS_MusicPlayEx = NULL;
__declspec(dllexport) int __stdcall BASS_MusicPlayEx(DWORD a, DWORD b, BOOL c) {
    if (real_BASS_MusicPlayEx) return real_BASS_MusicPlayEx(a, b, c);
    return 1;
}
typedef int  (__stdcall *BASS_SetConfig_t)(DWORD, DWORD);
static BASS_SetConfig_t real_BASS_SetConfig = NULL;
__declspec(dllexport) int __stdcall BASS_SetConfig(DWORD a, DWORD b) {
    if (real_BASS_SetConfig) return real_BASS_SetConfig(a, b);
    return 1;
}
typedef int  (__stdcall *BASS_Init_t)(int, DWORD, DWORD, HWND, void*);
static BASS_Init_t real_BASS_Init = NULL;
__declspec(dllexport) int __stdcall BASS_Init(int a, DWORD b, DWORD c, HWND d, void* e) {
    if (real_BASS_Init) return real_BASS_Init(a, b, c, d, e);
    return 1;
}
typedef int  (__stdcall *BASS_Free_t)(void);
static BASS_Free_t real_BASS_Free = NULL;
__declspec(dllexport) int __stdcall BASS_Free(void) {
    if (real_BASS_Free) return real_BASS_Free();
    return 1;
}
typedef int  (__stdcall *BASS_Start_t)(void);
static BASS_Start_t real_BASS_Start = NULL;
__declspec(dllexport) int __stdcall BASS_Start(void) {
    if (real_BASS_Start) return real_BASS_Start();
    return 1;
}
typedef int  (__stdcall *BASS_Stop_t)(void);
static BASS_Stop_t real_BASS_Stop = NULL;
__declspec(dllexport) int __stdcall BASS_Stop(void) {
    if (real_BASS_Stop) return real_BASS_Stop();
    return 1;
}
typedef int  (__stdcall *BASS_ErrorGetCode_t)(void);
static BASS_ErrorGetCode_t real_BASS_ErrorGetCode = NULL;
__declspec(dllexport) int __stdcall BASS_ErrorGetCode(void) {
    if (real_BASS_ErrorGetCode) return real_BASS_ErrorGetCode();
    return 0;
}
typedef DWORD (__stdcall *BASS_MusicLoad_t)(int, void*, DWORD, DWORD, DWORD, DWORD);
static BASS_MusicLoad_t real_BASS_MusicLoad = NULL;
__declspec(dllexport) DWORD __stdcall BASS_MusicLoad(int a, void* b, DWORD c, DWORD d, DWORD e, DWORD f) {
    if (real_BASS_MusicLoad) return real_BASS_MusicLoad(a, b, c, d, e, f);
    return 0;
}
typedef int  (__stdcall *BASS_ChannelStop_t)(DWORD);
static BASS_ChannelStop_t real_BASS_ChannelStop = NULL;
__declspec(dllexport) int __stdcall BASS_ChannelStop(DWORD a) {
    if (real_BASS_ChannelStop) return real_BASS_ChannelStop(a);
    return 1;
}

/* Extra stubs */
__declspec(dllexport) void __stdcall BASS_Pause(void) {}
__declspec(dllexport) void __stdcall BASS_SetVolume(DWORD a) {}
__declspec(dllexport) DWORD __stdcall BASS_GetVolume(void) { return 0; }
__declspec(dllexport) int __stdcall BASS_GetDevice(void) { return 0; }
__declspec(dllexport) int __stdcall BASS_SetDevice(DWORD a) { return 1; }
__declspec(dllexport) void __stdcall BASS_GetInfo(void *a) {}
__declspec(dllexport) int __stdcall BASS_Update(DWORD a) { return 0; }
__declspec(dllexport) DWORD __stdcall BASS_StreamCreateFile(void *a, void *b, DWORD c, DWORD d, DWORD e) { return 0; }
__declspec(dllexport) DWORD __stdcall BASS_SampleLoad(int a, void *b, DWORD c, DWORD d, DWORD e) { return 0; }
__declspec(dllexport) int __stdcall BASS_ChannelPlay(DWORD a, BOOL b) { return 1; }
__declspec(dllexport) int __stdcall BASS_ChannelSetAttribute(DWORD a, DWORD b, float c) { return 1; }
__declspec(dllexport) int __stdcall BASS_ChannelGetAttribute(DWORD a, DWORD b, float *c) { return 1; }
__declspec(dllexport) DWORD __stdcall BASS_ChannelGetData(DWORD a, void *b, DWORD c) { return 0; }
__declspec(dllexport) DWORD __stdcall BASS_ChannelGetLevel(DWORD a) { return 0; }
__declspec(dllexport) int __stdcall BASS_ChannelSetPosition(DWORD a, void *b, DWORD c) { return 1; }
__declspec(dllexport) DWORD __stdcall BASS_ChannelGetPosition(DWORD a, DWORD b) { return 0; }
__declspec(dllexport) int __stdcall BASS_ChannelIsActive(DWORD a) { return 0; }
__declspec(dllexport) int __stdcall BASS_ChannelRemoveSync(DWORD a, DWORD b) { return 1; }
__declspec(dllexport) DWORD __stdcall BASS_ChannelSetSync(DWORD a, DWORD b, DWORD c, void *d, void *e) { return 0; }
__declspec(dllexport) DWORD __stdcall BASS_SampleCreate(DWORD a, DWORD b, DWORD c, DWORD d, DWORD e) { return 0; }
__declspec(dllexport) DWORD __stdcall BASS_SampleGetChannel(DWORD a, BOOL b) { return 0; }

static void load_real_bass(void)
{
    g_hRealBass = LoadLibraryA("bass_real.dll");
    if (g_hRealBass == NULL) {
        char path[MAX_PATH];
        HMODULE hSelf = NULL;
        GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                          | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                          (LPCSTR)&load_real_bass, &hSelf);
        GetModuleFileNameA(hSelf, path, MAX_PATH);
        char *p = strrchr(path, '\\');
        if (p) {
            strcpy(p + 1, "bass_real.dll");
            g_hRealBass = LoadLibraryA(path);
        }
    }
    if (g_hRealBass) {
        #define LOAD(name) real_##name = (name##_t)GetProcAddress(g_hRealBass, #name)
        LOAD(BASS_ChannelSetAttributes);
        LOAD(BASS_MusicPlayEx);
        LOAD(BASS_SetConfig);
        LOAD(BASS_Init);
        LOAD(BASS_Free);
        LOAD(BASS_Start);
        LOAD(BASS_Stop);
        LOAD(BASS_ErrorGetCode);
        LOAD(BASS_MusicLoad);
        LOAD(BASS_ChannelStop);
        #undef LOAD
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Custom Name Strips Mod
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Game addresses */
#define APP_PTR_ADDR     0x005341E0
#define BOARD_NAME_OFF   0x29B4   /* board+0x29B4 = char* race/arena name */
#define BALL_VTABLE      0x4CF3A0  /* for board validation */

/* Patch site: the push sequence in FUN_0041B710 */
#define PATCH_ADDR       0x0041B763
#define PATCH_SIZE       14         /* 5+2+1+1+5 bytes */
#define RETURN_ADDR      0x0041B771 /* instruction after the call */
#define MATRIX_SCALE     0x00453150 /* Matrix_Scale4x4 target */

/* Original bytes at patch site (verified from EXE) */
static const BYTE g_original_bytes[14] = {
    0x68, 0x00, 0x00, 0x40, 0x3F,  /* push 0x3F400000 (A=0.75) */
    0x6A, 0x00,                     /* push 0 (B=0.0) */
    0x55,                           /* push ebp (G=1.0) */
    0x55,                           /* push ebp (R=1.0) */
    0xE8, 0xDF, 0x79, 0x03, 0x00   /* call Matrix_Scale4x4 */
};

/* Default RGBA (original game values) */
#define DEFAULT_R  1.0f
#define DEFAULT_G  1.0f
#define DEFAULT_B  0.0f
#define DEFAULT_A  0.75f

/* Level definitions: 15 races + 15 arenas = 30 entries */
#define NUM_LEVELS 15
#define NUM_ENTRIES 30  /* 15 races + 15 arenas */

/* Level keywords for name matching (priority order: WARM before UP) */
static const char *g_level_keywords[NUM_LEVELS] = {
    "WARM",        /* 0: Warm-Up */
    "BEGINNER",    /* 1: Beginner */
    "INTERMED",    /* 2: Intermediate */
    "DIZZY",       /* 3: Dizzy */
    "TOWER",       /* 4: Tower */
    "UP",          /* 5: Up (checked after WARM) */
    "NEON",        /* 6: Neon */
    "EXPERT",      /* 7: Expert */
    "ODD",         /* 8: Odd */
    "TOOB",        /* 9: Toob */
    "WOBBLY",      /* 10: Wobbly */
    "GLASS",       /* 11: Glass */
    "SKY",         /* 12: Sky */
    "MASTER",      /* 13: Master */
    "IMPOSSIBLE"   /* 14: Impossible */
};

/* Level display names for config file generation */
static const char *g_level_names[NUM_LEVELS] = {
    "Warm-Up", "Beginner", "Intermediate", "Dizzy", "Tower",
    "Up", "Neon", "Expert", "Odd", "Toob",
    "Wobbly", "Glass", "Sky", "Master", "Impossible"
};

/* RGBA storage: [0-14] = races, [15-29] = arenas */
typedef struct {
    float r, g, b, a;
    int   valid;  /* 1 if custom values loaded */
} StripColor;

static StripColor g_colors[NUM_ENTRIES];
static char g_config_path[MAX_PATH] = {0};

/* Code cave state */
static BYTE *g_cave_mem = NULL;
static DWORD g_cave_float_offsets[4];  /* offsets into g_cave_mem for R,G,B,A */
static int g_cave_installed = 0;

/* ── String helpers ─────────────────────────────────────────────────────── */

static char to_lower(char c) { return (c >= 'A' && c <= 'Z') ? (c + 32) : c; }

static int stristr_ci(const char *haystack, const char *needle) {
    if (!haystack || !*haystack || !needle || !*needle) return 0;
    int nl = lstrlenA(needle);
    int hl = lstrlenA(haystack);
    for (int i = 0; i <= hl - nl; i++) {
        int j;
        for (j = 0; j < nl; j++) {
            if (to_lower(haystack[i + j]) != to_lower(needle[j])) break;
        }
        if (j == nl) return 1;
    }
    return 0;
}

/* Match level name → returns 0-14 or -1 */
static int match_level(const char *name) {
    if (!name || !*name) return -1;
    for (int i = 0; i < NUM_LEVELS; i++) {
        if (stristr_ci(name, g_level_keywords[i])) return i;
    }
    return -1;
}

/* Determine if name is an arena */
static int is_arena_name(const char *name) {
    return stristr_ci(name, "ARENA");
}

/* ── Config file parsing ────────────────────────────────────────────────── */

/* Parse a float from a string. Returns 1 on success. */
static int parse_float_val(const char *s, float *out) {
    while (*s == ' ' || *s == '\t') s++;
    if (*s == '\0' || *s == '\r' || *s == '\n') return 0;
    char *end;
    float val = strtof(s, &end);
    if (end == s) return 0;  /* no digits parsed */
    *out = val;
    return 1;
}

/* Find "X = <float>" in a line and parse the float. X is 'R', 'G', 'B', or 'A'. */
static int extract_float(const char *line, char key, float *out) {
    /* Search for "key =" or "key=" pattern (case-insensitive) */
    const char *p = line;
    while (*p) {
        if (to_lower(*p) == to_lower(key)) {
            /* Check if followed by optional space + '=' */
            const char *q = p + 1;
            while (*q == ' ' || *q == '\t') q++;
            if (*q == '=') {
                q++;
                return parse_float_val(q, out);
            }
        }
        p++;
    }
    return 0;
}

/* Parse the config file */
static void read_config(void) {
    HANDLE h = CreateFileA(g_config_path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                           NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return;

    char buf[16384] = {0};
    DWORD bytesRead = 0;
    ReadFile(h, buf, sizeof(buf) - 1, &bytesRead, NULL);
    CloseHandle(h);
    if (bytesRead == 0) return;

    /* Clear previous entries */
    for (int i = 0; i < NUM_ENTRIES; i++) g_colors[i].valid = 0;

    /* Parse line by line */
    char *p = buf;
    while (*p) {
        /* Extract current line */
        char *nl = p;
        while (*nl && *nl != '\n') nl++;
        char saved = *nl;
        *nl = '\0';

        /* Skip empty/comment lines */
        char *line = p;
        while (*line == ' ' || *line == '\t') line++;
        if (*line == ';' || *line == '#' || *line == '\r' || *line == '\0') {
            *nl = saved;
            if (*nl == '\n') nl++;
            p = nl;
            continue;
        }

        /* Check for "Level N" or "Arena N" header */
        int is_arena = 0;
        int level_num = 0;
        const char *prefix = NULL;

        if (stristr_ci(line, "Arena")) {
            is_arena = 1;
            prefix = "Arena";
        } else if (stristr_ci(line, "Level")) {
            is_arena = 0;
            prefix = "Level";
        }

        if (prefix) {
            /* Find the number after the prefix */
            char *num_start = line;
            /* Find the prefix in the line */
            while (*num_start) {
                if (to_lower(*num_start) == to_lower(prefix[0])) {
                    int match = 1;
                    for (int i = 0; prefix[i]; i++) {
                        if (to_lower(num_start[i]) != to_lower(prefix[i])) {
                            match = 0; break;
                        }
                    }
                    if (match) { num_start += strlen(prefix); break; }
                }
                num_start++;
            }
            /* Skip spaces */
            while (*num_start == ' ' || *num_start == '\t') num_start++;
            /* Parse number */
            level_num = atoi(num_start);
            if (level_num >= 1 && level_num <= 15) {
                /* Look for RGBA values: either on this line or the next */
                float r = 1.0f, g = 1.0f, b = 1.0f, a = 1.0f;
                int found_r = 0, found_g = 0, found_b = 0, found_a = 0;

                /* Try parsing from the rest of this line first */
                found_r = extract_float(line, 'R', &r);
                found_g = extract_float(line, 'G', &g);
                found_b = extract_float(line, 'B', &b);
                found_a = extract_float(line, 'A', &a);

                /* If not all found, check the next line */
                if (!(found_r && found_g && found_b && found_a)) {
                    *nl = saved;
                    if (*nl == '\n') {
                        /* Read next line */
                        char *next = nl + 1;
                        char *next_nl = next;
                        while (*next_nl && *next_nl != '\n') next_nl++;
                        char saved2 = *next_nl;
                        *next_nl = '\0';

                        if (!found_r) found_r = extract_float(next, 'R', &r);
                        if (!found_g) found_g = extract_float(next, 'G', &g);
                        if (!found_b) found_b = extract_float(next, 'B', &b);
                        if (!found_a) found_a = extract_float(next, 'A', &a);

                        *next_nl = saved2;
                    }
                    /* Restore and continue from after current line */
                    *nl = '\0';  /* keep current line terminated for rest of processing */
                }

                if (found_r || found_g || found_b || found_a) {
                    int idx = (is_arena ? NUM_LEVELS : 0) + (level_num - 1);
                    g_colors[idx].r = r;
                    g_colors[idx].g = g;
                    g_colors[idx].b = b;
                    g_colors[idx].a = a;
                    g_colors[idx].valid = 1;
                }
            }
        }

        *nl = saved;
        if (*nl == '\n') nl++;
        p = nl;
    }
}

/* Create default config file */
static void create_default_config(void) {
    HANDLE h = CreateFileA(g_config_path, GENERIC_READ, FILE_SHARE_READ, NULL,
                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h != INVALID_HANDLE_VALUE) { CloseHandle(h); return; }

    /* Build default config */
    char buf[8192];
    int pos = 0;
    pos += wsprintfA(buf + pos,
        "; Custom Name Strips Config File\r\n"
        "; Per-level RGBA colors for the strip behind the Race/Arena name\r\n"
        "; Values are floats (0.0 to 1.0)\r\n"
        "; Original game values: R=1.0, G=1.0, B=0.0, A=0.75 (yellow)\r\n"
        ";\r\n"
        "; Format:\r\n"
        ";   Level N =\r\n"
        ";   R = <float>, G = <float>, B = <float>, A = <float>\r\n"
        ";\r\n"
        "; Levels 1-15 = Races, Arenas 1-15 = Arenas\r\n"
        "; Re-reads every 2 seconds. Edit at runtime!\r\n"
        "\r\n");

    for (int i = 0; i < NUM_LEVELS; i++) {
        pos += wsprintfA(buf + pos, "Level %d =\r\n", i + 1);
        pos += wsprintfA(buf + pos, "R = 1.0, G = 1.0, B = 1.0, A = 1.0\r\n\r\n");
    }
    for (int i = 0; i < NUM_LEVELS; i++) {
        pos += wsprintfA(buf + pos, "Arena %d =\r\n", i + 1);
        pos += wsprintfA(buf + pos, "R = 1.0, G = 1.0, B = 1.0, A = 1.0\r\n\r\n");
    }

    DWORD written;
    h = CreateFileA(g_config_path, GENERIC_WRITE, 0, NULL,
                    CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h != INVALID_HANDLE_VALUE) {
        WriteFile(h, buf, pos, &written, NULL);
        CloseHandle(h);
    }
}

/* ── Code cave installation ────────────────────────────────────────────── */

static int patch_byte(DWORD addr, BYTE value) {
    DWORD oldProtect;
    if (!VirtualProtect((void*)addr, 1, PAGE_EXECUTE_READWRITE, &oldProtect))
        return 0;
    *(BYTE*)addr = value;
    VirtualProtect((void*)addr, 1, oldProtect, &oldProtect);
    return 1;
}

static int patch_dword(DWORD addr, DWORD value) {
    DWORD oldProtect;
    if (!VirtualProtect((void*)addr, 4, PAGE_EXECUTE_READWRITE, &oldProtect))
        return 0;
    *(DWORD*)addr = value;
    VirtualProtect((void*)addr, 4, oldProtect, &oldProtect);
    return 1;
}

/* Write a float into the code cave at the given offset */
static void patch_cave_float(int float_idx, float value) {
    if (!g_cave_mem || !g_cave_installed) return;
    DWORD addr = (DWORD)g_cave_mem + g_cave_float_offsets[float_idx];
    DWORD oldProtect;
    if (VirtualProtect((void*)addr, 4, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        *(float*)addr = value;
        VirtualProtect((void*)addr, 4, oldProtect, &oldProtect);
    }
}

/* Install the code cave at 0x41B763.
 *
 * Original 14 bytes (PUSH A + PUSH 0 + PUSH EBP + PUSH EBP + CALL):
 *   68 00 00 40 3F  6A 00  55  55  E8 DF 79 03 00
 *
 * Replaced with:
 *   E9 <jmp_to_cave>  90 90 90 90 90 90 90 90 90   (5-byte JMP + 9 NOPs)
 *
 * Cave code (30 bytes):
 *   68 <A_imm32>      push A          (patchable, 5 bytes)
 *   68 <B_imm32>      push B          (patchable, 5 bytes)
 *   68 <G_imm32>      push G          (patchable, 5 bytes)
 *   68 <R_imm32>      push R          (patchable, 5 bytes)
 *   E8 <rel_call>     call Matrix_Scale4x4  (5 bytes)
 *   E9 <jmp_back>     jmp 0x41B771    (5 bytes)
 */
static int install_code_cave(void) {
    if (g_cave_installed) return 1;

    /* Verify original bytes */
    BYTE *patch_addr = (BYTE*)PATCH_ADDR;
    if (memcmp(patch_addr, g_original_bytes, PATCH_SIZE) != 0) {
        MessageBoxA(NULL, "Custom Name Strips: original bytes mismatch at 0x41B763",
                    "Mod Error", MB_OK | MB_ICONWARNING);
        return 0;
    }

    /* Allocate executable page for the cave */
    g_cave_mem = (BYTE*)VirtualAlloc(NULL, 4096, MEM_COMMIT | MEM_RESERVE,
                                     PAGE_EXECUTE_READWRITE);
    if (!g_cave_mem) return 0;

    /* Build the cave code */
    int off = 0;

    /* push A (imm32) — 5 bytes */
    g_cave_float_offsets[3] = off + 1;  /* Alpha float at offset+1 */
    g_cave_mem[off] = 0x68;  /* push imm32 */
    *(DWORD*)(g_cave_mem + off + 1) = *(DWORD*)&(float){DEFAULT_A};
    off += 5;

    /* push B (imm32) — 5 bytes */
    g_cave_float_offsets[2] = off + 1;  /* Blue float at offset+1 */
    g_cave_mem[off] = 0x68;
    *(DWORD*)(g_cave_mem + off + 1) = *(DWORD*)&(float){DEFAULT_B};
    off += 5;

    /* push G (imm32) — 5 bytes */
    g_cave_float_offsets[1] = off + 1;  /* Green float at offset+1 */
    g_cave_mem[off] = 0x68;
    *(DWORD*)(g_cave_mem + off + 1) = *(DWORD*)&(float){DEFAULT_G};
    off += 5;

    /* push R (imm32) — 5 bytes */
    g_cave_float_offsets[0] = off + 1;  /* Red float at offset+1 */
    g_cave_mem[off] = 0x68;
    *(DWORD*)(g_cave_mem + off + 1) = *(DWORD*)&(float){DEFAULT_R};
    off += 5;

    /* call Matrix_Scale4x4 — 5 bytes (relative call) */
    DWORD call_addr = (DWORD)g_cave_mem + off;
    DWORD call_target = MATRIX_SCALE;
    DWORD call_offset = call_target - (call_addr + 5);
    g_cave_mem[off] = 0xE8;  /* call rel32 */
    *(DWORD*)(g_cave_mem + off + 1) = call_offset;
    off += 5;

    /* jmp back to RETURN_ADDR (0x41B771) — 5 bytes */
    DWORD jmp_addr = (DWORD)g_cave_mem + off;
    DWORD jmp_offset = RETURN_ADDR - (jmp_addr + 5);
    g_cave_mem[off] = 0xE9;  /* jmp rel32 */
    *(DWORD*)(g_cave_mem + off + 1) = jmp_offset;
    off += 5;

    /* Now patch the original code site: JMP to cave + NOPs */
    DWORD cave_addr = (DWORD)g_cave_mem;
    DWORD jmp_to_cave_offset = cave_addr - (PATCH_ADDR + 5);

    /* Write JMP (E9 + offset) */
    patch_byte(PATCH_ADDR, 0xE9);
    patch_dword(PATCH_ADDR + 1, jmp_to_cave_offset);

    /* Fill remaining 9 bytes with NOPs */
    for (int i = 5; i < PATCH_SIZE; i++) {
        patch_byte(PATCH_ADDR + i, 0x90);
    }

    g_cave_installed = 1;
    return 1;
}

/* ── Background thread ─────────────────────────────────────────────────── */

static volatile int g_thread_running = 1;
static HANDLE g_thread_handle = NULL;
static int g_last_level_idx = -1;

/* Get the current board pointer via the verified pointer chain */
static int *get_board(void) {
    int *app = *(int**)APP_PTR_ADDR;
    if (!app || IsBadReadPtr(app, 4)) return NULL;
    int *profile = *(int**)((int)app + 0x220);
    if (!profile || IsBadReadPtr(profile, 4)) return NULL;
    int *board = *(int**)((int)profile + 0x0C);
    if (!board || IsBadReadPtr(board, 4)) return NULL;
    return board;
}

/* Determine the current level index from the board's race name.
 * Returns table index 0-29, or -1 if unknown. */
static int get_current_level_idx(void) {
    int *board = get_board();
    if (!board) return -1;

    /* Read race name string pointer from board+0x29B4 */
    char *name = *(char**)((int)board + BOARD_NAME_OFF);
    if (!name || IsBadReadPtr(name, 1)) return -1;

    /* Match level keyword */
    int level = match_level(name);
    if (level < 0) return -1;

    /* Determine race vs arena */
    int arena = is_arena_name(name);
    return arena ? (level + NUM_LEVELS) : level;
}

static DWORD WINAPI strip_thread(LPVOID param) {
    DWORD config_tick = 0;

    while (g_thread_running) {
        DWORD tick = GetTickCount();

        /* Re-read config every 2 seconds */
        if (tick - config_tick > 2000) {
            read_config();
            config_tick = tick;
        }

        /* Determine current level and patch cave floats */
        int idx = get_current_level_idx();

        if (idx != g_last_level_idx) {
            g_last_level_idx = idx;
        }

        if (idx >= 0 && g_cave_installed) {
            StripColor *c;
            if (g_colors[idx].valid) {
                c = &g_colors[idx];
            } else {
                /* Use default (original game values) */
                static StripColor def = {DEFAULT_R, DEFAULT_G, DEFAULT_B, DEFAULT_A, 1};
                c = &def;
            }

            /* Patch the 4 float operands in the cave:
             * g_cave_float_offsets[0] = R
             * g_cave_float_offsets[1] = G
             * g_cave_float_offsets[2] = B
             * g_cave_float_offsets[3] = A
             */
            patch_cave_float(0, c->r);
            patch_cave_float(1, c->g);
            patch_cave_float(2, c->b);
            patch_cave_float(3, c->a);
        }

        Sleep(50);  /* poll every 50ms */
    }

    return 0;
}

/* ── Initialization ────────────────────────────────────────────────────── */

static void init_mod(void) {
    /* Get config path (next to bass.dll) */
    HMODULE hSelf = NULL;
    GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                      | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                      (LPCSTR)&init_mod, &hSelf);
    GetModuleFileNameA(hSelf, g_config_path, MAX_PATH);
    char *p = strrchr(g_config_path, '\\');
    if (p) {
        strcpy(p + 1, "mknp_custom_name_strips.txt");
    } else {
        strcpy(g_config_path, "mknp_custom_name_strips.txt");
    }

    /* Create default config if it doesn't exist */
    create_default_config();

    /* Read initial config */
    read_config();

    /* Install code cave */
    if (!install_code_cave()) {
        MessageBoxA(NULL, "Custom Name Strips: failed to install code cave",
                    "Mod Error", MB_OK | MB_ICONERROR);
        return;
    }

    /* Start background thread */
    g_thread_handle = CreateThread(NULL, 0, strip_thread, NULL, 0, NULL);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DLL Entry Point
 * ═══════════════════════════════════════════════════════════════════════════ */

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    switch (fdwReason) {
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls(hinstDLL);
            load_real_bass();
            init_mod();
            break;
        case DLL_PROCESS_DETACH:
            g_thread_running = 0;
            if (g_thread_handle) {
                WaitForSingleObject(g_thread_handle, 1000);
                CloseHandle(g_thread_handle);
            }
            break;
    }
    return TRUE;
}
