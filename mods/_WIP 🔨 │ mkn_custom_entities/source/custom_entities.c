/*
 * custom_entities.c — Hamsterball Custom Entities Mod v8
 *
 * bass.dll proxy mod. Runtime mesh loading:
 *   Reads testcube.MESHWORLD from mod directory.
 *   Parses vertices (24×32 bytes) and strips (6 strips).
 *   Creates MeshBuffer objects via game's CreateMeshBuffer.
 *   Adds them to the level's MeshWorld at S1 GRID positions.
 *   Colors each cube differently (Red, Orange, Yellow, Green, Blue).
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll custom_entities.c \
 *     -I../shared -lwinmm -Wl,--enable-stdcall-fixup -O2 -static \
 *     -static-libgcc -Wl,--add-stdcall-alias -msse2 -mfpmath=sse -lshlwapi
 */

#include "bass_proxy.h"
#include <shlwapi.h>
#include <ctype.h>
#include <stdio.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * Game function pointers
 * ═══════════════════════════════════════════════════════════════════════════ */

/* CreateMeshBuffer function pointer */
typedef void* (__fastcall *CreateMeshBuffer_t)(void* alloc);
static CreateMeshBuffer_t pfn_CreateMeshBuffer = (CreateMeshBuffer_t)0x00458600;

/* AthenaList_Append is declared in bass_proxy.h */
static AthenaList_Append_t  pfn_AthenaList_Append = (AthenaList_Append_t)0x00453810;

/* ═══════════════════════════════════════════════════════════════════════════
 * Structure offsets
 * ═══════════════════════════════════════════════════════════════════════════ */

#define BOARD_LEVEL             0x8AC
#define LEVEL_SCENEOBJECT       0x480
#define SCENEOBJ_S1_LIST        0x894
#define SCENEOBJ_MESHWORLD_PTR  0x08   /* sceneobj+0x08 = MeshWorld pointer */

#define S1ENTRY_NAME             0x00
#define S1ENTRY_POS_X            0x04
#define S1ENTRY_POS_Y            0x08
#define S1ENTRY_POS_Z            0x0C

/* MeshWorld layout */
#define MW_MB_COUNT             0x24   /* MeshBuffer file count */
#define MW_RENDERCTX_ARRAY      0x28   /* RenderContext array (allocated) */
#define MW_MBLIST               0x2C   /* AthenaList of MeshBuffer objects */
#define MW_VERTEX_DATA          0x45C  /* Vertex data start (24 byte header + bbox) */

/* MeshBuffer layout (0x874 bytes) */
#define MB_NAME_PTR             0x864  /* char* name */
#define MB_FLAG_NOSHADOW        0x85E  /* byte */
#define MB_STRIP_LIST           0x424  /* AthenaList of strip entries (0x109*4) */
#define MB_RENDERCTX_INDEX      0x04  /* DWORD index into MW_RENDERCTX_ARRAY */

/* RenderContext layout (0x50 bytes) */
#define RC_MATRIX_START         0x04   /* 4 matrices × 4 floats = 64 bytes */
#define RC_SCALE_FLAGS          0x4C   /* bool: scale != 1.0 */
#define RC_SHINE                0x44   /* float */
#define RC_HAS_TEX              0x48   /* DWORD: texture pointer or 0 */
#define RC_BOOL_FLAG            0x4D   /* byte */

/* ═══════════════════════════════════════════════════════════════════════════
 * State
 * ═══════════════════════════════════════════════════════════════════════════ */

static HANDLE g_thread = NULL;
static volatile int g_running = 1;
static char g_game_dir[MAX_PATH] = {0};
static DWORD g_last_level = 0;

/* Parsed testcube mesh data */
typedef struct {
    float vertices[24 * 8];  /* 24 verts × 8 floats (pos3 + norm3 + uv2) */
    int vertex_count;
    float material[16];      /* 4 groups × 4 floats */
    float shine;
    int has_texture;
    int bool_flag;
    int strip_count;
    int strips[12][2];       /* up to 12 strips, each (vref, count) */
} MeshData;

static MeshData g_mesh_data;
static int g_mesh_loaded = 0;

/* ═══════════════════════════════════════════════════════════════════════════
 * Helpers
 * ═══════════════════════════════════════════════════════════════════════════ */

static DWORD get_level(DWORD board) {
    if (!board) return 0;
    if (IsBadReadPtr((void*)(board + BOARD_LEVEL), 4)) return 0;
    DWORD level = *(DWORD*)(board + BOARD_LEVEL);
    if (!level || level < 0x10000) return 0;
    return level;
}

static DWORD get_sceneobj(DWORD board) {
    DWORD level = get_level(board);
    if (!level) return 0;
    if (IsBadReadPtr((void*)(level + LEVEL_SCENEOBJECT), 4)) return 0;
    DWORD sceneobj = *(DWORD*)(level + LEVEL_SCENEOBJECT);
    if (!sceneobj || sceneobj < 0x10000) return 0;
    return sceneobj;
}

static DWORD get_meshworld(DWORD board) {
    DWORD sceneobj = get_sceneobj(board);
    if (!sceneobj) return 0;
    if (IsBadReadPtr((void*)(sceneobj + SCENEOBJ_MESHWORLD_PTR), 4)) return 0;
    DWORD mw = *(DWORD*)(sceneobj + SCENEOBJ_MESHWORLD_PTR);
    if (!mw || mw < 0x10000) return 0;
    return mw;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * GRID: Parse (GRIDxx) from name
 * ═══════════════════════════════════════════════════════════════════════════ */

static int parse_grid_flag(const char* name) {
    const char* p = name;
    while ((p = strstr(p, "(GRID")) != NULL) {
        const char* digits = p + 5;
        if (isdigit((unsigned char)digits[0]) && isdigit((unsigned char)digits[1]) &&
            digits[2] == ')') {
            int num = (digits[0] - '0') * 10 + (digits[1] - '0');
            if (num >= 1 && num <= 99) return num;
        }
        if (isdigit((unsigned char)digits[0]) && digits[1] == ')') {
            int num = digits[0] - '0';
            if (num >= 1 && num <= 9) return num;
        }
        p++;
    }
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Load testcube.MESHWORLD from mod directory
 * ═══════════════════════════════════════════════════════════════════════════ */

static int load_testcube_mesh(void) {
    char path[MAX_PATH];
    /* Try mod dir first, then Levels subdirectory */
    snprintf(path, MAX_PATH, "%s\\testcube.MESHWORLD", g_game_dir);
    FILE* f = NULL;
    fopen_s(&f, path, "rb");
    if (!f) {
        snprintf(path, MAX_PATH, "%s\\Levels\\testcube.MESHWORLD", g_game_dir);
        fopen_s(&f, path, "rb");
    }
    if (!f) return 0;

    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (fsize < 100 || fsize > 100000) { fclose(f); return 0; }

    unsigned char* data = (unsigned char*)malloc(fsize);
    if (!data) { fclose(f); return 0; }
    fread(data, 1, fsize, f);
    fclose(f);

    /* Parse: S1(4) + S2(4) + S3(4) + bbox(24) + vc(4) + vertices(vc*32) + octree */
    int pos = 0;
    /* S1/S2/S3 counts (all 0 for testcube) */
    pos += 12;
    /* bbox */
    pos += 24;
    /* vertex count */
    int vc = *(int*)(data + pos);
    pos += 4;
    if (vc < 1 || vc > 1000) { free(data); return 0; }

    g_mesh_data.vertex_count = vc;

    /* Read vertices: each is 32 bytes (pos3 + norm3 + pad2 + uv2 = 8 floats) */
    int v;
    for (v = 0; v < vc && v < 24; v++) {
        float* vdata = (float*)(data + pos + v * 32);
        g_mesh_data.vertices[v * 8 + 0] = vdata[0];  /* posX */
        g_mesh_data.vertices[v * 8 + 1] = vdata[1];  /* posY */
        g_mesh_data.vertices[v * 8 + 2] = vdata[2];  /* posZ */
        g_mesh_data.vertices[v * 8 + 3] = vdata[3];  /* normX */
        g_mesh_data.vertices[v * 8 + 4] = vdata[4];  /* normY */
        g_mesh_data.vertices[v * 8 + 5] = vdata[5];  /* normZ */
        g_mesh_data.vertices[v * 8 + 6] = vdata[6];  /* uvU */
        g_mesh_data.vertices[v * 8 + 7] = vdata[7];  /* uvV */
    }
    pos += vc * 32;

    /* Parse octree: root(bbox24 + sc4) → if sc=1, child(bbox24 + sc4=0 + gc4=1) → mesh */
    /* Root */
    pos += 24;  /* bbox */
    int root_sc = *(int*)(data + pos);
    pos += 4;

    if (root_sc > 0) {
        /* Child */
        pos += 24;  /* bbox */
        int child_sc = *(int*)(data + pos);
        pos += 4;

        if (child_sc == 0) {
            int gc = *(int*)(data + pos);
            pos += 4;

            if (gc >= 1) {
                /* Parse mesh entry */
                int nl = *(int*)(data + pos);
                pos += 4;
                /* Skip name */
                pos += nl;

                /* Material: 4 groups × 4 floats = 64 bytes */
                memcpy(g_mesh_data.material, data + pos, 64);
                pos += 64;

                /* Shine */
                g_mesh_data.shine = *(float*)(data + pos);
                pos += 4;

                /* has_texture */
                g_mesh_data.has_texture = *(int*)(data + pos);
                pos += 4;

                /* Skip texture name if present */
                if (g_mesh_data.has_texture) {
                    int tl = *(int*)(data + pos);
                    pos += 4 + tl;
                }

                /* bool_flag */
                g_mesh_data.bool_flag = *(int*)(data + pos);
                pos += 4;

                /* strip_count */
                g_mesh_data.strip_count = *(int*)(data + pos);
                pos += 4;

                /* Read strips */
                int s;
                for (s = 0; s < g_mesh_data.strip_count && s < 12; s++) {
                    g_mesh_data.strips[s][0] = *(int*)(data + pos);      /* vref */
                    g_mesh_data.strips[s][1] = *(int*)(data + pos + 4);  /* count */
                    pos += 8;
                }
            }
        }
    }

    free(data);
    g_mesh_loaded = 1;
    return 1;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Spawn a MeshBuffer at a given position with a given color
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Color presets for GRID01-05: Red, Orange, Yellow, Green, Blue */
static const float GRID_COLORS[5][3] = {
    {0.8f, 0.0f, 0.0f},   /* GRID01 = Red */
    {1.0f, 0.5f, 0.0f},   /* GRID02 = Orange */
    {1.0f, 1.0f, 0.0f},   /* GRID03 = Yellow */
    {0.0f, 0.8f, 0.0f},   /* GRID04 = Green */
    {0.0f, 0.0f, 0.8f},   /* GRID05 = Blue */
};

static void spawn_testcube_at(DWORD meshworld, float px, float py, float pz, int grid_num, FILE* logf) {
    if (!g_mesh_loaded || !meshworld) return;

    /* Allocate a MeshBuffer (0x874 bytes) */
    void* alloc = (void*)LocalAlloc(LPTR, 0x874);
    if (!alloc) {
        if (logf) fprintf(logf, "  GRID: failed to allocate MeshBuffer for grid=%d\n", grid_num);
        return;
    }

    /* Create the MeshBuffer using game's constructor */
    void* mb = pfn_CreateMeshBuffer(alloc);
    if (!mb) {
        LocalFree(alloc);
        if (logf) fprintf(logf, "  GRID: CreateMeshBuffer failed for grid=%d\n", grid_num);
        return;
    }

    /* Set the "has geometry" flag at +0x217 */
    *(BYTE*)((char*)mb + 0x217) = 1;

    /* Set name */
    char name_buf[32];
    snprintf(name_buf, 32, "testcube(GRID%02d)", grid_num);
    /* Allocate name string */
    char* name = (char*)LocalAlloc(LPTR, strlen(name_buf) + 1);
    if (name) {
        strcpy(name, name_buf);
        *(char**)((char*)mb + MB_NAME_PTR) = name;
    }

    /* Add to MeshWorld's MeshBuffer list */
    void* mblist = (void*)(meshworld + MW_MBLIST);
    pfn_AthenaList_Append(mblist, mb);

    /* Get the RenderContext index (use the next available index) */
    int rc_count = 0;
    if (!IsBadReadPtr((void*)(meshworld + MW_MB_COUNT), 4)) {
        rc_count = *(int*)(meshworld + MW_MB_COUNT);
    }

    /* Set MeshBuffer's RenderContext index */
    *(int*)((char*)mb + MB_RENDERCTX_INDEX) = rc_count;

    /* Write position into the RenderContext array */
    DWORD* rc_array = *(DWORD**)(meshworld + MW_RENDERCTX_ARRAY);
    if (rc_array && !IsBadWritePtr(rc_array, (rc_count + 1) * 0x50)) {
        char* rc = (char*)rc_array + rc_count * 0x50;

        /* Set position in the matrix (4th row of first matrix = translation) */
        float* mat = (float*)(rc + RC_MATRIX_START + 0x0C * 4);  /* offset 0x30 in matrix = translation */
        /* Actually the matrix layout is:
         * mat[0..3] = row 0 (scale X)
         * mat[4..7] = row 1
         * mat[8..11] = row 2
         * mat[12..15] = row 3 (translation)
         * But RenderContext has 4 matrices of 16 bytes each (4 floats each, not 4x4)
         * 
         * From the decompilation, the material has 4 groups of 4 floats:
         * Group 0 = diffuse (RGBA)
         * Group 1 = ambient (RGBA)
         * Group 2 = specular (RGBA)
         * Group 3 = emissive/position (XYZ + flag)
         *
         * Actually, the RenderContext has 4 matrices:
         * mat0 (offset 0x04): 4 floats = scale XYZ + pad
         * mat1 (offset 0x14): 4 floats = rotation XYZ + pad
         * mat2 (offset 0x24): 4 floats = position XYZ + pad
         * mat3 (offset 0x34): 4 floats = ???
         *
         * Wait, from LoadMeshWorld:
         * iVar4 = *(int*)(*(int *)((int)this + 8) + 0x28);  // RC array
         * *(undefined4 *)(iVar4 + 0x18 + iVar7) = local_150;  // posX at RC+0x18
         * *(undefined4 *)(iVar4 + 0x1c) = local_158;  // posY at RC+0x1C
         * *(undefined4 *)(iVar4 + 0x14) = local_160;  // posZ at RC+0x14
         * *(float *)(iVar4 + 0x20) = local_154;  // scale at RC+0x20
         *
         * So: posZ at RC+0x14, posX at RC+0x18, posY at RC+0x1C, scale at RC+0x20
         */

        /* Position */
        *(float*)(rc + 0x14) = pz;   /* posZ */
        *(float*)(rc + 0x18) = px;   /* posX */
        *(float*)(rc + 0x1C) = py;   /* posY */
        *(float*)(rc + 0x20) = 1.0f; /* scale = 1.0 (visible) */

        /* Scale flag (RC+0x4C = bool: scale != 1.0) */
        *(BYTE*)(rc + 0x4C) = 0;  /* scale = 1.0, so flag = 0 */

        /* Material colors: diffuse at RC+0x04, ambient at RC+0x14? No...
         * Actually the RC layout from LoadMeshWorld is complex.
         * The game writes 4 groups of 4 floats at offsets:
         * Group 0: RC+0x04, +0x08, +0x0C, +0x10 (diffuse RGBA)
         * Group 1: RC+0x14, +0x18, +0x1C, +0x20 (ambient RGBA = position+scale!)
         * Group 2: RC+0x24, +0x28, +0x2C, +0x30 (specular RGBA)
         * Group 3: RC+0x34, +0x38, +0x3C, +0x40 (emissive RGBA)
         * Shine at RC+0x44
         * Texture at RC+0x48
         * Bool at RC+0x4D
         *
         * But the position writes to +0x14/+0x18/+0x1C which is AMBIENT group!
         * That means position IS stored as ambient color RGB, and scale as ambient alpha.
         * This makes sense: the RenderContext stores position in the "ambient" slot.
         */

        /* Set diffuse color from GRID_COLORS */
        int color_idx = grid_num - 1;
        if (color_idx < 0) color_idx = 0;
        if (color_idx > 4) color_idx = 4;

        *(float*)(rc + 0x04) = GRID_COLORS[color_idx][0];  /* diffuse R */
        *(float*)(rc + 0x08) = GRID_COLORS[color_idx][1];  /* diffuse G */
        *(float*)(rc + 0x0C) = GRID_COLORS[color_idx][2];  /* diffuse B */
        *(float*)(rc + 0x10) = 1.0f;                        /* diffuse A */

        /* Ambient = same as diffuse */
        /* These also serve as position + scale */
        /* But wait — we already wrote position to +0x14/+0x18/+0x1C above! */
        /* The position IS the ambient RGB. So we need to set position = color? */
        /* No — the game uses ambient for BOTH color and position. This is a dual-use field. */
        /* 
         * Actually, looking more carefully at LoadMeshWorld:
         * First it reads 4 floats for group 0 (diffuse) at RC+0x04..0x10
         * Then 4 floats for group 1 at RC+0x14..0x20
         * Then 4 floats for group 2 at RC+0x24..0x30
         * Then 4 floats for group 3 at RC+0x34..0x40
         *
         * The position writes are to the SAME offsets as group 1 (ambient).
         * This means the ambient color IS the position. The game stores
         * position as ambient color values.
         *
         * So we should set:
         * RC+0x04: diffuse R  (visible color)
         * RC+0x08: diffuse G
         * RC+0x0C: diffuse B
         * RC+0x10: diffuse A
         * RC+0x14: posX  (this IS ambient R, but used as position!)
         * RC+0x18: posY
         * RC+0x1C: posZ
         * RC+0x20: scale (= ambient A)
         */
    }

    /* Increment MeshBuffer count */
    if (!IsBadWritePtr((void*)(meshworld + MW_MB_COUNT), 4)) {
        *(int*)(meshworld + MW_MB_COUNT) = rc_count + 1;
    }

    if (logf) {
        fprintf(logf, "  GRID: spawned testcube(GRID%02d) at (%.1f,%.1f,%.1f) mb=0x%08X rc_idx=%d\n",
                grid_num, px, py, pz, (DWORD)mb, rc_count);
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Scan S1 ref points and spawn testcubes at GRID positions
 * ═══════════════════════════════════════════════════════════════════════════ */

static void apply_grid_visibility(DWORD board, FILE* logf) {
    DWORD sceneobj = get_sceneobj(board);
    if (!sceneobj) {
        if (logf) fprintf(logf, "  GRID: no sceneobj\n");
        return;
    }

    DWORD meshworld = get_meshworld(board);
    if (!meshworld) {
        if (logf) fprintf(logf, "  GRID: no meshworld\n");
        return;
    }

    /* S1 list is embedded AthenaList at sceneobj+0x894 */
    DWORD s1_list = sceneobj + SCENEOBJ_S1_LIST;
    if (IsBadReadPtr((void*)(s1_list + 0x04), 4)) return;
    int s1_count = *(int*)(s1_list + 0x04);
    if (s1_count < 1 || s1_count > 10000) return;

    if (IsBadReadPtr((void*)(s1_list + 0x40C), 4)) return;
    DWORD* s1_array = *(DWORD**)(s1_list + 0x40C);
    if (!s1_array || IsBadReadPtr(s1_array, s1_count * 4)) return;

    if (logf) fprintf(logf, "  GRID: scanning %d S1 ref points, meshworld=0x%08X\n", s1_count, meshworld);

    int found = 0;
    int i;
    for (i = 0; i < s1_count; i++) {
        DWORD entry = s1_array[i];
        if (!entry || entry < 0x10000) continue;
        if (IsBadReadPtr((void*)entry, 0x20)) continue;

        char* name = *(char**)(entry + S1ENTRY_NAME);
        if (!name || IsBadReadPtr(name, 4)) continue;

        int grid_num = parse_grid_flag(name);
        if (grid_num == 0) continue;

        found++;

        float px = *(float*)(entry + S1ENTRY_POS_X);
        float py = *(float*)(entry + S1ENTRY_POS_Y);
        float pz = *(float*)(entry + S1ENTRY_POS_Z);

        /* Spawn testcube at this position */
        spawn_testcube_at(meshworld, px, py, pz, grid_num, logf);
    }

    if (logf) fprintf(logf, "  GRID: found %d GRID ref points\n", found);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Main mod thread
 * ═══════════════════════════════════════════════════════════════════════════ */

static void init_game_dir(void) {
    char path[MAX_PATH];
    HMODULE hSelf = NULL;
    GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                      | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                      (LPCSTR)&init_game_dir, &hSelf);
    if (hSelf && GetModuleFileNameA(hSelf, path, MAX_PATH) > 0) {
        char *p = strrchr(path, '\\');
        if (p) { *p = '\0'; strcpy(g_game_dir, path); return; }
    }
    if (GetCurrentDirectoryA(MAX_PATH, path) > 0) {
        strcpy(g_game_dir, path);
    }
}

static DWORD WINAPI mod_thread(LPVOID param) {
    Sleep(3000);

    {
        char log_path[MAX_PATH];
        snprintf(log_path, MAX_PATH, "%s\\custom_entities.log", g_game_dir);
        FILE* f = NULL;
        fopen_s(&f, log_path, "a");
        if (f) {
            fprintf(f, "=== Custom Entities Mod v8 Started ===\n");
            fprintf(f, "Game dir: %s\n", g_game_dir);
            fclose(f);
        }
    }

    /* Load testcube mesh data */
    if (load_testcube_mesh()) {
        char log_path[MAX_PATH];
        snprintf(log_path, MAX_PATH, "%s\\custom_entities.log", g_game_dir);
        FILE* f = NULL;
        fopen_s(&f, log_path, "a");
        if (f) {
            fprintf(f, "Loaded testcube.MESHWORLD: %d verts, %d strips\n",
                    g_mesh_data.vertex_count, g_mesh_data.strip_count);
            fclose(f);
        }
    } else {
        char log_path[MAX_PATH];
        snprintf(log_path, MAX_PATH, "%s\\custom_entities.log", g_game_dir);
        FILE* f = NULL;
        fopen_s(&f, log_path, "a");
        if (f) {
            fprintf(f, "FAILED to load testcube.MESHWORLD from %s\n", g_game_dir);
            fclose(f);
        }
    }

    while (g_running) {
        DWORD board = get_board();
        if (!board) {
            Sleep(100);
            continue;
        }

        DWORD level = get_level(board);
        if (level != g_last_level && level) {
            g_last_level = level;

            /* Poll until sceneobj is ready (up to 10 seconds) */
            int poll;
            for (poll = 0; poll < 200; poll++) {
                DWORD so = get_sceneobj(board);
                if (so) break;
                Sleep(50);
            }

            char log_path[MAX_PATH];
            snprintf(log_path, MAX_PATH, "%s\\custom_entities.log", g_game_dir);
            FILE* logf = NULL;
            fopen_s(&logf, log_path, "a");

            if (logf) fprintf(logf, "\n--- Level loaded (board=0x%08X, level=0x%08X, after %dms poll) ---\n", board, level, poll * 50);

            /* Apply GRID: spawn testcubes at GRID positions */
            if (g_mesh_loaded) {
                apply_grid_visibility(board, logf);
            } else {
                if (logf) fprintf(logf, "  GRID: testcube mesh not loaded\n");
            }

            if (logf) {
                fprintf(logf, "Done.\n\n");
                fclose(logf);
            }
        }

        Sleep(2000);
    }

    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DllMain
 * ═══════════════════════════════════════════════════════════════════════════ */

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        load_real_bass();
        init_game_dir();
        g_thread = CreateThread(NULL, 0, mod_thread, NULL, 0, NULL);
    }
    else if (reason == DLL_PROCESS_DETACH) {
        g_running = 0;
        if (g_thread) {
            WaitForSingleObject(g_thread, 2000);
            CloseHandle(g_thread);
        }
    }
    return TRUE;
}
