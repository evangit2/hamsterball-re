/*
 * jump_mod.c — BASS.dll proxy that lets Player 1 jump with the spacebar.
 *
 * GROUND DETECTION: Raycast via Mesh_FindClosestCollision (0x00465D90).
 * A background thread probes downward from the ball center each ~10ms.
 * If geometry is within (radius + 2.0) units below, g_on_ground is set.
 * The code cave checks g_on_ground — NO cooldown timer.
 *
 * This means the player can jump again the instant they touch ground.
 *
 * HOOK LOCATION: Phase 15 of Ball_Update (0x00407BB4) — the Ball_ApplyForce
 * call site. The jump impulse is FADD'd to vel.y BEFORE position is finalized,
 * so the ball lifts off the ground in the SAME frame. This preserves horizontal
 * momentum: next frame the ball is airborne, so floor collision (type 5) won't
 * fire and zero the XZ velocity.
 *
 * ARCHITECTURE (Pattern 4: volatile flag + polling thread):
 *   Code cave (Ball_Update epilogue):
 *     - Stores ball pointer in g_ball_ptr (for the background thread)
 *     - Edge-detects spacebar press
 *     - Checks g_on_ground flag (set by background thread)
 *     - If all conditions met: writes jump velocity to ball+0x174
 *   Background thread (Sleep 10ms loop):
 *     - Reads g_ball_ptr → ball → Scene → CollisionLevel
 *     - Calls Mesh_FindClosestCollision(cl, &out, pos, {0,-1,0}, radius+0.5)
 *     - Sets g_on_ground = (|out.y - pos.y| <= radius + 2.0) ? 1 : 0
 *
 * Keyboard reading: Uses the game's own DirectInput8 buffer.
 *   App = *(DWORD*)0x005341E0
 *   InputHandler = *(DWORD*)(App + 0x180)
 *   KeyboardDevice = *(DWORD*)(InputHandler + 0x434)
 *   DIK_SPACE state = *(BYTE*)(KeyboardDevice + 0xC + 0x39)
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll jump_mod.c -lwinmm \
 *     -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc -Wl,--add-stdcall-alias
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * BASS Proxy Exports — forward all game imports to bass_real.dll
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
        if (hSelf && GetModuleFileNameA(hSelf, path, MAX_PATH)) {
            char *p = strrchr(path, '\\');
            if (p) { strcpy(p + 1, "bass_real.dll"); g_hRealBass = LoadLibraryA(path); }
        }
    }
    if (g_hRealBass) {
        real_BASS_ChannelSetAttributes = (BASS_ChannelSetAttributes_t)GetProcAddress(g_hRealBass, "BASS_ChannelSetAttributes");
        real_BASS_MusicPlayEx          = (BASS_MusicPlayEx_t)GetProcAddress(g_hRealBass, "BASS_MusicPlayEx");
        real_BASS_SetConfig             = (BASS_SetConfig_t)GetProcAddress(g_hRealBass, "BASS_SetConfig");
        real_BASS_Init                  = (BASS_Init_t)GetProcAddress(g_hRealBass, "BASS_Init");
        real_BASS_Free                  = (BASS_Free_t)GetProcAddress(g_hRealBass, "BASS_Free");
        real_BASS_Start                 = (BASS_Start_t)GetProcAddress(g_hRealBass, "BASS_Start");
        real_BASS_Stop                  = (BASS_Stop_t)GetProcAddress(g_hRealBass, "BASS_Stop");
        real_BASS_ErrorGetCode          = (BASS_ErrorGetCode_t)GetProcAddress(g_hRealBass, "BASS_ErrorGetCode");
        real_BASS_MusicLoad             = (BASS_MusicLoad_t)GetProcAddress(g_hRealBass, "BASS_MusicLoad");
        real_BASS_ChannelStop            = (BASS_ChannelStop_t)GetProcAddress(g_hRealBass, "BASS_ChannelStop");
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Jump Mod — Raycast Ground Detection + Binary Hook
 * ═══════════════════════════════════════════════════════════════════════════ */

#define G_APP_ADDR          0x005341E0
#define BALL_UPDATE_HOOK    0x00407BB4   /* MOV ECX,[ESP+0x1C]; MOV EDX,[ECX] — Phase 15 */
#define HOOK_ORIG_BYTES     6            /* 8B 4C 24 1C 8B 11 */
#define MESH_RAYCAST_ADDR   0x00465D90   /* Mesh_FindClosestCollision */

/* Ball struct offsets (byte) */
#define BALL_SCENE          0x014   /* Scene* */
#define BALL_PLAYER_IDX     0x018   /* int (player index, 0 = Player 1) */
#define BALL_POS_X          0x164   /* float (position X) */
#define BALL_POS_Y          0x168   /* float (position Y) */
#define BALL_POS_Z          0x16C   /* float (position Z) */
#define BALL_VEL_Y          0x174   /* accumulated force Y (impulse, zeroed each tick) */
#define BALL_RADIUS         0x284   /* float (collision radius) */
#define BALL_FALL_MODE      0xC4C   /* fall-off-level (death/respawn) state */

/* Scene struct offsets */
#define SCENE_COLLISION_LEVEL  0x8B0  /* CollisionLevel* */

/* App struct offsets */
#define APP_INPUT_HANDLER   0x180

/* InputHandler offsets */
#define IH_KEYBOARD_DEV    0x434

/* KeyboardDevice offsets */
#define KBD_KEY_BUFFER     0x00C
#define DIK_SPACE          0x039

/* Jump velocity: 500.0f = 0x43FA0000
 * Stored as a float constant in memory so the code cave can FADD it. */
static float g_jump_vel = 500.0f;

/* Ground check threshold: ball is "on ground" if floor is within radius+2.0 below */
#define GROUND_THRESHOLD   2.0f

/* ─── Raycast API ─── */

typedef struct { float x, y, z; } Vec3;

/* Mesh_FindClosestCollision — __thiscall
 *   ECX = collision_level (Scene+0x8B0)
 *   Stack: out*, origin(3 floats), direction(3 floats), max_dist(float)
 *   ret 0x20 (32 bytes = 8 DWORDs on stack)
 *
 * Direction is normalized internally then scaled to 99999, clamped to ~994.
 * max_dist = sphere radius for AABB broad-phase. Use ball_radius + 0.5f.
 *
 * No-hit behavior: returns endpoint ~994 units along direction (NOT origin).
 * Always check distance between origin and out after the call.
 */
typedef Vec3* (__thiscall *MeshRaycast_t)(
    void* collision_level,
    Vec3* out,
    Vec3 origin,
    Vec3 direction,
    float max_dist
);

static MeshRaycast_t pfn_raycast = (MeshRaycast_t)MESH_RAYCAST_ADDR;

/* ─── Shared state between code cave and background thread ─── */

/* Set by background thread, read by code cave */
static volatile DWORD g_on_ground = 0;

/* Set by code cave each frame (ball pointer in ESI), read by background thread */
static volatile DWORD g_ball_ptr = 0;

/* Background thread control */
static volatile DWORD g_bg_active = 1;
static HANDLE g_bg_thread = NULL;

/* Edge detection state (code cave only) */
static BYTE g_space_was_down = 0;

/* Debug counter */
static volatile DWORD g_jump_count = 0;

/* ═══════════════════════════════════════════════════════════════════════════
 * Background Thread — Raycast Ground Detection
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * Continuously probes downward from the ball position using the engine's
 * own collision raycast. Sets g_on_ground = 1 when geometry is close below.
 *
 * This thread calls Mesh_FindClosestCollision, which is safe because:
 * - Level collision geometry is static during gameplay
 * - The function creates a temp CollisionMesh per call (no shared state)
 * - We're NOT calling from inside Ball_Update (code cave rule)
 *
 * Sleep(10) gives ~100 checks/sec — about 1.5x per frame at 60fps.
 * Ground state staleness is at most 10ms (less than one frame).
 */

static DWORD WINAPI ground_check_thread(LPVOID param)
{
    (void)param;
    while (g_bg_active) {
        DWORD ball_val = g_ball_ptr;
        if (ball_val) {
            char *ball = (char*)ball_val;
            void* scene = *(void**)(ball + BALL_SCENE);
            if (scene) {
                void* cl = *(void**)((char*)scene + SCENE_COLLISION_LEVEL);
                if (cl) {
                    Vec3 pos;
                    pos.x = *(float*)(ball + BALL_POS_X);
                    pos.y = *(float*)(ball + BALL_POS_Y);
                    pos.z = *(float*)(ball + BALL_POS_Z);
                    float radius = *(float*)(ball + BALL_RADIUS);

                    Vec3 down   = { 0.0f, -1.0f, 0.0f };
                    Vec3 out    = { 0.0f, 0.0f, 0.0f };

                    /* Raycast downward: origin=ball center, direction=(0,-1,0)
                     * max_dist = radius + 0.5 (matches engine's own ground probes) */
                    pfn_raycast(cl, &out, pos, down, radius + 0.5f);

                    /* Check if hit point is within radius + 2.0 below the ball.
                     * On hit: out.y ≈ pos.y - radius (floor directly below)
                     * On miss: out.y ≈ pos.y - 994 (ray endpoint, far away) */
                    float dy = out.y - pos.y;
                    /* dy is negative (we cast downward). Use absolute value. */
                    if (dy < 0.0f) dy = -dy;
                    g_on_ground = (dy <= radius + GROUND_THRESHOLD) ? 1 : 0;
                }
            }
        }
        Sleep(10);
    }
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Code Cave — Hand-assembled x86 in Ball_Update Phase 15
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * Hook point: 0x00407BB4 — the Ball_ApplyForce call site in Phase 15.
 * At this point in Ball_Update, all roll physics are done and velocity
 * accumulators (0x170/0x174/0x178) contain the frame's forces. Ball_ApplyForce
 * is about to be called to accumulate these into position delta.
 *
 * By adding the jump impulse to vel.y HERE, the impulse enters the force
 * pipeline BEFORE position is finalized. The ball lifts off the ground in the
 * SAME frame, so next frame's floor collision (type 5) won't fire and zero
 * the horizontal velocity. This preserves horizontal momentum.
 *
 * At entry: ESI = ball pointer (this)
 *           ESP at the hook point in Ball_Update's Phase 15
 *
 * The cave:
 *   1.  Saves registers (EAX, EDI) — NOT ECX/EDX (those are set by the
 *       original instructions and must survive to the code after the hook)
 *   2.  Executes the original 6 bytes: MOV ECX,[ESP+0x24]; MOV EDX,[ECX]
 *       (ESP+0x24 = original ESP+0x1C + 0x08 from 2 PUSHes)
 *   3.  Stores ball pointer: g_ball_ptr = ESI (for background thread)
 *   4.  Checks ball+0x18 == 0 (Player 1)
 *   5.  Checks fall_mode == 0 (not dying)
 *   6.  Checks g_on_ground == 1 (raycast says floor is close)
 *   7.  Reads App → InputHandler(App+0x180) → KeyboardDevice(IH+0x434)
 *   8.  Reads DIK_SPACE from KeyboardDevice+0xC+0x39
 *   9.  Edge-detects key press (rising edge only)
 *   10. If all conditions met: ADDS jump impulse to ball+0x174 via FADD
 *   11. Restores registers
 *   12. JMP back to hook_addr + 6
 *
 * Ground detection: background thread raycasts downward and sets g_on_ground.
 * NO cooldown timer — the player can jump the instant they touch ground.
 */

static void install_hook(void)
{
    BYTE *hook_addr = (BYTE*)BALL_UPDATE_HOOK;

    BYTE *cave = (BYTE*)VirtualAlloc(NULL, 256, MEM_COMMIT | MEM_RESERVE,
                                     PAGE_EXECUTE_READWRITE);
    if (!cave) return;

    DWORD jmp_to_cave = (DWORD)(cave - hook_addr - 5);

    int p = 0;

    /* PUSH EAX, EDI — save ONLY registers we clobber.
     * Do NOT save/restore ECX or EDX: the original instructions set them,
     * and the code after the hook (CALL [EDX] at 0x407BBC) depends on their
     * values surviving. If we PUSH/POP them, we restore stale pre-hook values. */
    cave[p++] = 0x50;  /* PUSH EAX */
    cave[p++] = 0x57;  /* PUSH EDI */

    /* Execute original 6 bytes: MOV ECX,[ESP+0x1C+0x08]; MOV EDX,[ECX]
     * Original: 8B 4C 24 1C 8B 11
     * With 2 pushes: ESP is 0x08 lower, so offset becomes 0x24 */
    cave[p++] = 0x8B; cave[p++] = 0x4C; cave[p++] = 0x24;
    cave[p++] = 0x24;
    cave[p++] = 0x8B; cave[p++] = 0x11;

    /* --- Store ball pointer for background thread: MOV [g_ball_ptr], ESI --- */
    cave[p++] = 0x89; cave[p++] = 0x35;
    *(DWORD*)(cave + p) = (DWORD)&g_ball_ptr; p += 4;

    /* --- Check player_index == 0 (ball+0x18) --- */
    /* MOV EAX, [ESI+0x18] */
    cave[p++] = 0x8B; cave[p++] = 0x86;
    cave[p++] = 0x18; cave[p++] = 0x00; cave[p++] = 0x00; cave[p++] = 0x00;
    /* TEST EAX, EAX */
    cave[p++] = 0x85; cave[p++] = 0xC0;
    /* JNZ .done */
    cave[p++] = 0x75; cave[p++] = 0x00;
    int jnz_player = p - 1;

    /* --- Check fall_mode == 0 (ball+0xC4C, byte) --- */
    /* MOV AL, [ESI+0xC4C] */
    cave[p++] = 0x8A; cave[p++] = 0x86;
    cave[p++] = 0x4C; cave[p++] = 0x0C; cave[p++] = 0x00; cave[p++] = 0x00;
    /* TEST AL, AL */
    cave[p++] = 0x84; cave[p++] = 0xC0;
    /* JNZ .done */
    cave[p++] = 0x75; cave[p++] = 0x00;
    int jnz_fallmode = p - 1;

    /* --- Check g_on_ground == 1 (raycast ground detection) --- */
    /* MOV EAX, [g_on_ground] */
    cave[p++] = 0xA1;
    *(DWORD*)(cave + p) = (DWORD)&g_on_ground; p += 4;
    /* TEST EAX, EAX */
    cave[p++] = 0x85; cave[p++] = 0xC0;
    /* JZ .done (not on ground, can't jump) */
    cave[p++] = 0x74; cave[p++] = 0x00;
    int jz_not_grounded = p - 1;

    /* --- Read App pointer: MOV EAX, [0x005341E0] --- */
    cave[p++] = 0xA1;
    *(DWORD*)(cave + p) = G_APP_ADDR; p += 4;
    /* TEST EAX, EAX */
    cave[p++] = 0x85; cave[p++] = 0xC0;
    /* JZ .done */
    cave[p++] = 0x74; cave[p++] = 0x00;
    int jz_app = p - 1;

    /* --- InputHandler = [EAX+0x180]: MOV EDI, [EAX+0x180] --- */
    cave[p++] = 0x8B; cave[p++] = 0xB8;
    *(DWORD*)(cave + p) = APP_INPUT_HANDLER; p += 4;
    /* TEST EDI, EDI */
    cave[p++] = 0x85; cave[p++] = 0xFF;
    /* JZ .done */
    cave[p++] = 0x74; cave[p++] = 0x00;
    int jz_ih = p - 1;

    /* --- KeyboardDevice = [EDI+0x434]: MOV EDI, [EDI+0x434] --- */
    cave[p++] = 0x8B; cave[p++] = 0xBF;
    *(DWORD*)(cave + p) = IH_KEYBOARD_DEV; p += 4;
    /* TEST EDI, EDI */
    cave[p++] = 0x85; cave[p++] = 0xFF;
    /* JZ .done */
    cave[p++] = 0x74; cave[p++] = 0x00;
    int jz_kbd = p - 1;

    /* --- Read DIK_SPACE: MOV AL, [EDI+0x45] (0xC+0x39=0x45) --- */
    cave[p++] = 0x8A; cave[p++] = 0x87;
    *(DWORD*)(cave + p) = (KBD_KEY_BUFFER + DIK_SPACE); p += 4;
    /* TEST AL, 0x80 */
    cave[p++] = 0xA8; cave[p++] = 0x80;
    /* JZ .not_pressed */
    cave[p++] = 0x74; cave[p++] = 0x00;
    int jz_notpressed = p - 1;

    /* --- Key IS pressed: edge detect --- */
    /* CMP byte ptr [g_space_was_down], 0 */
    cave[p++] = 0x80; cave[p++] = 0x3D;
    *(DWORD*)(cave + p) = (DWORD)&g_space_was_down; p += 4;
    cave[p++] = 0x00;
    /* JNE .done (already pressed, skip) */
    cave[p++] = 0x75; cave[p++] = 0x00;
    int jne_already = p - 1;

    /* --- Rising edge! ADD jump impulse to vel.y (preserves vel.x/vel.z) ---
     *
     * We are in Phase 15, BEFORE Ball_ApplyForce runs. The velocity
     * accumulators (0x170/0x174/0x178) contain the frame's roll physics
     * forces. By FADDing the jump impulse to vel.y NOW, the impulse enters
     * the force pipeline and Ball_ApplyForce will accumulate it into the
     * position delta. The ball lifts off the ground THIS frame, so next
     * frame's floor collision (type 5) won't fire and zero XZ velocity.
     *
     * FLD [ESI+0x174] → FADD [g_jump_vel] → FSTP [ESI+0x174]
     */
    /* FLD DWORD PTR [ESI+0x174]  — load current vel.y onto FPU stack */
    cave[p++] = 0xD9; cave[p++] = 0x86;
    *(DWORD*)(cave + p) = BALL_VEL_Y; p += 4;
    /* FADD DWORD PTR [g_jump_vel]  — add 500.0f from memory constant */
    cave[p++] = 0xD8; cave[p++] = 0x05;
    *(DWORD*)(cave + p) = (DWORD)&g_jump_vel; p += 4;
    /* FSTP DWORD PTR [ESI+0x174]  — store result back, preserving vel.x/z */
    cave[p++] = 0xD9; cave[p++] = 0x9E;
    *(DWORD*)(cave + p) = BALL_VEL_Y; p += 4;

    /* Set g_space_was_down = 1 */
    cave[p++] = 0xC6; cave[p++] = 0x05;
    *(DWORD*)(cave + p) = (DWORD)&g_space_was_down; p += 4;
    cave[p++] = 0x01;

    /* Increment g_jump_count */
    /* MOV EAX, [g_jump_count] */
    cave[p++] = 0xA1;
    *(DWORD*)(cave + p) = (DWORD)&g_jump_count; p += 4;
    /* INC EAX */
    cave[p++] = 0x40;
    /* MOV [g_jump_count], EAX */
    cave[p++] = 0xA3;
    *(DWORD*)(cave + p) = (DWORD)&g_jump_count; p += 4;

    /* JMP .done */
    cave[p++] = 0xEB; cave[p++] = 0x00;
    int jmp_done = p - 1;

    /* .not_pressed: */
    int not_pressed_label = p;
    /* Set g_space_was_down = 0 */
    cave[p++] = 0xC6; cave[p++] = 0x05;
    *(DWORD*)(cave + p) = (DWORD)&g_space_was_down; p += 4;
    cave[p++] = 0x00;

    /* .done: */
    int done_label = p;

    /* POP EDI, EAX — restore only the 2 registers we saved */
    cave[p++] = 0x5F;  /* POP EDI */
    cave[p++] = 0x58;  /* POP EAX */

    /* JMP back to hook_addr + 6 */
    cave[p++] = 0xE9;
    DWORD jmp_back = (DWORD)(hook_addr + HOOK_ORIG_BYTES) - (DWORD)(cave + p + 4);
    *(DWORD*)(cave + p) = jmp_back; p += 4;

    /* Fix up all placeholder jumps */
    cave[jnz_player]       = (BYTE)(done_label - (jnz_player + 1));
    cave[jnz_fallmode]     = (BYTE)(done_label - (jnz_fallmode + 1));
    cave[jz_not_grounded]  = (BYTE)(done_label - (jz_not_grounded + 1));
    cave[jz_app]           = (BYTE)(done_label - (jz_app + 1));
    cave[jz_ih]            = (BYTE)(done_label - (jz_ih + 1));
    cave[jz_kbd]           = (BYTE)(done_label - (jz_kbd + 1));
    cave[jz_notpressed]    = (BYTE)(not_pressed_label - (jz_notpressed + 1));
    cave[jne_already]      = (BYTE)(done_label - (jne_already + 1));
    cave[jmp_done]         = (BYTE)(done_label - (jmp_done + 1));

    /* Patch the hook site: JMP + 1 NOP (6 bytes total) */
    DWORD old_protect;
    VirtualProtect(hook_addr, HOOK_ORIG_BYTES, PAGE_EXECUTE_READWRITE, &old_protect);
    hook_addr[0] = 0xE9;
    *(DWORD*)(hook_addr + 1) = jmp_to_cave;
    hook_addr[5] = 0x90;
    VirtualProtect(hook_addr, HOOK_ORIG_BYTES, old_protect, &old_protect);
    FlushInstructionCache(GetCurrentProcess(), hook_addr, HOOK_ORIG_BYTES);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Patch Thread
 * ═══════════════════════════════════════════════════════════════════════════ */

static DWORD WINAPI patch_thread(LPVOID param)
{
    (void)param;
    Sleep(5000);

    /* Verify hook site has expected bytes */
    BYTE *hook_addr = (BYTE*)BALL_UPDATE_HOOK;
    BYTE expected[] = { 0x8B, 0x4C, 0x24, 0x1C, 0x8B, 0x11 };
    if (memcmp(hook_addr, expected, 6) != 0) {
        return 1;
    }

    install_hook();
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DllMain
 * ═══════════════════════════════════════════════════════════════════════════ */

BOOL APIENTRY DllMain(HMODULE hInst, DWORD reason, LPVOID lpReserved)
{
    (void)lpReserved;
    switch (reason) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hInst);
        load_real_bass();
        CreateThread(NULL, 0, patch_thread, NULL, 0, NULL);
        g_bg_thread = CreateThread(NULL, 0, ground_check_thread, NULL, 0, NULL);
        break;
    case DLL_PROCESS_DETACH:
        g_bg_active = 0;
        if (g_bg_thread) {
            WaitForSingleObject(g_bg_thread, 2000);
            CloseHandle(g_bg_thread);
            g_bg_thread = NULL;
        }
        break;
    }
    return TRUE;
}
