/*
 * mknp_custom_race_descriptions.c — BASS.dll proxy mod
 *
 * Allows customizing the 15 tournament race description texts shown on the
 * Tourney Menu screen. The original game stores these as a hardcoded pointer
 * table at 0x4F7148 (15 char* entries, one per race). This mod overwrites
 * those pointers at runtime to point to strings loaded from a .txt file.
 *
 * Config file: mknp_custom_race_descriptions.txt (next to bass.dll)
 *
 * The file format is:
 *
 *   [Level 1]
 *   Take your time on the Warm-Up race! This easy little...
 *   (multi-line descriptions are supported — everything between
 *    [Level N] and [Level N+1] is the description text)
 *
 *   [Level 2]
 *   Now let's try something a little more interesting...
 *
 *   ...through [Level 15]
 *
 * Lines starting with # or ; are comments. If a level is missing from
 * the file, the original game description is kept. If the file is missing
 * entirely, all original descriptions are used (no-op mod).
 *
 * The game calls Font_WordWrap with the description string, which word-wraps
 * to a 375-pixel width (0x177). Long descriptions will wrap automatically.
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll mknp_custom_race_descriptions.c -lwinmm \
 *     -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
 *     -Wl,--add-stdcall-alias
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

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
 * Custom Race Descriptions Mod
 * ═══════════════════════════════════════════════════════════════════════════ */

/*
 * Tournament description pointer table:
 *   Address 0x4F7148 in Hamsterball.exe (.data section, writable)
 *   15 entries, each a char* (4 bytes) pointing to the description string
 *
 * TourneyMenu_ctor at 0x44FDA0 does:
 *   race_idx = PlayerProfile->race_idx;     // [profile+0x08]
 *   desc_ptr = PTR_TABLE[race_idx];          // [0x4F7148 + race_idx*4]
 *   Font_WordWrap(font, desc_ptr, 0x177, dest_buffer);
 *
 * So we just overwrite the pointers in the table. No code patching needed.
 */

#define DESC_TABLE_ADDR  0x004F7148
#define NUM_RACES         15

/* Original description pointers (from the binary, verified via Ghidra) */
static const DWORD g_original_desc_ptrs[NUM_RACES] = {
    0x004D8238,  /*  0: Warm-Up    — "TAKE YOUR TIME ON THE WARM-UP RACE!..." */
    0x004D8128,  /*  1: Beginner   — "NOW LET'S TRY SOMETHING A LITTLE MORE..." */
    0x004D7FD8,  /*  2: Intermediate — "OKAY, YOU'RE READY FOR THE INTERMEDIATE..." */
    0x004D7E78,  /*  3: Dizzy      — "ROUND AND ROUND SHE GOES..." */
    0x004D7D20,  /*  4: Tower      — "IT'S NOT A MEDIEVAL TORTURE DEVICE..." */
    0x004D7B88,  /*  5: Up         — "UP, UP AND AWAY! AND NOW FOR SOMETHING..." */
    0x004D7A60,  /*  6: Neon       — "IT'S THE BIG BLACKOUT!..." */
    0x004D78D8,  /*  7: Expert     — "OKAY, YOU'VE PASSED THE KIDDIE RACES..." */
    0x004D7718,  /*  8: Odd        — "IT'S CALLED THE 'ODD RACE'..." */
    0x004D7570,  /*  9: Toob       — "TOOBS AND HAMSTERS GO TOGETHER..." */
    0x004D7408,  /* 10: Wobbly     — "CREAK TO THE LEFT..." */
    0x004D7290,  /* 11: Glass      — "FROM THE DERANGED MIND..." */
    0x004D7108,  /* 12: Sky        — "UP HERE IN THE SKY..." */
    0x004D6FB8,  /* 13: Master     — "ARE YOU READY FOR THE MASTER'S CHALLENGE?..." */
    0x004D6E60,  /* 14: Impossible — "THE IMPOSSIBLE RACE..." */
};

/* Race names for the default config file */
static const char *g_race_names[NUM_RACES] = {
    "Warm-Up Race",
    "Beginner Race",
    "Intermediate Race",
    "Dizzy Race",
    "Tower Race",
    "Up Race",
    "Neon Race",
    "Expert Race",
    "Odd Race",
    "Toob Race",
    "Wobbly Race",
    "Glass Race",
    "Sky Race",
    "Master Race",
    "Impossible Race",
};

/* Storage for custom description strings (heap-allocated) */
static char *g_custom_descs[NUM_RACES] = {0};
static int  g_desc_loaded[NUM_RACES] = {0};

/* Path to config file */
static char g_config_path[MAX_PATH] = {0};

/* ── Helpers ─────────────────────────────────────────────────────────────── */

static void get_config_path(void)
{
    HMODULE hSelf = NULL;
    GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                      | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                      (LPCSTR)&get_config_path, &hSelf);
    GetModuleFileNameA(hSelf, g_config_path, MAX_PATH);
    char *p = strrchr(g_config_path, '\\');
    if (p) {
        strcpy(p + 1, "mknp_custom_race_descriptions.txt");
    } else {
        strcpy(g_config_path, "mknp_custom_race_descriptions.txt");
    }
}

/* Read entire file into a malloc'd buffer. Returns NULL on failure. */
static char *read_file_to_buffer(const char *path)
{
    FILE *f = NULL;
    if (fopen_s(&f, path, "rb") != 0 || !f) return NULL;
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (size <= 0 || size > 1024 * 1024) { /* 1MB max */
        fclose(f);
        return NULL;
    }
    char *buf = (char *)malloc(size + 1);
    if (!buf) {
        fclose(f);
        return NULL;
    }
    size_t rd = fread(buf, 1, size, f);
    fclose(f);
    buf[rd] = '\0';
    return buf;
}

/* ── Config file parser ──────────────────────────────────────────────────── */

/*
 * Format:
 *   [Level 1]
 *   Description text on one or more lines.
 *   Continuation lines are joined with spaces.
 *
 *   [Level 2]
 *   Next description...
 *
 *   # Comments and ; comments are skipped
 *   ; Blank lines between levels are ignored
 */
static void parse_config_file(const char *buf)
{
    const char *p = buf;
    int current_level = -1;  /* 0-14 = active level, -1 = none */
    /* We build each description in a temp buffer */
    char *temp_descs[NUM_RACES] = {0};
    int   temp_lens[NUM_RACES] = {0};
    int   temp_caps[NUM_RACES] = {0};

    /* Allocate initial buffers for each level */
    for (int i = 0; i < NUM_RACES; i++) {
        temp_caps[i] = 512;
        temp_descs[i] = (char *)malloc(temp_caps[i]);
        if (temp_descs[i]) {
            temp_descs[i][0] = '\0';
            temp_lens[i] = 0;
        }
    }

    /* Process line by line */
    while (*p) {
        /* Skip leading whitespace */
        const char *line_start = p;
        while (*p == ' ' || *p == '\t') p++;
        const char *content = p;

        /* Find end of line */
        while (*p && *p != '\r' && *p != '\n') p++;
        int line_len = (int)(p - content);

        /* Skip line ending */
        if (*p == '\r') p++;
        if (*p == '\n') p++;

        /* Skip empty lines and comments */
        if (line_len == 0) continue;
        if (content[0] == '#' || content[0] == ';') continue;

        /* Check for [Level N] header */
        if (content[0] == '[') {
            /* Find closing bracket */
            const char *close = content;
            while (close < content + line_len && *close != ']') close++;
            if (close < content + line_len) {
                /* Extract "Level N" between brackets */
                int hdr_len = (int)(close - content - 1);
                const char *hdr = content + 1;
                /* Find "Level " prefix */
                if (hdr_len >= 6 && _strnicmp(hdr, "Level ", 6) == 0) {
                    int level_num = atoi(hdr + 6);
                    if (level_num >= 1 && level_num <= NUM_RACES) {
                        current_level = level_num - 1;  /* 0-indexed */
                        /* Reset the temp buffer for this level */
                        if (temp_descs[current_level]) {
                            temp_descs[current_level][0] = '\0';
                            temp_lens[current_level] = 0;
                        }
                    } else {
                        current_level = -1;
                    }
                } else {
                    /* Unknown header, skip until next */
                    current_level = -1;
                }
            }
            continue;
        }

        /* Accumulate description text for current_level */
        if (current_level >= 0 && current_level < NUM_RACES) {
            /* Add a space if we already have text (join continuation lines) */
            int need_space = (temp_lens[current_level] > 0) ? 1 : 0;
            int total = temp_lens[current_level] + need_space + line_len + 1;

            /* Grow buffer if needed */
            if (total > temp_caps[current_level]) {
                while (temp_caps[current_level] < total)
                    temp_caps[current_level] *= 2;
                char *newbuf = (char *)realloc(temp_descs[current_level],
                                               temp_caps[current_level]);
                if (newbuf) {
                    temp_descs[current_level] = newbuf;
                } else {
                    continue;  /* can't grow, skip this line */
                }
            }

            /* Append space if needed */
            if (need_space) {
                temp_descs[current_level][temp_lens[current_level]++] = ' ';
            }

            /* Append the line content (trimmed of trailing whitespace) */
            int actual_len = line_len;
            while (actual_len > 0 &&
                   (content[actual_len - 1] == ' ' ||
                    content[actual_len - 1] == '\t')) {
                actual_len--;
            }
            if (actual_len > 0) {
                memcpy(temp_descs[current_level] + temp_lens[current_level],
                       content, actual_len);
                temp_lens[current_level] += actual_len;
            }
            temp_descs[current_level][temp_lens[current_level]] = '\0';
        }
    }

    /* Transfer successfully parsed descriptions */
    for (int i = 0; i < NUM_RACES; i++) {
        if (temp_descs[i] && temp_lens[i] > 0) {
            g_custom_descs[i] = temp_descs[i];
            g_desc_loaded[i] = 1;
        } else {
            /* Not in config file — keep original */
            if (temp_descs[i]) free(temp_descs[i]);
            g_desc_loaded[i] = 0;
        }
    }
}

/* ── Generate default config file ────────────────────────────────────────── */

/* Write the original descriptions into a template config file */
static void generate_default_config(void)
{
    FILE *f = NULL;
    if (fopen_s(&f, g_config_path, "w") != 0 || !f) return;

    fprintf(f, "# Race Descriptions Configuration File\n");
    fprintf(f, "# Edit the text below to customize the descriptions shown on the\n");
    fprintf(f, "# Tournament Menu screen for each race.\n");
    fprintf(f, "#\n");
    fprintf(f, "# Format:\n");
    fprintf(f, "#   [Level N]\n");
    fprintf(f, "#   Your description text here.\n");
    fprintf(f, "#   Multi-line descriptions are joined with spaces.\n");
    fprintf(f, "#\n");
    fprintf(f, "# Lines starting with # or ; are comments.\n");
    fprintf(f, "# Missing levels will use the original game description.\n");
    fprintf(f, "\n");

    for (int i = 0; i < NUM_RACES; i++) {
        /* Read original description string from the game's memory */
        const char *desc = (const char *)g_original_desc_ptrs[i];
        fprintf(f, "[Level %d]  # %s\n", i + 1, g_race_names[i]);
        fprintf(f, "%s\n", desc);
        fprintf(f, "\n");
    }

    fclose(f);
}

/* ── Apply descriptions to game's pointer table ─────────────────────────── */

static void apply_descriptions(void)
{
    DWORD *desc_table = (DWORD *)DESC_TABLE_ADDR;
    DWORD old_protect = 0;

    /* Make the table writable (it's in .data, usually already writable) */
    if (!VirtualProtect(desc_table, NUM_RACES * 4,
                        PAGE_READWRITE, &old_protect)) {
        return;
    }

    for (int i = 0; i < NUM_RACES; i++) {
        if (g_desc_loaded[i] && g_custom_descs[i]) {
            desc_table[i] = (DWORD)g_custom_descs[i];
        }
        /* Otherwise: leave the original pointer untouched */
    }

    VirtualProtect(desc_table, NUM_RACES * 4, old_protect, &old_protect);
}

/* ── Mod initialization ──────────────────────────────────────────────────── */

static void init_mod(void)
{
    get_config_path();

    /* Try to read the config file */
    char *buf = read_file_to_buffer(g_config_path);
    if (buf) {
        parse_config_file(buf);
        free(buf);
    } else {
        /* File doesn't exist — generate a default template */
        generate_default_config();
    }

    /* Apply the descriptions to the game's pointer table */
    apply_descriptions();
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DLL Entry Point
 * ═══════════════════════════════════════════════════════════════════════════ */

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
    switch (fdwReason) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hinstDLL);
        load_real_bass();
        init_mod();
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
