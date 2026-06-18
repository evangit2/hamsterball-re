/*
 * Hamsterball Collision Hook Injector
 *
 * Usage: injector.exe [pid]
 *   If no PID given, automatically finds Hamsterball.exe.
 *
 * Loads collision_hook.dll into the target process.
 *
 * Build: i686-w64-mingw32-gcc -o injector.exe injector.c
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static char *stristr_impl(const char *haystack, const char *needle) {
    size_t hlen = strlen(haystack), nlen = strlen(needle);
    for (size_t i = 0; i + nlen <= hlen; i++) {
        size_t j;
        for (j = 0; j < nlen; j++) {
            if (tolower((unsigned char)haystack[i+j]) != tolower((unsigned char)needle[j]))
                break;
        }
        if (j == nlen) return (char *)(haystack + i);
    }
    return NULL;
}

static DWORD find_process(const char *name) {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) return 0;

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(pe);

    if (Process32First(snapshot, &pe)) {
        do {
            if (_stricmp(pe.szExeFile, name) == 0) {
                CloseHandle(snapshot);
                return pe.th32ProcessID;
            }
        } while (Process32Next(snapshot, &pe));
    }
    CloseHandle(snapshot);
    return 0;
}

int main(int argc, char *argv[]) {
    DWORD pid = 0;
    const char *dll_path = "collision_hook.dll";

    if (argc >= 2) {
        pid = atoi(argv[1]);
    }
    if (pid == 0) {
        pid = find_process("Hamsterball.exe");
        if (!pid) pid = find_process("HamsterBall.exe");
    }
    if (pid == 0) {
        /* Also try to find any process containing "hamster" */
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snapshot != INVALID_HANDLE_VALUE) {
            PROCESSENTRY32 pe;
            pe.dwSize = sizeof(pe);
            if (Process32First(snapshot, &pe)) {
                do {
                    if (stristr_impl(pe.szExeFile, "hamster")) {
                        pid = pe.th32ProcessID;
                        printf("Found: %s (PID %lu)\n", pe.szExeFile, pid);
                        break;
                    }
                } while (Process32Next(snapshot, &pe));
            }
            CloseHandle(snapshot);
        }
    }

    if (pid == 0) {
        fprintf(stderr, "Error: Could not find Hamsterball.exe. Usage: %s [pid]\n", argv[0]);
        return 1;
    }

    printf("Target PID: %lu\n", pid);

    /* Get full path to DLL */
    char full_path[MAX_PATH];
    GetFullPathNameA(dll_path, MAX_PATH, full_path, NULL);
    printf("DLL path: %s\n", full_path);

    /* Open process */
    HANDLE hProcess = OpenProcess(
        PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION |
        PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ,
        FALSE, pid);
    if (!hProcess) {
        fprintf(stderr, "Error: OpenProcess failed (error %lu). Run as Administrator.\n", GetLastError());
        return 1;
    }

    /* Allocate memory in target for DLL path */
    SIZE_T path_len = strlen(full_path) + 1;
    LPVOID pRemotePath = VirtualAllocEx(hProcess, NULL, path_len,
        MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!pRemotePath) {
        fprintf(stderr, "Error: VirtualAllocEx failed (error %lu)\n", GetLastError());
        CloseHandle(hProcess);
        return 1;
    }

    /* Write DLL path */
    if (!WriteProcessMemory(hProcess, pRemotePath, full_path, path_len, NULL)) {
        fprintf(stderr, "Error: WriteProcessMemory failed (error %lu)\n", GetLastError());
        VirtualFreeEx(hProcess, pRemotePath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return 1;
    }

    /* Get LoadLibraryA address */
    HMODULE hKernel32 = GetModuleHandleA("kernel32.dll");
    FARPROC pLoadLibrary = GetProcAddress(hKernel32, "LoadLibraryA");

    /* Create remote thread to load DLL */
    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0,
        (LPTHREAD_START_ROUTINE)pLoadLibrary, pRemotePath, 0, NULL);
    if (!hThread) {
        fprintf(stderr, "Error: CreateRemoteThread failed (error %lu)\n", GetLastError());
        VirtualFreeEx(hProcess, pRemotePath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return 1;
    }

    /* Wait for LoadLibrary to complete */
    WaitForSingleObject(hThread, 10000);

    /* Check result */
    DWORD exitCode = 0;
    GetExitCodeThread(hThread, &exitCode);
    if (exitCode != 0) {
        printf("DLL injected successfully! (module handle 0x%lX)\n", (unsigned long)exitCode);
        printf("Collision events will be logged to collision_log.csv\n");
    } else {
        fprintf(stderr, "Warning: LoadLibrary returned 0. DLL may have failed to load.\n");
        fprintf(stderr, "Make sure collision_hook.dll is in the same directory as injector.exe\n");
    }

    /* Cleanup */
    CloseHandle(hThread);
    VirtualFreeEx(hProcess, pRemotePath, 0, MEM_RELEASE);
    CloseHandle(hProcess);

    printf("\nPress Enter to exit...");
    getchar();

    return 0;
}
