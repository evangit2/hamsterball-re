/*
 * mods_first.c - BASS.dll proxy mod that checks mods/ before loading files.
 *
 * When the game tries to open any file (levels, textures, sounds, meshes),
 * this mod first checks if a replacement exists at mods/<original_path>.
 * If yes, it redirects there. Otherwise, it uses the original path.
 *
 * This lets you drop modified files into a mods/ folder alongside the game
 * without overwriting originals.
 *
 * HOW IT WORKS:
 *   Patches the game's IAT entries for CreateFileA and GetFileAttributesA.
 *   These are the two Win32 APIs the game uses for all file access.
 *   The hook prepends "mods\" to the path and tries that first; if the
 *   mods/ variant exists, the redirected path is used instead.
 *
 * SAFETY:
 *   - Does NOT modify CreateFileW or other wide-char variants
 *     (game is ASCII-only)
 *   - Only applies to EXISTING file opens (OPEN_EXISTING / OPEN_ALWAYS)
 *   - mods/ path is built on a per-call stack buffer
 *   - Logs all redirects to mods_first.log for debugging
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll mods_first.c \
 *     -lwinmm -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
 *     -Wl,--add-stdcall-alias
 *
 * Install:
 *   1. Rename original bass.dll -> bass_real.dll
 *   2. Copy this bass.dll into the game folder
 *   3. Create a mods/ folder next to the EXE
 *   4. Put override files in mods/ with same relative path,
 *      e.g. mods/levels/level1.MESHWORLD overrides Warm-Up
 *   5. Launch the game!
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

/* ================================================================
 * Logging
 * ================================================================ */

static char g_logPath[MAX_PATH] = "";

static void log_msg(const char *msg) {
    if (g_logPath[0] == '\0') return;
    HANDLE h = CreateFileA(g_logPath, FILE_APPEND_DATA,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h != INVALID_HANDLE_VALUE) {
        DWORD w;
        SetFilePointer(h, 0, NULL, FILE_END);
        WriteFile(h, msg, (DWORD)strlen(msg), &w, NULL);
        WriteFile(h, "\r\n", 2, &w, NULL);
        CloseHandle(h);
    }
}

static void log_fmt(const char *fmt, ...) {
    char buf[512];
    va_list args;
    if (g_logPath[0] == '\0') return;
    va_start(args, fmt);
    wvsprintfA(buf, fmt, args);
    va_end(args);
    log_msg(buf);
}

/* ================================================================
 * IAT Patching
 * ================================================================ */

/* Original function typedefs */
typedef HANDLE (__stdcall *CreateFileA_t)(LPCSTR, DWORD, DWORD,
    LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);
typedef DWORD  (__stdcall *GetFileAttributesA_t)(LPCSTR);

/* Saved original function pointers */
static CreateFileA_t        g_orig_CreateFileA = NULL;
static GetFileAttributesA_t g_orig_GetFileAttributesA = NULL;

#define MODS_PREFIX "mods\\"

/* Build "mods\<original>" path. Returns pointer to static buffer or NULL. */
static const char* build_mods_path(const char *original) {
    static char s_buf[MAX_PATH];
    size_t prefixLen = strlen(MODS_PREFIX);
    size_t origLen  = strlen(original);
    if (prefixLen + origLen + 1 > MAX_PATH)
        return NULL;
    memcpy(s_buf, MODS_PREFIX, prefixLen);
    memcpy(s_buf + prefixLen, original, origLen + 1);
    return s_buf;
}

/* Hooked CreateFileA */
static HANDLE __stdcall hook_CreateFileA(LPCSTR lpFileName,
    DWORD dwDesiredAccess, DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile)
{
    /* Only redirect opens of existing files */
    if (dwCreationDisposition == OPEN_EXISTING ||
        dwCreationDisposition == OPEN_ALWAYS) {
        const char *modsPath = build_mods_path(lpFileName);
        if (modsPath) {
            DWORD attr = g_orig_GetFileAttributesA(modsPath);
            if (attr != INVALID_FILE_ATTRIBUTES &&
                !(attr & FILE_ATTRIBUTE_DIRECTORY)) {
                log_fmt("REDIRECT: %s -> %s", lpFileName, modsPath);
                return g_orig_CreateFileA(modsPath, dwDesiredAccess,
                    dwShareMode, lpSecurityAttributes,
                    dwCreationDisposition, dwFlagsAndAttributes,
                    hTemplateFile);
            }
        }
    }
    return g_orig_CreateFileA(lpFileName, dwDesiredAccess,
        dwShareMode, lpSecurityAttributes,
        dwCreationDisposition, dwFlagsAndAttributes,
        hTemplateFile);
}

/* Hooked GetFileAttributesA */
static DWORD __stdcall hook_GetFileAttributesA(LPCSTR lpFileName) {
    const char *modsPath = build_mods_path(lpFileName);
    if (modsPath) {
        DWORD attr = g_orig_GetFileAttributesA(modsPath);
        if (attr != INVALID_FILE_ATTRIBUTES &&
            !(attr & FILE_ATTRIBUTE_DIRECTORY)) {
            log_fmt("ATTREDIR: %s -> %s", lpFileName, modsPath);
            return attr;
        }
    }
    return g_orig_GetFileAttributesA(lpFileName);
}

/* ================================================================
 * IAT Table Patching
 * ================================================================ */

static void patch_iat_table(void) {
    HMODULE hMod = GetModuleHandleA(NULL);
    if (!hMod) return;

    PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER)hMod;
    if (!dos || dos->e_magic != IMAGE_DOS_SIGNATURE) return;

    PIMAGE_NT_HEADERS nt = (PIMAGE_NT_HEADERS)((BYTE*)hMod + dos->e_lfanew);
    if (!nt || nt->Signature != IMAGE_NT_SIGNATURE) return;

    PIMAGE_IMPORT_DESCRIPTOR impDesc = (PIMAGE_IMPORT_DESCRIPTOR)(
        (BYTE*)hMod + nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

    for (; impDesc->Name; impDesc++) {
        const char *dllName = (const char*)((BYTE*)hMod + impDesc->Name);
        if (_stricmp(dllName, "KERNEL32.dll") != 0) continue;

        PIMAGE_THUNK_DATA thunkILT = (PIMAGE_THUNK_DATA)(
            (BYTE*)hMod + impDesc->OriginalFirstThunk);
        PIMAGE_THUNK_DATA thunkIAT = (PIMAGE_THUNK_DATA)(
            (BYTE*)hMod + impDesc->FirstThunk);

        for (int i = 0; thunkILT[i].u1.AddressOfData; i++) {
            if (IMAGE_SNAP_BY_ORDINAL(thunkILT[i].u1.AddressOfData))
                continue; /* Skip ordinal imports */

            PIMAGE_IMPORT_BY_NAME ibn = (PIMAGE_IMPORT_BY_NAME)(
                (BYTE*)hMod + thunkILT[i].u1.AddressOfData);
            const char *funcName = (const char*)ibn->Name;

            DWORD oldProt;
            VirtualProtect(&thunkIAT[i], sizeof(DWORD),
                PAGE_READWRITE, &oldProt);

            if (strcmp(funcName, "CreateFileA") == 0) {
                g_orig_CreateFileA = (CreateFileA_t)thunkIAT[i].u1.Function;
                thunkIAT[i].u1.Function = (DWORD_PTR)hook_CreateFileA;
                log_msg("PATCHED: CreateFileA");
            }
            else if (strcmp(funcName, "GetFileAttributesA") == 0) {
                g_orig_GetFileAttributesA = (GetFileAttributesA_t)thunkIAT[i].u1.Function;
                thunkIAT[i].u1.Function = (DWORD_PTR)hook_GetFileAttributesA;
                log_msg("PATCHED: GetFileAttributesA");
            }

            VirtualProtect(&thunkIAT[i], sizeof(DWORD),
                oldProt, &oldProt);
        }
        break;
    }
}

/* ================================================================
 * DllMain
 * ================================================================ */

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    (void)hinstDLL;
    (void)lpvReserved;

    if (fdwReason == DLL_PROCESS_ATTACH) {
        /* Set up log path next to the EXE */
        GetModuleFileNameA(NULL, g_logPath, MAX_PATH);
        char *p = strrchr(g_logPath, '\\');
        if (p) {
            strcpy(p + 1, "mods_first.log");
        } else {
            strcpy(g_logPath, "mods_first.log");
        }

        /* Install IAT hooks */
        patch_iat_table();
    }

    return TRUE;
}