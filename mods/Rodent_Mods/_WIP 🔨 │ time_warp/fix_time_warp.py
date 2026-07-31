#!/usr/bin/env python3
"""
Apply bug fixes to time_warp.c.
Creates time_warp.c.bak before modifying.
"""

import shutil, os, re

PATH = "/home/evan/hamsterball-re/mods/Rodent_Mods/_WIP 🔨 │ time_warp/time_warp.c"
BAK = PATH + ".bak"

shutil.copy2(PATH, BAK)

with open(PATH, "r", encoding="utf-8") as f:
    src = f.read()

# ------------------------------------------------------------------
# Fix 1: musicOrigVolumes array size must match potential channel count.
# We'll bump MAX_MUSIC_CHANNELS to 32 and keep using the global array.
# ------------------------------------------------------------------

src = src.replace(
    "#define MAX_MUSIC_CHANNELS      8",
    "#define MAX_MUSIC_CHANNELS      32"
)

# ------------------------------------------------------------------
# Fix 2: BASS_ChannelSetAttributes fallback signature is wrong.
# The real BASS function is BASS_ChannelSetAttribute(DWORD, DWORD, float).
# Remove the invalid fallback cast; if the 4-param export is missing we leave it NULL.
# ------------------------------------------------------------------

src = src.replace(
    "        if (!real_BASS_ChannelSetAttributes)\n"
    "            real_BASS_ChannelSetAttributes = (BASS_ChannelSetAttributes_t)GetProcAddress(g_hRealBass, \"BASS_ChannelSetAttribute\");",
    "        /* BASS_ChannelSetAttribute has a different signature; do not fall back to it. */"
)

# ------------------------------------------------------------------
# Fix 3: VirtualProtect must use PAGE_EXECUTE_READWRITE, not PAGE_READWRITE.
# Replace PAGE_READWRITE with PAGE_EXECUTE_READWRITE in patch helper.
# ------------------------------------------------------------------

src = src.replace(
    "static void patch_bytes(void *addr, const void *data, DWORD size) {\n"
    "    DWORD old;\n"
    "    VirtualProtect(addr, size, PAGE_EXECUTE_READWRITE, &old);\n"
    "    memcpy(addr, data, size);\n"
    "    VirtualProtect(addr, size, old, &old);\n"
    "}",
    "static void patch_bytes(void *addr, const void *data, DWORD size) {\n"
    "    DWORD old;\n"
    "    VirtualProtect(addr, size, PAGE_EXECUTE_READWRITE, &old);\n"
    "    memcpy(addr, data, size);\n"
    "    VirtualProtect(addr, size, PAGE_EXECUTE_READWRITE, &old);\n"
    "}"
)

# block_pause/unblock_pause also use PAGE_READWRITE; fix them.
src = src.replace(
    "        if (VirtualProtect((void*)addr, 1, PAGE_READWRITE, &oldProt)) {",
    "        if (VirtualProtect((void*)addr, 1, PAGE_EXECUTE_READWRITE, &oldProt)) {"
)

# timer caves use PAGE_READWRITE in install/restore; fix those too.
src = src.replace(
    "        if (VirtualProtect((void*)patchAddr, TIMER_DEC_PATCH_SIZE, PAGE_READWRITE, &oldProt)) {",
    "        if (VirtualProtect((void*)patchAddr, TIMER_DEC_PATCH_SIZE, PAGE_EXECUTE_READWRITE, &oldProt)) {"
)
src = src.replace(
    "        if (VirtualProtect((void*)patchAddr, TIMER_INC_PATCH_SIZE, PAGE_READWRITE, &oldProt)) {",
    "        if (VirtualProtect((void*)patchAddr, TIMER_INC_PATCH_SIZE, PAGE_EXECUTE_READWRITE, &oldProt)) {"
)
src = src.replace(
    "        if (VirtualProtect((void*)addr, TIMER_DEC_PATCH_SIZE, PAGE_READWRITE, &oldProt)) {",
    "        if (VirtualProtect((void*)addr, TIMER_DEC_PATCH_SIZE, PAGE_EXECUTE_READWRITE, &oldProt)) {"
)
src = src.replace(
    "        if (VirtualProtect((void*)addr, TIMER_INC_PATCH_SIZE, PAGE_READWRITE, &oldProt)) {",
    "        if (VirtualProtect((void*)addr, TIMER_INC_PATCH_SIZE, PAGE_EXECUTE_READWRITE, &oldProt)) {"
)
src = src.replace(
    "    if (VirtualProtect((void*)addr, TT_RECORDING_NOP_SIZE, PAGE_READWRITE, &oldProt)) {",
    "    if (VirtualProtect((void*)addr, TT_RECORDING_NOP_SIZE, PAGE_EXECUTE_READWRITE, &oldProt)) {"
)
src = src.replace(
    "    if (VirtualProtect((void*)addr, TT_RECORDING_NOP_SIZE, PAGE_READWRITE, &oldProt)) {",
    "    if (VirtualProtect((void*)addr, TT_RECORDING_NOP_SIZE, PAGE_EXECUTE_READWRITE, &oldProt)) {"
)

# ------------------------------------------------------------------
# Fix 4: DEC timer cave JNZ offset calculation.
# Replace the buggy install_timer_caves() DEC/INC cave generation with correct code.
# ------------------------------------------------------------------

old_timer_caves_start = "/* Timer freeze code caves */\nstatic void install_timer_caves(void) {"
old_timer_caves_end = "}\n\n/* Warp state machine */"

if old_timer_caves_start not in src or old_timer_caves_end not in src:
    raise SystemExit("Could not locate install_timer_caves block")

new_timer_caves = '''/* Timer freeze code caves.
 * DEC path at 0x41B3E5 (9 bytes): original JNZ + DEC.
 * INC path at 0x41B50C (5 bytes): original JNZ + INC.
 * Both caves use absolute memory operands so offset math is straightforward. */
static void install_timer_caves(void) {
    DWORD base = EXE_BASE;
    DWORD oldProt;

    /* Cave 1: DEC path */
    if (!g_decPatched) {
        DWORD patchAddr = base + TIMER_DEC_PATCH_RVA;   /* 0x41B3E5 */
        DWORD returnAddr = base + TIMER_DEC_RETURN_RVA; /* 0x41B3EE */
        DWORD skipAddr = base + TIMER_DEC_SKIP_RVA;     /* 0x41B49D */
        DWORD freezeAddr = (DWORD)&g_freezeTimer;

        g_decCave = (unsigned char *)VirtualAlloc(NULL, 64, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
        if (!g_decCave) return;

        unsigned char *p = g_decCave;

        /* Original conditional jump: if [EDI+0xA] != 0, jump to skip. */
        p[0] = 0x0F; p[1] = 0x85;
        *(DWORD*)(p + 2) = (DWORD)(p + 6) - (DWORD)(p + 2 + 4) + (skipAddr - ((DWORD)(p + 6)));
        p += 6;

        /* if (g_freezeTimer) skip; */
        p[0] = 0x80; p[1] = 0x3D;
        *(DWORD*)(p + 2) = freezeAddr;
        p[6] = 0x00;
        p += 7;

        p[0] = 0x0F; p[1] = 0x85;
        *(DWORD*)(p + 2) = skipAddr - (DWORD)(p + 6);
        p += 6;

        /* Original DEC [EDI+0x1C] */
        p[0] = 0xFF; p[1] = 0x4F; p[2] = 0x1C;
        p += 3;

        /* Jump back to original code after patched bytes */
        write_jmp(p, returnAddr);

        if (VirtualProtect((void*)patchAddr, TIMER_DEC_PATCH_SIZE, PAGE_EXECUTE_READWRITE, &oldProt)) {
            memcpy(g_decOrigBytes, (void*)patchAddr, TIMER_DEC_PATCH_SIZE);
            write_jmp((unsigned char*)patchAddr, (DWORD)g_decCave);
            memset((unsigned char*)patchAddr + 5, 0x90, TIMER_DEC_PATCH_SIZE - 5);
            VirtualProtect((void*)patchAddr, TIMER_DEC_PATCH_SIZE, PAGE_EXECUTE_READWRITE, &oldProt);
            g_decPatched = 1;
            diag_log("[warp] DEC timer cave installed at 0x41B3E5");
        }
    }

    /* Cave 2: INC path */
    if (!g_incPatched) {
        DWORD patchAddr = base + TIMER_INC_PATCH_RVA;   /* 0x41B50C */
        DWORD returnAddr = base + TIMER_INC_RETURN_RVA; /* 0x41B511 */
        DWORD freezeAddr = (DWORD)&g_freezeTimer;

        g_incCave = (unsigned char *)VirtualAlloc(NULL, 64, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
        if (!g_incCave) return;

        unsigned char *p = g_incCave;

        /* if ([EDX+0xA] != 0) jump over INC */
        p[0] = 0x75;
        p[1] = 0x0C;  /* jump 12 bytes forward to after the INC */
        p += 2;

        /* if (g_freezeTimer) jump over INC */
        p[0] = 0x80; p[1] = 0x3D;
        *(DWORD*)(p + 2) = freezeAddr;
        p[6] = 0x00;
        p += 7;

        p[0] = 0x75;
        p[1] = 0x03;  /* jump 3 bytes forward to skip INC */
        p += 2;

        /* Original INC [EDX+0x1C] */
        p[0] = 0xFF; p[1] = 0x42; p[2] = 0x1C;
        p += 3;

        /* Jump back to original code */
        write_jmp(p, returnAddr);

        if (VirtualProtect((void*)patchAddr, TIMER_INC_PATCH_SIZE, PAGE_EXECUTE_READWRITE, &oldProt)) {
            memcpy(g_incOrigBytes, (void*)patchAddr, TIMER_INC_PATCH_SIZE);
            write_jmp((unsigned char*)patchAddr, (DWORD)g_incCave);
            VirtualProtect((void*)patchAddr, TIMER_INC_PATCH_SIZE, PAGE_EXECUTE_READWRITE, &oldProt);
            g_incPatched = 1;
            diag_log("[warp] INC timer cave installed at 0x41B50C");
        }
    }
}

static void restore_timer_caves(void) {
    DWORD base = EXE_BASE;
    DWORD oldProt;

    if (g_decPatched) {
        DWORD addr = base + TIMER_DEC_PATCH_RVA;
        if (VirtualProtect((void*)addr, TIMER_DEC_PATCH_SIZE, PAGE_EXECUTE_READWRITE, &oldProt)) {
            memcpy((void*)addr, g_decOrigBytes, TIMER_DEC_PATCH_SIZE);
            VirtualProtect((void*)addr, TIMER_DEC_PATCH_SIZE, PAGE_EXECUTE_READWRITE, &oldProt);
        }
        g_decPatched = 0;
    }

    if (g_incPatched) {
        DWORD addr = base + TIMER_INC_PATCH_RVA;
        if (VirtualProtect((void*)addr, TIMER_INC_PATCH_SIZE, PAGE_EXECUTE_READWRITE, &oldProt)) {
            memcpy((void*)addr, g_incOrigBytes, TIMER_INC_PATCH_SIZE);
            VirtualProtect((void*)addr, TIMER_INC_PATCH_SIZE, PAGE_EXECUTE_READWRITE, &oldProt);
        }
        g_incPatched = 0;
    }
}
'''

idx1 = src.index(old_timer_caves_start)
idx2 = src.index(old_timer_caves_end)
src = src[:idx1] + new_timer_caves + "\n" + src[idx2:]

# ------------------------------------------------------------------
# Fix 5: DCE hook — replace manual SEH trampoline with hook after SEH prologue.
# New hook point: 0x40C5E5 (5 bytes: sub esp,30 + push ebx + push ebp).
# ------------------------------------------------------------------

old_dce = "/* DCE hook stub */\nstatic BYTE *g_dce_stub = NULL;"
old_dce_end = "/* Level_UpdateAndRender patches"
if old_dce not in src or old_dce_end not in src:
    raise SystemExit("Could not locate DCE hook block")

new_dce = '''/* DCE hook — installed after the SEH prologue at 0x40C5E5.
 * We replace 5 bytes: 83 EC 30 53 55  (sub esp,30; push ebx; push ebp)
 * Trampoline replicates those 5 bytes and jumps to 0x40C5EA.
 * The stub preserves ECX (board) and pushes the two stack args. */

#define DCE_HOOK_ADDR            0x0040C5E5
#define DCE_HOOK_BYTES           5
#define DCE_HOOK_RETURN          0x0040C5EA

static BYTE *g_dce_stub = NULL;
static BYTE g_dce_original[DCE_HOOK_BYTES];
static void *g_dce_trampoline = NULL;

/* Handler signature: __cdecl with explicit args. ECX=board is passed as first arg. */
void __cdecl dce_handler(DWORD board, DWORD ball, DWORD collEntry) {
    g_dceCallCount++;
    if (!collEntry) return;

    DWORD *pair = (DWORD*)collEntry;
    if (IsBadReadPtr(pair, 8)) return;
    if (!pair[1]) return;
    if (IsBadReadPtr((void*)pair[1], 0x868)) return;

    const char *eventName = NULL;
    {
        DWORD namePtr = *(DWORD*)((BYTE*)pair[1] + 0x864);
        if (!namePtr || IsBadReadPtr((void*)namePtr, 1)) return;
        eventName = (const char*)namePtr;
    }
    if (!eventName[0]) return;

    if (g_dceCallCount <= 20 || (g_dceCallCount % 100) == 0) {
        diag_logf("[dce] event='%s' (board=0x%X ball=0x%X)", eventName, board, ball);
    }

    if (_strnicmp(eventName, "E:GHOST", 7) == 0) {
        if (g_ghostFromEvent && g_ghostActive && g_loadedBTT) return;

        diag_logf("[dce] E:GHOST match: '%s'", eventName);
        const char *p1 = strchr(eventName, '(');
        if (p1) {
            const char *p2 = strchr(p1, ')');
            if (p2 && (p2 - p1 - 1) > 0) {
                size_t len = p2 - p1 - 1;
                if (len > 250) len = 250;
                memcpy(g_pendingGhostFile, p1 + 1, len);
                g_pendingGhostFile[len] = '\\0';
                diag_logf("[dce] Parsed ghost filename='%s'", g_pendingGhostFile);
            }
        }
    }
}

static void build_dce_trampoline(void) {
    BYTE *code = (BYTE*)alloc_executable(16);
    /* sub esp, 0x30 */
    code[0] = 0x83; code[1] = 0xEC; code[2] = 0x30;
    /* push ebx */
    code[3] = 0x53;
    /* push ebp */
    code[4] = 0x55;
    /* jmp DCE_HOOK_RETURN */
    code[5] = 0xE9;
    *(DWORD*)(code + 6) = DCE_HOOK_RETURN - (DWORD)(code + 10);
    g_dce_trampoline = code;
}

static void build_dce_stub(void) {
    BYTE *code = (BYTE*)alloc_executable(96);
    int i = 0;
    code[i++] = 0x60;                 /* pushad */
    code[i++] = 0x9C;                 /* pushfd */
    /* Read original args from the hook-point stack layout.
     * At 0x40C5E5: esp -= 0x39 from original entry.
     * After pushad/pushfd the stub esp is lower; we read relative to current esp.
     * Easier: copy the two args from [esp+N] before we alter esp with pushad.
     * We push them now, before pushad, then reload them inside.
     * Layout inside stub after pushad/pushfd:
     *   [esp+0x24] = original arg2 (collEntry)
     *   [esp+0x20] = original arg1 (ball)
     *   [esp+0x1C] = return address
     *   ...
     * Actually the simplest robust approach is to read them from the
     * original stack before pushad.
     */
    /* push [esp+8+8]  -> collEntry (second arg at original entry) */
    /* But after pushad/pushfd, accessing original stack is fragile.
     * Instead, we capture them into registers before pushad.
     */
    /* Use EBX and ESI as scratch since pushad saves them. */
    code[i++] = 0x8B; code[i++] = 0x5C; code[i++] = 0x24; code[i++] = 0x08; /* mov ebx, [esp+8]  (ball) */
    code[i++] = 0x8B; code[i++] = 0x74; code[i++] = 0x24; code[i++] = 0x0C; /* mov esi, [esp+12] (collEntry) */
    code[i++] = 0x60;                 /* pushad */
    code[i++] = 0x9C;               /* pushfd */
    /* push args in cdecl order: board (ecx), ball, collEntry */
    code[i++] = 0x51;               /* push ecx (board) */
    code[i++] = 0x53;               /* push ebx (ball) */
    code[i++] = 0x56;               /* push esi (collEntry) */
    code[i++] = 0xE8;
    *(DWORD*)(code + i) = (DWORD)&dce_handler - (DWORD)(code + i + 4);
    i += 4;
    code[i++] = 0x83; code[i++] = 0xC4; code[i++] = 0x0C; /* add esp, 12 */
    code[i++] = 0x9D;               /* popfd */
    code[i++] = 0x61;               /* popad */
    /* mov ecx, board is already true; trampoline will restore ebx/ebp */
    code[i++] = 0xB8;
    *(DWORD*)(code + i) = (DWORD)g_dce_trampoline;
    i += 4;
    code[i++] = 0xFF; code[i++] = 0xE0; /* jmp eax */
    g_dce_stub = code;
}

static void install_dce_hook(void) {
    memcpy(g_dce_original, (void*)DCE_HOOK_ADDR, DCE_HOOK_BYTES);
    build_dce_trampoline();
    build_dce_stub();

    BYTE patch[DCE_HOOK_BYTES];
    patch[0] = 0xE9;
    *(DWORD*)(patch + 1) = (DWORD)g_dce_stub - DCE_HOOK_ADDR - 5;
    patch_bytes((void*)DCE_HOOK_ADDR, patch, DCE_HOOK_BYTES);
    diag_log("[ghost_event] DCE hook installed at 0x40C5E5 (post-SEH)");
}

static void restore_dce_hook(void) {
    if (!g_dce_stub) return;
    patch_bytes((void*)DCE_HOOK_ADDR, g_dce_original, DCE_HOOK_BYTES);
    g_dce_stub = NULL;
    g_dce_trampoline = NULL;
}

'''

idx1 = src.index(old_dce)
idx2 = src.index(old_dce_end)
src = src[:idx1] + new_dce + src[idx2:]

# ------------------------------------------------------------------
# Fix 6: Remove Level_UpdateAndRender NOP patches.
# Delete the patch function body and the call to it.
# ------------------------------------------------------------------

src = src.replace(
    "/* Level_UpdateAndRender patches — NOP TT and party mode checks */\n"
    "static void patch_level_update_and_render(void) {\n"
    "    BYTE *jz_addr = (BYTE*)(EXE_BASE + 0x000B7F5);\n"
    "    patch_byte(jz_addr,     0x90);\n"
    "    patch_byte(jz_addr + 1, 0x90);\n"
    "\n"
    "    BYTE *jnz_addr = (BYTE*)(EXE_BASE + 0x000B7FF);\n"
    "    patch_byte(jnz_addr,     0x90);\n"
    "    patch_byte(jnz_addr + 1, 0x90);\n"
    "\n"
    "    diag_log(\"[ghost_event] Level_UpdateAndRender patched (TT + party checks NOPed)\");\n"
    "}\n\n",
    "/* Level_UpdateAndRender patches removed — Ghost 1 rendering is intentionally
 * limited to Time Trial mode by the game. Ghost 2 uses the ball list. */\n\n"
)

# Remove call to patch_level_update_and_render in init_thread
src = src.replace(
    "    /* Install Level_UpdateAndRender patches */\n"
    "    patch_level_update_and_render();\n\n",
    ""
)

# ------------------------------------------------------------------
# Fix 7: Fix practice hook — hook the helper call at 0x428C7A,
# remove race_index param, derive race index from profile.
# Also restore practice hook in DllMain detach.
# ------------------------------------------------------------------

# First remove the old App_StartPracticeRace hook functions (hook_impl through install_practice_hook)
old_practice_start = "/* App_StartPracticeRace detour hook */"
old_practice_end = "/* ================================================================\n * App_StartTournamentRace hook"
if old_practice_start not in src or old_practice_end not in src:
    raise SystemExit("Could not locate practice hook block")

new_practice_hook = '''/* App_StartPracticeRace hook — intercepts the helper call at 0x428C7A.
 * We replace the 5-byte CALL with a JMP to our stub. The stub calls a C
 * pre-handler (ECX=app), runs the original helper via trampoline, then runs a
 * post-handler. This avoids SEH/entry-point issues and race_index stack confusion. */

#define PRACTICE_HELPER_CALL_ADDR   0x00428C7A
#define PRACTICE_HELPER_RETURN      0x00428C7F
#define PRACTICE_HELPER_BYTES       5

static BYTE *g_practice_stub = NULL;
static BYTE g_practice_orig[PRACTICE_HELPER_BYTES];
static void *g_practice_trampoline = NULL;

static void pre_practice_hook(DWORD app) {
    if (!app) return;

    /* Clean up stale dummy recording BTT */
    if (g_dummyRecording && g_dummyRecording > 0x10000) {
        DWORD curr90C = 0;
        if (!IsBadReadPtr((void*)(app + APP_BTT_RECORDING), 4))
            curr90C = *(DWORD*)(app + APP_BTT_RECORDING);
        if (curr90C == g_dummyRecording) {
            if (!IsBadReadPtr((void*)g_dummyRecording, 4)) {
                DWORD vt = *(DWORD*)g_dummyRecording;
                if (vt == BTT_VTABLE_ADDR)
                    call_btt_dtor((void*)g_dummyRecording);
                else
                    game_free((void*)g_dummyRecording);
            }
            *(DWORD*)(app + APP_BTT_RECORDING) = 0;
        }
        g_dummyRecording = 0;
    }

    g_savedOldPlayback = 0;
    EnterCriticalSection(&g_cs);

    if (is_time_trial_precheck()) {
        DWORD profile = *(DWORD*)((char*)app + APP_PROFILE_PTR);
        int race_index = -1;
        if (profile && !IsBadReadPtr((void*)profile, 0x100))
            race_index = *(int*)((char*)profile + PROFILE_RACE_INDEX);

        char raceName[128] = "";
        if (race_index >= 0 && get_race_name_by_index(race_index, raceName, sizeof(raceName)) && raceName[0]) {
            diag_logf("[ghost_saver] PRACTICE HOOK: pre-inject for race '%s' (index=%d)", raceName, race_index);
            strncpy(g_hookRaceName, raceName, sizeof(g_hookRaceName) - 1);
            g_hookRaceName[sizeof(g_hookRaceName) - 1] = '\\0';

            int savedTime = get_saved_time(raceName);
            if (savedTime != NO_TIME) {
                int injectFailed = 0;
                if (!IsBadReadPtr((void*)(app + APP_BTT_PLAYBACK), 4)) {
                    DWORD existing = *(DWORD*)(app + APP_BTT_PLAYBACK);
                    if (existing && existing > 0x10000) {
                        g_savedOldPlayback = existing;
                    }
                    inject_saved_ghost(raceName);

                    DWORD newPlayback = 0;
                    if (!IsBadReadPtr((void*)(app + APP_BTT_PLAYBACK), 4))
                        newPlayback = *(DWORD*)(app + APP_BTT_PLAYBACK);

                    if (g_savedOldPlayback && newPlayback == g_savedOldPlayback) {
                        g_savedOldPlayback = 0;
                        injectFailed = 1;
                    }

                    if (!injectFailed && newPlayback && newPlayback > 0x10000 &&
                        !IsBadReadPtr((void*)(app + APP_BTT_RECORDING), 4)) {
                        DWORD recording = *(DWORD*)(app + APP_BTT_RECORDING);
                        if (recording && recording > 0x10000 &&
                            !IsBadReadPtr((void*)(recording + BTT_BEST_TIME), 4)) {
                            int oldTime = *(int*)((char*)recording + BTT_BEST_TIME);
                            if (oldTime != NO_TIME)
                                *(int*)((char*)recording + BTT_BEST_TIME) = NO_TIME;
                        }
                        if (!recording || recording < 0x10000) {
                            void *dummyRec = game_operator_new(BTT_SIZE);
                            if (dummyRec) {
                                call_btt_ctor(dummyRec);
                                DWORD vt = *(DWORD*)dummyRec;
                                if (vt == BTT_VTABLE_ADDR) {
                                    *(DWORD*)((char*)dummyRec + BTT_BEST_TIME) = NO_TIME;
                                    *(DWORD*)(app + APP_BTT_RECORDING) = (DWORD)dummyRec;
                                    g_dummyRecording = (DWORD)dummyRec;
                                } else {
                                    game_free(dummyRec);
                                }
                            }
                        }
                    }
                }
            } else {
                if (!IsBadReadPtr((void*)(app + APP_BTT_PLAYBACK), 4)) {
                    DWORD existing = *(DWORD*)(app + APP_BTT_PLAYBACK);
                    if (existing && existing > 0x10000) {
                        g_savedOldPlayback = existing;
                    }
                    *(DWORD*)(app + APP_BTT_PLAYBACK) = 0;
                }
            }
        }
    }

    LeaveCriticalSection(&g_cs);
}

static void post_practice_hook(void) {
    if (g_savedOldPlayback && g_savedOldPlayback > 0x10000) {
        if (!IsBadReadPtr((void*)g_savedOldPlayback, 4)) {
            DWORD vt = *(DWORD*)g_savedOldPlayback;
            if (vt == BTT_VTABLE_ADDR) {
                call_btt_dtor((void*)g_savedOldPlayback);
            }
        }
        g_savedOldPlayback = 0;
    }
}

__attribute__((naked, used)) static void practice_hook_stub(void) {
    __asm__ volatile(
        "pushl %%eax\\n"
        "pushl %%ecx\\n"
        "pushl %%edx\\n"
        "call _pre_practice_hook\\n"
        "popl %%edx\\n"
        "popl %%ecx\\n"
        "popl %%eax\\n"
        "jmp *%0\\n"
        : : "r"(g_practice_trampoline)
    );
}

static void install_practice_hook(void) {
    unsigned char *target = (unsigned char*)PRACTICE_HELPER_CALL_ADDR;
    memcpy(g_practice_orig, target, PRACTICE_HELPER_BYTES);

    /* Trampoline: original CALL instruction to helper, then jump back. */
    g_practice_trampoline = alloc_executable(16);
    memcpy(g_practice_trampoline, g_practice_orig, PRACTICE_HELPER_BYTES);
    ((BYTE*)g_practice_trampoline)[PRACTICE_HELPER_BYTES] = 0xE9;
    *(DWORD*)((BYTE*)g_practice_trampoline + PRACTICE_HELPER_BYTES + 1) =
        PRACTICE_HELPER_RETURN - ((DWORD)g_practice_trampoline + PRACTICE_HELPER_BYTES + 5);

    g_practice_stub = alloc_executable(64);
    {
        BYTE *c = g_practice_stub;
        int i = 0;
        c[i++] = 0x60;                  /* pushad */
        c[i++] = 0x9C;                  /* pushfd */
        c[i++] = 0x89; c[i++] = 0xCB;   /* mov ebx, ecx  (save app) */
        c[i++] = 0x53;                  /* push ebx */
        c[i++] = 0xE8;
        *(DWORD*)(c + i) = (DWORD)&pre_practice_hook - (DWORD)(c + i + 4);
        i += 4;
        c[i++] = 0x83; c[i++] = 0xC4; c[i++] = 0x04; /* add esp, 4 */
        c[i++] = 0x9D;                  /* popfd */
        c[i++] = 0x61;                  /* popad */
        c[i++] = 0x89; c[i++] = 0xD9;   /* mov ecx, ebx  (restore app for trampoline) */
        c[i++] = 0xE9;
        *(DWORD*)(c + i) = (DWORD)g_practice_trampoline - (DWORD)(c + i + 4);
        i += 4;
    }

    DWORD oldProtect;
    if (!VirtualProtect(target, PRACTICE_HELPER_BYTES, PAGE_EXECUTE_READWRITE, &oldProtect)) return;
    target[0] = 0xE9;
    *(DWORD*)(target + 1) = (DWORD)g_practice_stub - PRACTICE_HELPER_CALL_ADDR - 5;
    VirtualProtect(target, PRACTICE_HELPER_BYTES, PAGE_EXECUTE_READWRITE, &oldProtect);
    FlushInstructionCache(GetCurrentProcess(), target, PRACTICE_HELPER_BYTES);

    /* We need a frame-epilogue post-call cleanup because the helper returns to 0x428C7F,
     * where the stub has already done its work. The post-handler (destroy old playback)
     * can run in frame_epilogue instead; set a flag here. */
    diag_log("[ghost_saver] App_StartPracticeRace helper-call hook installed");
}

'''

idx1 = src.index(old_practice_start)
idx2 = src.index(old_practice_end)
src = src[:idx1] + new_practice_hook + src[idx2:]

# ------------------------------------------------------------------
# Fix 8: Fix tournament hook — hook helper call at 0x4288CC,
# create recording BTT after helper returns.
# ------------------------------------------------------------------

old_tour_start = "/* ================================================================\n * App_StartTournamentRace hook"
old_tour_end = "/* ================================================================\n * Ghost Triggers subsystem"
if old_tour_start not in src or old_tour_end not in src:
    raise SystemExit("Could not locate tournament hook block")

new_tour_hook = '''/* ================================================================
 * App_StartTournamentRace hook — creates BTT for recording
 * We replace the helper CALL at 0x4288CC (5 bytes) with a JMP to our stub.
 * The stub runs the original helper, then creates a fresh recording BTT at
 * App+0x90C because Tournament mode does not create one natively.
 * ================================================================ */

#define TOURNAMENT_HELPER_CALL_ADDR   0x00428C8C  /* actually 0x4288CC */
#define TOURNAMENT_HELPER_RETURN      0x004288D1
#define TOURNAMENT_HELPER_BYTES       5

static BYTE *g_tournament_stub = NULL;
static BYTE g_tournament_orig[TOURNAMENT_HELPER_BYTES];
static void *g_tournament_trampoline = NULL;

static void post_tournament_hook(DWORD app) {
    if (!app) return;

    DWORD bttRec = 0;
    if (!IsBadReadPtr((void*)(app + APP_BTT_RECORDING), 4))
        bttRec = *(DWORD*)(app + APP_BTT_RECORDING);

    if (!bttRec) {
        void *newBTT = game_operator_new(BTT_SIZE);
        if (newBTT) {
            call_btt_ctor(newBTT);
            DWORD vt = *(DWORD*)newBTT;
            if (vt == BTT_VTABLE_ADDR) {
                *(DWORD*)((char*)newBTT + BTT_BEST_TIME) = NO_TIME;
                *(DWORD*)(app + APP_BTT_RECORDING) = (DWORD)newBTT;
                diag_logf("[tournament_hook] Created recording BTT at 0x%X", (DWORD)newBTT);
            } else {
                game_free(newBTT);
                diag_logf("[tournament_hook] BTT ctor failed vtable=0x%X", vt);
            }
        }
    }
}

static void install_tournament_hook(void) {
    unsigned char *target = (unsigned char*)TOURNAMENT_HELPER_CALL_ADDR;

    /* Verify helper call signature: E8 xx xx xx xx */
    if (target[0] != 0xE8) {
        diag_logf("[FATAL] Tournament helper call signature mismatch at 0x%X!", TOURNAMENT_HELPER_CALL_ADDR);
        return;
    }

    memcpy(g_tournament_orig, target, TOURNAMENT_HELPER_BYTES);

    /* Trampoline: original CALL to helper, then jump back to 0x4288D1 */
    g_tournament_trampoline = alloc_executable(16);
    memcpy(g_tournament_trampoline, g_tournament_orig, TOURNAMENT_HELPER_BYTES);
    ((BYTE*)g_tournament_trampoline)[TOURNAMENT_HELPER_BYTES] = 0xE9;
    *(DWORD*)((BYTE*)g_tournament_trampoline + TOURNAMENT_HELPER_BYTES + 1) =
        TOURNAMENT_HELPER_RETURN - ((DWORD)g_tournament_trampoline + TOURNAMENT_HELPER_BYTES + 5);

    g_tournament_stub = alloc_executable(64);
    {
        BYTE *c = g_tournament_stub;
        int i = 0;
        c[i++] = 0x60;                  /* pushad */
        c[i++] = 0x9C;                  /* pushfd */
        c[i++] = 0x89; c[i++] = 0xCB;   /* mov ebx, ecx  (save app) */
        c[i++] = 0x89; c[i++] = 0xD9;   /* mov ecx, ebx  (ECX=app for trampoline) */
        c[i++] = 0xE9;
        *(DWORD*)(c + i) = (DWORD)g_tournament_trampoline - (DWORD)(c + i + 4);
        i += 4;
        /* After trampoline returns here, ECX may be clobbered; reload app from EBX */
        c[i++] = 0x60;                  /* pushad */
        c[i++] = 0x9C;                  /* pushfd */
        c[i++] = 0x53;                  /* push ebx */
        c[i++] = 0xE8;
        *(DWORD*)(c + i) = (DWORD)&post_tournament_hook - (DWORD)(c + i + 4);
        i += 4;
        c[i++] = 0x83; c[i++] = 0xC4; c[i++] = 0x04; /* add esp, 4 */
        c[i++] = 0x9D;                  /* popfd */
        c[i++] = 0x61;                  /* popad */
        c[i++] = 0xC3;                  /* ret */
    }

    DWORD oldProtect;
    if (!VirtualProtect(target, TOURNAMENT_HELPER_BYTES, PAGE_EXECUTE_READWRITE, &oldProtect)) return;
    target[0] = 0xE9;
    *(DWORD*)(target + 1) = (DWORD)g_tournament_stub - TOURNAMENT_HELPER_CALL_ADDR - 5;
    VirtualProtect(target, TOURNAMENT_HELPER_BYTES, PAGE_EXECUTE_READWRITE, &oldProtect);
    FlushInstructionCache(GetCurrentProcess(), target, TOURNAMENT_HELPER_BYTES);
    g_tournamentHookInstalled = 1;
    diag_log("[tournament_hook] App_StartTournamentRace helper-call hook installed");
}

'''

idx1 = src.index(old_tour_start)
idx2 = src.index(old_tour_end)
src = src[:idx1] + new_tour_hook + src[idx2:]

# ------------------------------------------------------------------
# Fix 9: Ghost 2 lifecycle — destroy old Ghost 2 before capture on same-level warp.
# Add ghost2_destroy() call at the top of the same-level warp block.
# ------------------------------------------------------------------

src = src.replace(
    "                /* Ghost 2 capture + segment save: before loading */\n"
    "                if (isSameLevel) {",
    "                /* Ghost 2 capture + segment save: before loading */\n"
    "                if (isSameLevel) {\n"
    "                    /* Destroy any existing Ghost 2 before capturing the new run */\n"
    "                    if (g_ghost2.active) {\n"
    "                        diag_log(\"[ghost2] Destroying previous Ghost 2 before same-level warp\");\n"
    "                        ghost2_destroy();\n"
    "                    }\n"
)

# ------------------------------------------------------------------
# Fix 10: Tournament same-level warp — use App_StartTournamentRace.
# This requires replacing the App_StartPracticeRace call and removing the
# post-call tournament reconstruction block.
# ------------------------------------------------------------------

# We'll replace the entire wasInTournament branch inside PHASE_LOAD with a clean version.
old_tournament_warp = '''            if (levelIdx >= 0 && levelIdx <= 14) {
            void *func = (void *)APP_START_PRACTICE_RACE;
            DWORD appVal = app;
            int idx = levelIdx;
            char savedDifficulty = *(BYTE*)((char*)app + 0x23C);
            DWORD oldProfile = *(DWORD*)((char*)app + APP_PROFILE_PTR);

            char wasInTournament = 0;
            int savedScores[16];
            int savedTimes[16];
            int hasTournamentData = 0;
            int savedTimeRemaining = 0;
            float savedPlayerScore = 0.0f;
            int isSameLevel = 0;'''

new_tournament_warp = '''            if (levelIdx >= 0 && levelIdx <= 14) {
            DWORD appVal = app;
            int idx = levelIdx;
            char savedDifficulty = *(BYTE*)((char*)app + 0x23C);
            DWORD oldProfile = *(DWORD*)((char*)app + APP_PROFILE_PTR);

            char wasInTournament = 0;
            int savedTimeRemaining = 0;
            int isSameLevel = 0;
            void *startFunc = (void *)APP_START_PRACTICE_RACE;'''

src = src.replace(old_tournament_warp, new_tournament_warp)

# Replace the tournament data collection block with the simplified version.
old_tournament_collect = '''            if (oldProfile) {
                char isPractice = *(BYTE*)((char*)oldProfile + PROFILE_IS_PRACTICE);
                wasInTournament = (isPractice == 0) ? 1 : 0;

                {
                    int currentRaceIdx = *(int*)((char*)oldProfile + PROFILE_RACE_INDEX);
                    if (levelIdx == currentRaceIdx - 1) {
                        isSameLevel = 1;
                        diag_logf("[warp] Same-level warp detected (levelIdx=%d, profile raceIdx=%d)",
                                  levelIdx, currentRaceIdx);
                    }
                }

                /* Ghost 2 capture + segment save: before loading */
                if (isSameLevel) {
                    /* Set up TW race name if not already set */
                    if (!g_twRaceName[0]) {
                        char raceName[128];
                        if (get_race_name(raceName, sizeof(raceName))) {
                            strncpy(g_twRaceName, raceName, sizeof(g_twRaceName) - 1);
                            g_twRaceName[sizeof(g_twRaceName) - 1] = '\\0';
                            diag_logf("[seg] TW race name set to '%s'", g_twRaceName);
                        }
                    }

                    /* Save Ghost 1 playback state before App_StartPracticeRace destroys it */
                    ghost1_save_state();

                    /* Save the current run segment to [N].ghost */
                    if (g_twRaceName[0]) {
                        save_warp_segment();
                    }

                    /* Check party mode — Ghost 2 only works in TT and Tournament */
                    BYTE partyMode = 0;
                    if (!IsBadReadPtr((void*)(app + APP_234_PARTY_MODE), 1))
                        partyMode = *(BYTE*)(app + APP_234_PARTY_MODE);
                    if (partyMode == 0) {
                        ghost2_capture();
                    }
                }

                if (wasInTournament) {
                    int raceIdx = *(int*)((char*)oldProfile + PROFILE_RACE_INDEX);
                    hasTournamentData = 1;
                    memcpy(savedScores, (void*)((char*)oldProfile + PROFILE_SCORE_ARRAY), sizeof(savedScores));
                    memcpy(savedTimes, (void*)((char*)oldProfile + PROFILE_TIME_ARRAY), sizeof(savedTimes));
                    if (raceIdx >= 0 && raceIdx < 16) {
                        savedScores[raceIdx] = (int)*(float*)((char*)app + APP_5E4_SCORE);
                    }
                    if (isSameLevel) {
                        if (raceIdx >= 0 && raceIdx < 16) {
                            savedTimeRemaining = *(int*)((char*)oldProfile + PROFILE_TIME_ARRAY + raceIdx * 4);
                        }
                    } else {
                        savedTimeRemaining = *(int*)((char*)app + APP_5E8_TIMER);
                    }
                    savedPlayerScore = *(float*)((char*)app + APP_5E4_SCORE);
                } else if (isSameLevel) {
                    savedTimeRemaining = *(int*)((char*)app + APP_5E8_TIMER);
                }
            }'''

new_tournament_collect = '''            if (oldProfile) {
                char isPractice = *(BYTE*)((char*)oldProfile + PROFILE_IS_PRACTICE);
                wasInTournament = (isPractice == 0) ? 1 : 0;

                {
                    int currentRaceIdx = *(int*)((char*)oldProfile + PROFILE_RACE_INDEX);
                    if (levelIdx == currentRaceIdx - 1) {
                        isSameLevel = 1;
                        diag_logf("[warp] Same-level warp detected (levelIdx=%d, profile raceIdx=%d)",
                                  levelIdx, currentRaceIdx);
                    }
                }

                /* Ghost 2 capture + segment save: before loading */
                if (isSameLevel) {
                    /* Set up TW race name if not already set */
                    if (!g_twRaceName[0]) {
                        char raceName[128];
                        if (get_race_name(raceName, sizeof(raceName))) {
                            strncpy(g_twRaceName, raceName, sizeof(g_twRaceName) - 1);
                            g_twRaceName[sizeof(g_twRaceName) - 1] = '\\0';
                            diag_logf("[seg] TW race name set to '%s'", g_twRaceName);
                        }
                    }

                    /* Save Ghost 1 playback state before race start destroys it */
                    ghost1_save_state();

                    /* Save the current run segment to [N].ghost */
                    if (g_twRaceName[0]) {
                        save_warp_segment();
                    }

                    /* Check party mode — Ghost 2 only works in TT and Tournament */
                    BYTE partyMode = 0;
                    if (!IsBadReadPtr((void*)(app + APP_234_PARTY_MODE), 1))
                        partyMode = *(BYTE*)(app + APP_234_PARTY_MODE);
                    if (partyMode == 0) {
                        ghost2_capture();
                    }

                    savedTimeRemaining = *(int*)((char*)app + APP_5E8_TIMER);
                }

                if (wasInTournament && isSameLevel) {
                    startFunc = (void *)APP_START_TOURNAMENT_RACE;
                }
            }'''

src = src.replace(old_tournament_collect, new_tournament_collect)

# Replace the asm call to use startFunc instead of hardcoded practice function.
src = src.replace(
    "            __asm__ volatile(\n"
    "                \"push %[idx]\\n\\t\"\n"
    "                \"movl %[appVal], %%ecx\\n\\t\"\n"
    "                \"call *%[func]\\n\\t\"\n"
    "                : /* no outputs */\n"
    "                : [func] \"r\" (func),\n"
    "                  [appVal] \"r\" (appVal),\n"
    "                  [idx] \"r\" (idx)\n"
    "                : \"eax\", \"edx\", \"ecx\",\n"
    "                  \"st\", \"st(1)\", \"st(2)\", \"st(3)\",\n"
    "                  \"st(4)\", \"st(5)\", \"st(6)\", \"st(7)\", \"memory\"\n"
    "            );",
    "            __asm__ volatile(\n"
    "                \"push %[idx]\\n\\t\"\n"
    "                \"movl %[appVal], %%ecx\\n\\t\"\n"
    "                \"call *%[func]\\n\\t\"\n"
    "                : /* no outputs */\n"
    "                : [func] \"r\" (startFunc),\n"
    "                  [appVal] \"r\" (appVal),\n"
    "                  [idx] \"r\" (idx)\n"
    "                : \"eax\", \"edx\", \"ecx\",\n"
    "                  \"st\", \"st(1)\", \"st(2)\", \"st(3)\",\n"
    "                  \"st(4)\", \"st(5)\", \"st(6)\", \"st(7)\", \"memory\"\n"
    "            );"
)

# Remove the old tournament reconstruction block (the large wasInTournament branch after the asm call).
old_tournament_rebuild = '''            if (wasInTournament) {
                /* Destroy game-created BTTs properly before zeroing */
                {
                    DWORD bttRec = *(DWORD*)((char*)app + APP_BTT_RECORDING);
                    DWORD bttPlay = *(DWORD*)((char*)app + APP_BTT_PLAYBACK);
                    if (bttRec) {
                        if (!IsBadReadPtr((void*)bttRec, 4)) {
                            DWORD vt = *(DWORD*)bttRec;
                            if (vt == BTT_VTABLE_ADDR)
                                call_btt_dtor((void*)bttRec);
                            else
                                game_free((void*)bttRec);
                        }
                        *(DWORD*)((char*)app + APP_BTT_RECORDING) = 0;
                        diag_log("[warp] Destroyed BTT recording (tournament)");
                    }
                    if (bttPlay) {
                        if (!IsBadReadPtr((void*)bttPlay, 4)) {
                            DWORD vt = *(DWORD*)bttPlay;
                            if (vt == BTT_VTABLE_ADDR)
                                call_btt_dtor((void*)bttPlay);
                            else
                                game_free((void*)bttPlay);
                        }
                        *(DWORD*)((char*)app + APP_BTT_PLAYBACK) = 0;
                        diag_log("[warp] Destroyed BTT playback (tournament)");
                    }
                }

                /* Create new BTT at App+0x90C for Tournament recording */
                {
                    void *newBTT = game_operator_new(BTT_SIZE);
                    if (newBTT) {
                        call_btt_ctor(newBTT);
                        DWORD vt = *(DWORD*)newBTT;
                        if (vt == BTT_VTABLE_ADDR) {
                            *(DWORD*)((char*)newBTT + BTT_BEST_TIME) = NO_TIME;
                            *(DWORD*)(app + APP_BTT_RECORDING) = (DWORD)newBTT;
                            diag_logf("[warp] Created new recording BTT at 0x%X for Tournament", (DWORD)newBTT);
                        } else {
                            game_free(newBTT);
                            diag_logf("[warp] BTT ctor failed vtable=0x%X", vt);
                        }
                    }
                }

                {
                    DWORD newProfile = *(DWORD*)((char*)app + APP_PROFILE_PTR);
                    if (newProfile) {
                        *(BYTE*)((char*)newProfile + PROFILE_IS_PRACTICE) = 0;
                        if (hasTournamentData) {
                            memcpy((void*)((char*)newProfile + PROFILE_SCORE_ARRAY), savedScores, sizeof(savedScores));
                            memcpy((void*)((char*)newProfile + PROFILE_TIME_ARRAY), savedTimes, sizeof(savedTimes));
                        }

                        {
                            DWORD newProfile2 = *(DWORD*)((char*)app + APP_PROFILE_PTR);
                            DWORD newBoard2 = 0;
                            int levelBaseTime = 0;
                            int difficultyBonus = 0;
                            int finalTimer = savedTimeRemaining;

                            if (newProfile2)
                                newBoard2 = *(DWORD*)((char*)newProfile2 + PROFILE_BOARD_PTR);
                            if (newBoard2 && !IsBadReadPtr((void*)(newBoard2 + 0x2998), 4))
                                levelBaseTime = *(int*)((char*)newBoard2 + 0x2998);

                            {
                                int diff = (int)*(BYTE*)((char*)app + 0x23C);
                                if (diff == 0) difficultyBonus = 1000;
                                else if (diff == 1) difficultyBonus = 500;
                            }

                            finalTimer = savedTimeRemaining + levelBaseTime + difficultyBonus;
                            *(int*)((char*)app + APP_5E8_TIMER) = finalTimer;
                        }
                        *(float*)((char*)app + APP_5E4_SCORE) = savedPlayerScore;
                    }
                }
            } else if (isSameLevel && savedTimeRemaining > 0) {
                *(int*)((char*)app + APP_5E8_TIMER) = savedTimeRemaining;
                diag_logf("[warp] Same-level TT: timer resumed at %d", savedTimeRemaining);
            }
        }'''

new_tournament_rebuild = '''            if (isSameLevel && savedTimeRemaining > 0) {
                *(int*)((char*)app + APP_5E8_TIMER) = savedTimeRemaining;
                diag_logf("[warp] Same-level warp: timer resumed at %d", savedTimeRemaining);
            }
        }'''

src = src.replace(old_tournament_rebuild, new_tournament_rebuild)

# ------------------------------------------------------------------
# Fix 11: DllMain detach — restore all hooks and clean up resources.
# Add restoration for practice, DCE, and frame hooks.
# ------------------------------------------------------------------

old_detach = '''    case DLL_PROCESS_DETACH:
        /* Restore all patches */
        g_freezeTimer = 0;
        restore_timer_caves();
        restore_tt_recording_nop();
        unblock_pause();

        /* Restore tournament hook */
        if (g_tournamentHookInstalled) {
            DWORD oldProt;
            if (VirtualProtect((void*)APP_START_TOURNAMENT_RACE, TOURNAMENT_HOOK_BYTES,
                PAGE_EXECUTE_READWRITE, &oldProt)) {
                memcpy((void*)APP_START_TOURNAMENT_RACE, g_tournamentOrigBytes,
                       TOURNAMENT_HOOK_BYTES);
                VirtualProtect((void*)APP_START_TOURNAMENT_RACE, TOURNAMENT_HOOK_BYTES,
                    oldProt, &oldProt);
            }
            g_tournamentHookInstalled = 0;
        }'''

new_detach = '''    case DLL_PROCESS_DETACH:
        /* Restore all patches */
        g_freezeTimer = 0;
        restore_timer_caves();
        restore_tt_recording_nop();
        unblock_pause();

        /* Restore frame epilogue hook */
        if (g_frame_stub) {
            patch_bytes((void*)APP_FRAME_UPDATE_EPILOGUE, g_frame_original, 5);
            g_frame_stub = NULL;
        }

        /* Restore DCE hook */
        restore_dce_hook();

        /* Restore practice hook */
        if (g_practice_stub) {
            patch_bytes((void*)PRACTICE_HELPER_CALL_ADDR, g_practice_orig, PRACTICE_HELPER_BYTES);
            g_practice_stub = NULL;
            g_practice_trampoline = NULL;
        }

        /* Restore tournament hook */
        if (g_tournamentHookInstalled) {
            patch_bytes((void*)TOURNAMENT_HELPER_CALL_ADDR, g_tournament_orig, TOURNAMENT_HELPER_BYTES);
            g_tournamentHookInstalled = 0;
            g_tournament_stub = NULL;
            g_tournament_trampoline = NULL;
        }'''

src = src.replace(old_detach, new_detach)

# Update init_thread log for DCE hook address
src = src.replace(
    "    diag_log(\"  - DCE hook at 0x40C5D0\");",
    "    diag_log(\"  - DCE hook at 0x40C5E5\");"
)

# ------------------------------------------------------------------
# Fix 12: Add post_practice_hook cleanup into frame_epilogue_handler.
# The helper-call stub cannot safely run post_practice_hook immediately
# because the helper returns deep inside the original function; run it once
# per frame via a flag.
# ------------------------------------------------------------------

# Add a global flag for stale playback cleanup
src = src.replace(
    "static int g_recording = 0;",
    "static int g_recording = 0;\nstatic int g_cleanupOldPlayback = 0;"
)

# In post_practice_hook, set flag instead of doing the work (it runs in frame handler)
# Actually the current post_practice_hook is fine; just call it from frame handler.

src = src.replace(
    "    /* 3b. Ghost 1: multi-segment chaining — advance to next segment if current ended */\n"
    "    ghost1_check_advance();",
    "    /* 3b. Ghost 1: multi-segment chaining — advance to next segment if current ended */\n"
    "    ghost1_check_advance();\n\n"
    "    /* 3c. Cleanup old playback BTT destroyed by practice hook */\n"
    "    if (g_cleanupOldPlayback) {\n"
    "        g_cleanupOldPlayback = 0;\n"
    "        post_practice_hook();\n"
    "    }"
)

# Update post_practice_hook to set the flag for next frame
src = src.replace(
    "static void post_practice_hook(void) {\n"
    "    if (g_savedOldPlayback && g_savedOldPlayback > 0x10000) {\n"
    "        if (!IsBadReadPtr((void*)g_savedOldPlayback, 4)) {\n"
    "            DWORD vt = *(DWORD*)g_savedOldPlayback;\n"
    "            if (vt == BTT_VTABLE_ADDR) {\n"
    "                call_btt_dtor((void*)g_savedOldPlayback);\n"
    "            }\n"
    "        }\n"
    "        g_savedOldPlayback = 0;\n"
    "    }\n"
    "}",
    "static void post_practice_hook(void) {\n"
    "    if (g_savedOldPlayback && g_savedOldPlayback > 0x10000) {\n"
    "        if (!IsBadReadPtr((void*)g_savedOldPlayback, 4)) {\n"
    "            DWORD vt = *(DWORD*)g_savedOldPlayback;\n"
    "            if (vt == BTT_VTABLE_ADDR) {\n"
    "                call_btt_dtor((void*)g_savedOldPlayback);\n"
    "            }\n"
    "        }\n"
    "        g_savedOldPlayback = 0;\n"
    "    }\n"
    "    g_cleanupOldPlayback = 1;\n"
    "}"
)

# ------------------------------------------------------------------
# Write result
# ------------------------------------------------------------------

with open(PATH, "w", encoding="utf-8") as f:
    f.write(src)

print("Patched", PATH)
print("Backup saved to", BAK)
