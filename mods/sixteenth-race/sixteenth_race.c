/*
 * sixteenth_race.c - Adds a 16th race ("Test Race") to Hamsterball
 * 
 * Clones Warm-Up Race board constructor and scene setup.
 * Loads "levels\leveltest" MESHWORLD file.
 * Adds "Test Race" entry to the Practice menu.
 *
 * Build: i686-w64-mingw32-gcc -shared -o bass.dll sixteenth_race.c \
 *        -lwinmm -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
 *        -Wl,--add-stdcall-alias
 *
 * Install: Replace bass.dll in game directory. Copy Level1.MESHWORLD to
 *          LevelTest.MESHWORLD in the levels\ folder.
 */

#include <windows.h>
#include <string.h>

/* ========== Game function addresses ========== */
#define ADDR_MALLOC              0x004BA57B
#define ADDR_BOARD_WARMUP_CTOR   0x0041CA40
#define ADDR_ARENA_WARMUP_CTOR   0x004224A0
#define ADDR_CREATE_COLOR        0x00453150
#define ADDR_UI_LIST_ADD_ITEM    0x004497F0
#define ADDR_UI_LIST_SEP         0x00449430

/* ========== Patch points ========== */
/* Tournament_AdvanceRace switch */
#define TOURNAMENT_CMP_BYTE      0x004270FB  /* byte: 0E -> 0F */
#define TOURNAMENT_JMP_ENTRY15   0x00427658  /* NOP padding -> code cave addr */
#define TOURNAMENT_POST_SWITCH   0x004273D1
#define TOURNAMENT_ALLOC_FAIL    0x004273CF

/* TourneyMenu_CreateBoard switch (arena) */
#define TOURNEY_CMP_BYTE         0x0042679F  /* byte: 0E -> 0F */
#define TOURNEY_JMP_ENTRY15      0x00426AEC  /* NOP padding -> code cave addr */
#define TOURNEY_POST_SWITCH      0x00426A79
#define TOURNEY_ALLOC_FAIL       0x00426A77

/* GetLevelPath string push */
#define GETLEVELPATH_PATCH       0x0040D202  /* PUSH 0x4CF8E0 -> JMP cave */
#define GETLEVELPATH_RETURN      0x0040D20A

/* Practice menu separator (after Impossible race entry) */
#define PRACTICE_MENU_PATCH      0x0042F4F7  /* PUSH 0xA -> JMP cave */
#define PRACTICE_MENU_RETURN     0x0042F500

/* Game string addresses */
#define STR_LEVEL1_PATH          0x004CF8E0  /* "levels\\level1" */

/* ========== DLL string constants ========== */
static const char STR_LEVELTEST[]      = "levels\\leveltest";
static const char STR_TEST_RACE[]     = "TEST RACE";
static const char STR_TEST_BOARD[]    = "Board (Test)";
static const char STR_TEST_ARENA_BD[] = "Board (Test Arena)";
static const char STR_TEST_ARENA[]    = "TEST ARENA";
static const char STR_TEST_DISPLAY[] = "Test Race";
static const char STR_ENTRY_15[]      = "15";

/* ========== Patch helpers ========== */
static void patch_byte(DWORD addr, BYTE val) {
    DWORD old;
    VirtualProtect((void*)addr, 1, PAGE_EXECUTE_READWRITE, &old);
    *(BYTE*)addr = val;
    VirtualProtect((void*)addr, 1, old, &old);
}

static void patch_dword(DWORD addr, DWORD val) {
    DWORD old;
    VirtualProtect((void*)addr, 4, PAGE_EXECUTE_READWRITE, &old);
    *(DWORD*)addr = val;
    VirtualProtect((void*)addr, 4, old, &old);
}

static void patch_jmp(DWORD addr, DWORD target) {
    DWORD old;
    VirtualProtect((void*)addr, 5, PAGE_EXECUTE_READWRITE, &old);
    *(BYTE*)addr = 0xE9;
    *(DWORD*)(addr + 1) = target - (addr + 5);
    VirtualProtect((void*)addr, 5, old, &old);
}

/* ========== Code cave builder ========== */
typedef struct { BYTE* p; DWORD off; } Cave;

static void c_byte(Cave* c, BYTE b) { c->p[c->off++] = b; }
static void c_word(Cave* c, WORD w) { *(WORD*)(c->p + c->off) = w; c->off += 2; }
static void c_dword(Cave* c, DWORD d) { *(DWORD*)(c->p + c->off) = d; c->off += 4; }

static void c_call(Cave* c, DWORD target) {
    c->p[c->off++] = 0xE8;
    *(DWORD*)(c->p + c->off) = target - ((DWORD)(c->p + c->off) + 4);
    c->off += 4;
}

static void c_jmp(Cave* c, DWORD target) {
    c->p[c->off++] = 0xE9;
    *(DWORD*)(c->p + c->off) = target - ((DWORD)(c->p + c->off) + 4);
    c->off += 4;
}

static void c_jz(Cave* c, DWORD target) {
    c->p[c->off++] = 0x0F;
    c->p[c->off++] = 0x84;
    *(DWORD*)(c->p + c->off) = target - ((DWORD)(c->p + c->off) + 4);
    c->off += 4;
}

static void c_jnz(Cave* c, DWORD target) {
    c->p[c->off++] = 0x0F;
    c->p[c->off++] = 0x85;
    *(DWORD*)(c->p + c->off) = target - ((DWORD)(c->p + c->off) + 4);
    c->off += 4;
}

static void c_push32(Cave* c, DWORD val) {
    c->p[c->off++] = 0x68;
    *(DWORD*)(c->p + c->off) = val;
    c->off += 4;
}

/* Emit: MOV DWORD [EAX+offset], imm32 */
static void c_mov_eax_dword(Cave* c, DWORD offset, DWORD val) {
    c->p[c->off++] = 0xC7;  /* MOV r/m32, imm32 */
    c->p[c->off++] = 0x80;  /* ModRM: [EAX+disp32] */
    *(DWORD*)(c->p + c->off) = offset;
    c->off += 4;
    *(DWORD*)(c->p + c->off) = val;
    c->off += 4;
}

/* ========== Copy level file ========== */
static void copy_level_file(void) {
    char dir[MAX_PATH], src[MAX_PATH], dst[MAX_PATH];
    HMODULE hMod = NULL;

    GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
                       (LPCSTR)&copy_level_file, &hMod);
    if (!GetModuleFileNameA(hMod, dir, MAX_PATH)) return;

    /* Strip filename to get directory */
    char* p = strrchr(dir, '\\');
    if (p) *p = '\0';

    /* Try Level1.MESHWORLD -> LevelTest.MESHWORLD */
    lstrcpyA(src, dir); lstrcatA(src, "\\levels\\Level1.MESHWORLD");
    lstrcpyA(dst, dir); lstrcatA(dst, "\\levels\\LevelTest.MESHWORLD");
    if (CopyFileA(src, dst, TRUE)) return;

    /* Try lowercase */
    lstrcpyA(src, dir); lstrcatA(src, "\\levels\\level1.MESHWORLD");
    lstrcpyA(dst, dir); lstrcatA(dst, "\\levels\\leveltest.MESHWORLD");
    if (CopyFileA(src, dst, TRUE)) return;
}

/* ========== Patch thread ========== */
static DWORD WINAPI patch_thread(LPVOID unused) {
    /* Allocate code cave memory (4KB) */
    BYTE* mem = (BYTE*)VirtualAlloc(NULL, 4096, MEM_COMMIT | MEM_RESERVE,
                                     PAGE_EXECUTE_READWRITE);
    if (!mem) return 1;

    Cave c = { mem, 0 };

    /* ===== Cave 1: Tournament case 15 (Test Race board) ===== */
    DWORD tournament_cave = (DWORD)(c.p + c.off);

    /* PUSH 0x436C (alloc size, same as WarmUp) */
    c_push32(&c, 0x436C);
    /* CALL malloc */
    c_call(&c, ADDR_MALLOC);
    /* ADD ESP, 4 */
    c_byte(&c, 0x83); c_byte(&c, 0xC4); c_byte(&c, 0x04);
    /* MOV [ESP+0x20], EAX */
    c_byte(&c, 0x89); c_byte(&c, 0x44); c_byte(&c, 0x24); c_byte(&c, 0x20);
    /* CMP EAX, EBX (EBX=0) */
    c_byte(&c, 0x39); c_byte(&c, 0xD8);
    /* MOV DWORD [ESP+0x18], 0xF (case index = 15) */
    c_byte(&c, 0xC7); c_byte(&c, 0x44); c_byte(&c, 0x24); c_byte(&c, 0x18);
    c_dword(&c, 0x0F);
    /* JZ alloc_fail (placeholder, fix later) */
    DWORD jz1 = c.off;
    c_byte(&c, 0x0F); c_byte(&c, 0x84); c_dword(&c, 0);
    /* MOV EDX, [ESI+0x4] (App pointer) */
    c_byte(&c, 0x8B); c_byte(&c, 0x56); c_byte(&c, 0x04);
    /* PUSH EDX */
    c_byte(&c, 0x52);
    /* MOV ECX, EAX (this = board) */
    c_byte(&c, 0x8B); c_byte(&c, 0xC8);
    /* CALL BoardLevel1_WarmUp_Ctor */
    c_call(&c, ADDR_BOARD_WARMUP_CTOR);
    /* MOV DWORD [EAX+0x868], STR_TEST_BOARD */
    c_mov_eax_dword(&c, 0x868, (DWORD)STR_TEST_BOARD);
    /* MOV DWORD [EAX+0x29B4], STR_TEST_RACE */
    c_mov_eax_dword(&c, 0x29B4, (DWORD)STR_TEST_RACE);
    /* JMP post-switch */
    c_jmp(&c, TOURNAMENT_POST_SWITCH);
    /* alloc_fail: (fix up JZ target) */
    *(DWORD*)(c.p + jz1 + 2) = (DWORD)(c.p + c.off) - ((DWORD)(c.p + jz1) + 6);
    c_jmp(&c, TOURNAMENT_ALLOC_FAIL);

    /* ===== Cave 2: TourneyMenu case 15 (Test Arena board) ===== */
    DWORD tourney_cave = (DWORD)(c.p + c.off);

    /* PUSH 0x47E0 (alloc size for arena board) */
    c_push32(&c, 0x47E0);
    /* CALL malloc */
    c_call(&c, ADDR_MALLOC);
    /* ADD ESP, 4 */
    c_byte(&c, 0x83); c_byte(&c, 0xC4); c_byte(&c, 0x04);
    /* MOV [ESP+0x4], EAX (tourney uses ESP+0x4) */
    c_byte(&c, 0x89); c_byte(&c, 0x44); c_byte(&c, 0x24); c_byte(&c, 0x04);
    /* TEST EAX, EAX */
    c_byte(&c, 0x85); c_byte(&c, 0xC0);
    /* MOV DWORD [ESP+0x10], 0xF (case index = 15) */
    c_byte(&c, 0xC7); c_byte(&c, 0x44); c_byte(&c, 0x24); c_byte(&c, 0x10);
    c_dword(&c, 0x0F);
    /* JZ alloc_fail (placeholder) */
    DWORD jz2 = c.off;
    c_byte(&c, 0x0F); c_byte(&c, 0x84); c_dword(&c, 0);
    /* MOV EDX, [ESI+0x4] (App pointer) */
    c_byte(&c, 0x8B); c_byte(&c, 0x56); c_byte(&c, 0x04);
    /* PUSH EDX */
    c_byte(&c, 0x52);
    /* MOV ECX, EAX (this = board) */
    c_byte(&c, 0x8B); c_byte(&c, 0xC8);
    /* CALL ArenaBoard_Warmup_Ctor */
    c_call(&c, ADDR_ARENA_WARMUP_CTOR);
    /* MOV DWORD [EAX+0x868], STR_TEST_ARENA_BD */
    c_mov_eax_dword(&c, 0x868, (DWORD)STR_TEST_ARENA_BD);
    /* MOV DWORD [EAX+0x29B4], STR_TEST_ARENA */
    c_mov_eax_dword(&c, 0x29B4, (DWORD)STR_TEST_ARENA);
    /* JMP post-switch */
    c_jmp(&c, TOURNEY_POST_SWITCH);
    /* alloc_fail: (fix up JZ target) */
    *(DWORD*)(c.p + jz2 + 2) = (DWORD)(c.p + c.off) - ((DWORD)(c.p + jz2) + 6);
    c_jmp(&c, TOURNEY_ALLOC_FAIL);

    /* ===== Cave 3: GetLevelPath redirect ===== */
    DWORD getlevelpath_cave = (DWORD)(c.p + c.off);

    /* PUSH EDX (save gfx device) */
    c_byte(&c, 0x52);
    /* MOV EDX, [ESI+0x29B4] (race name pointer) */
    c_byte(&c, 0x8B); c_byte(&c, 0x96);
    c_dword(&c, 0x29B4);
    /* TEST EDX, EDX (NULL check - race name not set during startup) */
    c_byte(&c, 0x85); c_byte(&c, 0xD2);
    /* JZ .original (NULL → use original path, don't dereference) */
    DWORD jz_null = c.off;
    c_byte(&c, 0x74); c_byte(&c, 0x00);
    /* CMP EDX, 0x10000 (reject pointers below 64KB - always invalid on Windows) */
    c_byte(&c, 0x81); c_byte(&c, 0xFA);
    c_dword(&c, 0x10000);
    /* JB .original (too low → use original path) */
    DWORD jb_low = c.off;
    c_byte(&c, 0x72); c_byte(&c, 0x00);
    /* CMP DWORD [EDX], 0x54534554 ("TEST" in little-endian) */
    c_byte(&c, 0x81); c_byte(&c, 0x3A);
    c_dword(&c, 0x54534554);
    /* POP EDX (restore gfx — taken before JNE, so .original_nopop skips POP) */
    c_byte(&c, 0x5A);
    /* JNE .original_nopop (placeholder — EDX already popped, skip POP) */
    DWORD jnz = c.off;
    c_byte(&c, 0x0F); c_byte(&c, 0x85); c_dword(&c, 0);
    /* PUSH STR_LEVELTEST ("levels\\leveltest") */
    c_push32(&c, (DWORD)STR_LEVELTEST);
    /* JMP .done (placeholder) */
    DWORD jmp_done = c.off;
    c_byte(&c, 0xE9); c_dword(&c, 0);
    /* .original_pop: POP EDX (for NULL/low paths that haven't popped yet) */
    DWORD orig_pop_pos = (DWORD)(c.p + c.off);
    c_byte(&c, 0x5A);  /* POP EDX */
    /* .original_nopop: PUSH 0x004CF8E0 ("levels\\level1") */
    DWORD orig_nopop_pos = (DWORD)(c.p + c.off);
    c_push32(&c, STR_LEVEL1_PATH);
    /* Fix JZ null target → .original_pop */
    *(c.p + jz_null + 1) = (BYTE)(orig_pop_pos - ((DWORD)(c.p + jz_null) + 2));
    /* Fix JB low target → .original_pop */
    *(c.p + jb_low + 1) = (BYTE)(orig_pop_pos - ((DWORD)(c.p + jb_low) + 2));
    /* Fix JNZ target → .original_nopop (EDX already popped, skip POP) */
    *(DWORD*)(c.p + jnz + 2) = orig_nopop_pos - ((DWORD)(c.p + jnz) + 6);
    /* Fix JMP .done target */
    *(DWORD*)(c.p + jmp_done + 1) = (DWORD)(c.p + c.off) - ((DWORD)(c.p + jmp_done) + 5);
    /* .done: PUSH EDX (original instruction at 0x0040D207) */
    c_byte(&c, 0x52);
    /* MOV ECX, EAX (original instruction at 0x0040D208) */
    c_byte(&c, 0x8B); c_byte(&c, 0xC8);
    /* JMP return to 0x0040D20A */
    c_jmp(&c, GETLEVELPATH_RETURN);

    /* ===== Cave 4: Practice menu 16th entry ===== */
    DWORD practice_cave = (DWORD)(c.p + c.off);

    /* --- Add "Test Race" entry (reuse WarmUp image at ESI+0xCDC) --- */
    /* MOV EAX, [ESI+0xCDC] (WarmUp image) */
    c_byte(&c, 0x8B); c_byte(&c, 0x86);
    c_dword(&c, 0xCDC);
    /* PUSH EAX (push image) */
    c_byte(&c, 0x50);
    /* SUB ESP, 0x14 (color struct space) */
    c_byte(&c, 0x83); c_byte(&c, 0xEC); c_byte(&c, 0x14);
    /* MOV ECX, ESP */
    c_byte(&c, 0x8B); c_byte(&c, 0xCC);
    /* MOV [ESP+0x30], ESP */
    c_byte(&c, 0x89); c_byte(&c, 0x64); c_byte(&c, 0x24); c_byte(&c, 0x30);
    /* PUSH 0x3f800000 (a = 1.0) */
    c_push32(&c, 0x3f800000);
    /* PUSH 0x3f800000 (b = 1.0) */
    c_push32(&c, 0x3f800000);
    /* PUSH 0x3f400000 (g = 0.75) */
    c_push32(&c, 0x3f400000);
    /* PUSH 0x3f800000 (r = 1.0) */
    c_push32(&c, 0x3f800000);
    /* MOV byte [ESP+0x38], 0 */
    c_byte(&c, 0xC6); c_byte(&c, 0x44); c_byte(&c, 0x24); c_byte(&c, 0x38); c_byte(&c, 0x00);
    /* CALL CreateColor */
    c_call(&c, ADDR_CREATE_COLOR);
    /* PUSH STR_ENTRY_15 ("15") */
    c_push32(&c, (DWORD)STR_ENTRY_15);
    /* PUSH STR_TEST_DISPLAY ("Test Race") */
    c_push32(&c, (DWORD)STR_TEST_DISPLAY);
    /* MOV ECX, ESI (this = menu) */
    c_byte(&c, 0x8B); c_byte(&c, 0xCE);
    /* CALL UIList_AddItem */
    c_call(&c, ADDR_UI_LIST_ADD_ITEM);

    /* --- Execute original separator (was at 0x0042F4F7) --- */
    /* PUSH 0xA */
    c_byte(&c, 0x6A); c_byte(&c, 0x0A);
    /* MOV ECX, ESI */
    c_byte(&c, 0x8B); c_byte(&c, 0xCE);
    /* CALL UI_LIST_SEP */
    c_call(&c, ADDR_UI_LIST_SEP);

    /* JMP return to 0x0042F500 */
    c_jmp(&c, PRACTICE_MENU_RETURN);

    /* ===== Apply all patches ===== */

    /* 1. Tournament switch: extend CMP from 0x0E to 0x0F */
    patch_byte(TOURNAMENT_CMP_BYTE, 0x0F);
    /* Write 16th jump table entry */
    patch_dword(TOURNAMENT_JMP_ENTRY15, tournament_cave);

    /* 2. TourneyMenu switch: extend CMP from 0x0E to 0x0F */
    patch_byte(TOURNEY_CMP_BYTE, 0x0F);
    /* Write 16th jump table entry */
    patch_dword(TOURNEY_JMP_ENTRY15, tourney_cave);

    /* 3. GetLevelPath: redirect string push to code cave */
    patch_jmp(GETLEVELPATH_PATCH, getlevelpath_cave);

    /* 4. Practice menu: inject 16th entry before separator */
    patch_jmp(PRACTICE_MENU_PATCH, practice_cave);

    /* 5. Copy Level1.MESHWORLD -> LevelTest.MESHWORLD */
    copy_level_file();

    return 0;
}

/* ========== BASS proxy exports (stubs - never called by game code) ========== */
__declspec(dllexport) BOOL __stdcall BASS_Init(int a, int b, int c, HWND d, void* e) { return TRUE; }
__declspec(dllexport) void __stdcall BASS_Free(void) {}
__declspec(dllexport) BOOL __stdcall BASS_Stop(void) { return TRUE; }
__declspec(dllexport) BOOL __stdcall BASS_Start(void) { return TRUE; }
__declspec(dllexport) BOOL __stdcall BASS_SetConfig(int a, int b) { return TRUE; }
__declspec(dllexport) int __stdcall BASS_ErrorGetCode(void) { return 0; }
__declspec(dllexport) void* __stdcall BASS_MusicLoad(void* a, void* b, void* c, DWORD d, DWORD e, DWORD f) { return NULL; }
__declspec(dllexport) BOOL __stdcall BASS_MusicPlayEx(DWORD a, DWORD b, BOOL c) { return TRUE; }
__declspec(dllexport) BOOL __stdcall BASS_ChannelStop(DWORD a) { return TRUE; }
__declspec(dllexport) BOOL __stdcall BASS_ChannelSetAttributes(DWORD a, float b, float c, int d) { return TRUE; }

/* ========== DllMain ========== */
BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        CreateThread(NULL, 0, patch_thread, NULL, 0, NULL);
    }
    return TRUE;
}
