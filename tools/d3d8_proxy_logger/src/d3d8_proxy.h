//// d3d8_proxy.h — Shared types and macros for D3D8 proxy logger
//// Hamsterball Browser Port — Phase 2

#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d8.h>
#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <cstring>

//// Logging ////

extern FILE* g_logfile;

void proxy_log_open();
void proxy_log_close();
void proxy_log(const char* category, const char* fmt, ...);
void proxy_log_raw(const char* json_line);

//// Real DLL loading ////

typedef IDirect3D8* (WINAPI *D3DC8_FN)(UINT);

extern HMODULE g_real_d3d8;
extern D3DC8_FN g_real_Direct3DCreate8;

bool load_real_d3d8();
void free_real_d3d8();

//// Helpers ////

const char* d3dformat_str(D3DFORMAT fmt);
const char* d3ddevtype_str(D3DDEVTYPE t);
const char* d3dpool_str(D3DPOOL p);
const char* d3dprim_str(D3DPRIMITIVETYPE t);
const char* d3drs_str(D3DRENDERSTATETYPE rs);
const char* d3dtss_str(D3DTEXTURESTAGESTATETYPE tss);
const char* d3dtop_str(D3DTEXTUREOP op);
const char* d3dcmp_str(D3DCMPFUNC f);
const char* d3dblend_str(D3DBLEND b);
const char* d3dcull_str(D3DCULL c);
const char* d3dfill_str(D3DFILLMODE f);
const char* d3dmultisample_str(D3DMULTISAMPLE_TYPE t);
const char* d3dswapeffect_str(D3DSWAPEFFECT e);
const char* d3dbackbuffer_str(D3DBACKBUFFER_TYPE t);
const char* fvf_flags_str(DWORD fvf);
void log_matrix(const char* name, const D3DMATRIX* m);

//// Forwarded vtable wrappers ////

// We create wrapper objects that forward all calls to the real object,
// log arguments/results, then return.
// MinGW d3d8.h declares COM interfaces as pure virtual — we can't hold
// them as struct fields. Instead we use raw function pointer arrays.

struct ProxyDevice8 {
    IDirect3DDevice8* real;
    void*             vt[97];  // IDirect3DDevice8 has 97 vtable slots (MinGW d3d8.h)
};

// Resource wrappers (stubs for now — used when we switch to faking)
struct ProxyTexture8     { IDirect3DTexture8*     real; void* vt[20]; };
struct ProxySurface8     { IDirect3DSurface8*     real; void* vt[15]; };
struct ProxyVB8          { IDirect3DVertexBuffer8* real; void* vt[14]; };
struct ProxyIB8          { IDirect3DIndexBuffer8*  real; void* vt[14]; };
struct ProxySwapChain8   { IDirect3DSwapChain8*   real; void* vt[10]; };
