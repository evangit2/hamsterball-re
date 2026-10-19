/*
 * mknp_custom_sfx.c — Custom Sound Effects Replacement Mod for Hamsterball
 *
 * Hooks Sound_LoadOggOrWav (0x459660) to redirect sound file loading.
 * Generates custom_sfx.txt with all 61 sounds, their properties, and
 * which entities/events trigger them. Users can rename any sound to
 * load any .ogg or .wav file from the sounds\ folder.
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll mknp_custom_sfx.c -lwinmm \
 *     -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
 *     -Wl,--add-stdcall-alias -msse2 -mfpmath=sse
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string.h>
#include <stdio.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * BASS Proxy Boilerplate
 * ═══════════════════════════════════════════════════════════════════════════ */

static HMODULE g_hRealBass = NULL;

typedef int  (__stdcall *BASS_ChannelSetAttributes_t)(DWORD, DWORD, int, int);
static BASS_ChannelSetAttributes_t real_BASS_ChannelSetAttributes = NULL;
__declspec(dllexport) int __stdcall BASS_ChannelSetAttributes(DWORD a, DWORD b, int c, int d) {
    if (real_BASS_ChannelSetAttributes) return real_BASS_ChannelSetAttributes(a, b, c, d);
    return 1;
}
typedef int  (__stdcall *BASS_MusicPlayEx_t)(DWORD, DWORD, int, DWORD);
static BASS_MusicPlayEx_t real_BASS_MusicPlayEx = NULL;
__declspec(dllexport) int __stdcall BASS_MusicPlayEx(DWORD a, DWORD b, int c, DWORD d) {
    if (real_BASS_MusicPlayEx) return real_BASS_MusicPlayEx(a, b, c, d);
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

static void load_real_bass(void) {
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
 * Sound Table — All 61 SFX in Hamsterball
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef struct {
    const char* name;         /* sound name (e.g. "collide") */
    int board_offset;          /* offset on Board struct (e.g. 0x43C) */
    int buffers;               /* concurrent playback buffer count */
    const char* entity;        /* entity/event that triggers it */
    char replacement[64];      /* user-specified replacement name */
} sound_entry_t;

static sound_entry_t g_sounds[] = {
    /* --- Collision & Impact Sounds --- */
    {"collide",          0x43C, 10, "Ball-wall collision, Bumper hit", ""},
    {"roll",             0x440, 10, "Ball rolling on surfaces", ""},
    {"whistle",          0x444, 1,  "Race start whistle, menu select", ""},
    {"bumper",           0x448, 10, "Bumper collision (Warm-Up, Beginner, Arena)", ""},
    {"ballbreak",        0x44C, 5,  "Ball shatter (full break)", ""},
    {"ballbreaksmall",   0x450, 5,  "Ball partial break", ""},
    {"thwomp",           0x454, 2,  "Bonkbash slam, heavy impact", ""},
    {"snap",             0x458, 2,  "BreakBridge snap", ""},
    {"popup",            0x45C, 2,  "Bonk popup, UI popup", ""},
    {"dropin",           0x460, 2,  "Ball drop-in at race start", ""},
    {"dropinshort",      0x464, 2,  "Short drop-in", ""},
    {"popout",           0x468, 2,  "Ball pop out (pipe exit)", ""},

    /* --- Pipe & Gear Sounds --- */
    {"pipebump1",        0x46C, 10, "Pipe bump variant 1", ""},
    {"pipebump2",        0x470, 10, "Pipe bump variant 2", ""},
    {"pipebump3",        0x474, 10, "Pipe bump variant 3", ""},
    {"gearclank",        0x478, 20, "Gear clank (Rotator collision)", ""},
    {"bridgeslam",       0x47C, 2,  "Bridge slam, Catapult launch", ""},
    {"platformtick",     0x480, 5,  "Platform tick (moving platform)", ""},
    {"gluestuck",        0x484, 5,  "Gluebie stuck sound", ""},
    {"bubble1",          0x488, 5,  "Tar bubble 1", ""},
    {"bubble2",          0x48C, 5,  "Tar bubble 2", ""},
    {"wheelcreak",       0x490, 2,  "Wheel creak (Spinner)", ""},
    {"catapult",         0x494, 2,  "Catapult wind-up", ""},
    {"trapdoor",         0x498, 2,  "Trapdoor activate, Rotator start", ""},
    {"fwing",            0x49C, 2,  "E:JUMP event, fan wing flap", ""},

    /* --- Mechanism Sounds --- */
    {"clink",            0x4A0, 3,  "Breaker extend, metal clink", ""},
    {"whoosh",           0x4A4, 3,  "Whoosh (air movement)", ""},
    {"chomp",            0x4A8, 1,  "Chomp (Mousetrap)", ""},
    {"fan-start",        0x4AC, 10, "Fan startup", ""},
    {"fan-blow",         0x4B0, 10, "Fan blowing (continuous)", ""},
    {"crack",            0x4B4, 2,  "Glass crack (Glass Race)", ""},
    {"crumble",          0x4B8, 2,  "BreakBridge crumble", ""},
    {"sawstartup",       0x4BC, 2,  "Saw startup (alert sound)", ""},
    {"sawcut",           0x4C0, 2,  "Saw cutting (contact)", ""},
    {"minipop",          0x4C4, 5,  "Mini pop (small collision)", ""},
    {"bell",             0x4C8, 3,  "Bell ring (Bonus Bell)", ""},
    {"zip",              0x4CC, 2,  "Zip (speed boost)", ""},
    {"ting",             0x4D0, 20, "Ting (Rotator gear tick)", ""},
    {"shrink",           0x4D4, 3,  "Ball shrink (power-down)", ""},
    {"grow",             0x4D8, 3,  "Ball grow (power-up)", ""},
    {"tweet",            0x4DC, 3,  "Tweet (bird/secret unlock)", ""},
    {"creakyplatform",   0x4E0, 20, "Creaky platform (Tower)", ""},
    {"wubba",            0x4E4, 5,  "Wubba (Wobbly Race platform)", ""},
    {"saw",              0x4E8, 2,  "Saw idle hum", ""},
    {"sawspeedy",        0x4EC, 2,  "Saw speedy (fast cutting)", ""},

    /* --- Blockdawg Sounds --- */
    {"dawgstep1",        0x4F0, 10, "Blockdawg footstep 1", ""},
    {"dawgstep2",        0x4F4, 10, "Blockdawg footstep 2", ""},
    {"dawgsmash",        0x4F8, 10, "Blockdawg smash (fall impact)", ""},

    /* --- Special & Race-Specific Sounds --- */
    {"sizzle",           0x4FC, 2,  "Sizzle (Neon Race)", ""},
    {"explode",          0x500, 3,  "Explode (Impossible Race)", ""},
    {"vac-o-sux",        0x504, 3,  "Vac-O-Sux vacuum suction", ""},
    {"speedcylinder",    0x508, 2,  "Speed Cylinder boost, Pendulum hit", ""},
    {"bonuspop",         0x50C, 5,  "Bonus pop (arena score)", ""},
    {"buzzbonus",        0x510, 1,  "Buzz bonus (Rotator trigger)", ""},
    {"breakbridge",      0x514, 1,  "Break bridge collapse", ""},
    {"unlock",           0x518, 1,  "Level/feature unlock", ""},
    {"NeonRide",         0x51C, 1,  "Neon Ride (Neon Race enter)", ""},
    {"NeonFlicker",      0x520, 50, "Neon Flicker (Neon Race ambient)", ""},
    {"ZoopDown",         0x524, 2,  "Zoop Down (Glass Race)", ""},
    {"LightsOff",        0x528, 2,  "Lights Off (Glass Race)", ""},
    {"GlassBonus",       0x52C, 2,  "Glass Bonus (Glass Race)", ""},
};
#define NUM_SOUNDS (sizeof(g_sounds) / sizeof(g_sounds[0]))

/* ═══════════════════════════════════════════════════════════════════════════
 * Config File Path
 * ═══════════════════════════════════════════════════════════════════════════ */

static HMODULE g_hSelf = NULL;

static void get_config_path(char* out, DWORD len) {
    char dll_path[MAX_PATH];
    if (GetModuleFileNameA(g_hSelf, dll_path, MAX_PATH)) {
        char* slash = strrchr(dll_path, '\\');
        if (slash) {
            slash[1] = '\0';
            _snprintf(out, len, "%scustom_sfx.txt", dll_path);
            return;
        }
    }
    _snprintf(out, len, "custom_sfx.txt");
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Config File Generator — writes custom_sfx.txt with all 61 sounds
 * ═══════════════════════════════════════════════════════════════════════════ */

static void generate_config(const char* path) {
    FILE* f = NULL;
    if (fopen_s(&f, path, "w") != 0 || !f) return;

    fprintf(f, "; =======================================================================\n");
    fprintf(f, "; Custom SFX Configuration - Hamsterball\n");
    fprintf(f, "; =======================================================================\n");
    fprintf(f, ";\n");
    fprintf(f, "; This file lists all 61 sound effects in the game.\n");
    fprintf(f, "; To replace a sound, change the filename after = to your custom file.\n");
    fprintf(f, "; Your file must be .ogg or .wav format, placed in the sounds\\ folder.\n");
    fprintf(f, ";\n");
    fprintf(f, "; Example: To replace the collide sound:\n");
    fprintf(f, ";   1. Place your file as sounds\\my_collision.ogg (or .wav)\n");
    fprintf(f, ";   2. Change: collide = my_collision\n");
    fprintf(f, ";\n");
    fprintf(f, "; You can also use subfolders: collide = subfolder\\my_sound\n");
    fprintf(f, "; The game will load sounds\\subfolder\\my_sound.ogg\n");
    fprintf(f, ";\n");
    fprintf(f, "; Properties:\n");
    fprintf(f, ";   Buffers  = max concurrent playback instances (higher = more overlapping)\n");
    fprintf(f, ";   Volume   = dynamic, based on 3D distance from ball to sound source\n");
    fprintf(f, ";   Pitch    = dynamic, not per-sound (uses DirectSound frequency)\n");
    fprintf(f, ";   Loop     = all sounds are one-shot (no looping SFX in Hamsterball)\n");
    fprintf(f, ";\n");
    fprintf(f, "; Audio System:\n");
    fprintf(f, ";   SFX: DirectSound (DSOUND.dll) - supports .ogg and .wav\n");
    fprintf(f, ";   Music: BASS.dll - plays .mod/.xm/.it/.mo3 files (not affected by this mod)\n");
    fprintf(f, ";   SoundDevice: App+0x178, master volume at SoundDevice+0x838 (0.0-1.0)\n");
    fprintf(f, ";   3D attenuation: min=0.0, max=6000.0 (SoundDevice+0x914/+0x918)\n");
    fprintf(f, ";\n");
    fprintf(f, "; =======================================================================\n\n");

    int i;
    for (i = 0; i < NUM_SOUNDS; i++) {
        fprintf(f, "; --- %s ---\n", g_sounds[i].name);
        fprintf(f, "; Buffers: %d | Board offset: 0x%X\n",
                g_sounds[i].buffers, g_sounds[i].board_offset);
        fprintf(f, "; Entity/Event: %s\n", g_sounds[i].entity);
        fprintf(f, "; Default file: sounds\\%s.ogg (or .wav)\n", g_sounds[i].name);
        fprintf(f, "%s = %s\n\n", g_sounds[i].name, g_sounds[i].name);
    }

    fclose(f);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Config Parser
 * ═══════════════════════════════════════════════════════════════════════════ */

static void read_config(void) {
    char path[MAX_PATH];
    FILE* f = NULL;
    char line[512];

    get_config_path(path, MAX_PATH);
    if (fopen_s(&f, path, "r") != 0 || !f) return;

    while (fgets(line, sizeof(line), f)) {
        char* p = line;
        char* eq;
        char* name;
        char* value;
        char* end;
        int i;

        /* Skip whitespace */
        while (*p == ' ' || *p == '\t') p++;
        /* Skip comments and empty lines */
        if (*p == ';' || *p == '#' || *p == '\n' || *p == '\r' || *p == '\0')
            continue;

        /* Find = separator */
        eq = strchr(p, '=');
        if (!eq) continue;
        *eq = '\0';

        /* Parse name (trim trailing whitespace) */
        name = p;
        while (*name == ' ' || *name == '\t') name++;
        end = name + strlen(name);
        while (end > name && (end[-1] == ' ' || end[-1] == '\t' ||
               end[-1] == '\n' || end[-1] == '\r'))
            *--end = '\0';

        /* Parse value (trim whitespace) */
        value = eq + 1;
        while (*value == ' ' || *value == '\t') value++;
        end = value + strlen(value);
        while (end > value && (end[-1] == ' ' || end[-1] == '\t' ||
               end[-1] == '\n' || end[-1] == '\r'))
            *--end = '\0';

        if (strlen(name) == 0 || strlen(value) == 0) continue;

        /* Look up in sound table */
        for (i = 0; i < (int)NUM_SOUNDS; i++) {
            if (_stricmp(name, g_sounds[i].name) == 0) {
                strncpy(g_sounds[i].replacement, value,
                        sizeof(g_sounds[i].replacement) - 1);
                g_sounds[i].replacement[sizeof(g_sounds[i].replacement) - 1] = '\0';
                break;
            }
        }
    }
    fclose(f);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Sound Replacement Lookup — called from assembly cave
 * ═══════════════════════════════════════════════════════════════════════════ */

static char g_replacement_buf[256];

static const char* __cdecl find_sound_replacement(const char* original) {
    const char* name;
    int i;

    if (!original || !*original) return NULL;

    /* Strip "sounds\" or "sounds/" prefix */
    name = original;
    if (_strnicmp(original, "sounds\\", 7) == 0 ||
        _strnicmp(original, "sounds/", 7) == 0) {
        name = original + 7;
    }

    /* Look up in sound table */
    for (i = 0; i < (int)NUM_SOUNDS; i++) {
        if (_stricmp(name, g_sounds[i].name) == 0) {
            /* Check if user specified a replacement */
            if (g_sounds[i].replacement[0] != '\0' &&
                _stricmp(g_sounds[i].replacement, g_sounds[i].name) != 0) {
                /* Build full path: sounds\<replacement> */
                _snprintf(g_replacement_buf, sizeof(g_replacement_buf),
                          "sounds\\%s", g_sounds[i].replacement);
                g_replacement_buf[sizeof(g_replacement_buf) - 1] = '\0';
                return g_replacement_buf;
            }
            return NULL; /* no replacement */
        }
    }
    return NULL; /* sound not in table */
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Hook Installation — Assembly Cave on Sound_LoadOggOrWav (0x459660)
 *
 * Original entry: 81 EC 00 01 00 00  (SUB ESP, 0x100 — 6 bytes)
 * Patched to:     E9 xx xx xx xx 90  (JMP cave + NOP — 6 bytes)
 *
 * Cave:
 *   PUSH EAX, ECX, EDX        ; save registers
 *   PUSH [ESP+0x10]           ; push filename param
 *   CALL find_sound_replacement
 *   ADD ESP, 4                ; clean param
 *   TEST EAX, EAX
 *   JZ skip                   ; no replacement → skip
 *   MOV [ESP+0x10], EAX       ; replace filename on stack
 * skip:
 *   POP EDX, ECX, EAX        ; restore registers
 *   SUB ESP, 0x100            ; original instruction
 *   JMP 0x459666              ; continue original function
 * ═══════════════════════════════════════════════════════════════════════════ */

#define SOUND_LOAD_OGG_WAV_ADDR  0x459660
#define SOUND_LOAD_OGG_WAV_CONT  0x459666  /* address after patched instruction */

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
    /* PUSH DWORD [ESP+0x10]  (filename — 3 pushes + ret_addr = 16 bytes offset) */
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
    /* MOV [ESP+0x10], EAX  (replace filename) */
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
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DllMain
 * ═══════════════════════════════════════════════════════════════════════════ */

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved)
{
    switch (reason) {
    case DLL_PROCESS_ATTACH:
        g_hSelf = hModule;
        load_real_bass();

        /* Generate config if it doesn't exist */
        {
            char path[MAX_PATH];
            FILE* f = NULL;
            get_config_path(path, MAX_PATH);
            if (fopen_s(&f, path, "r") != 0 || !f) {
                generate_config(path);
            } else {
                fclose(f);
            }
        }

        /* Read config and install hook BEFORE game loads sounds */
        read_config();
        install_hook();
        break;

    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
