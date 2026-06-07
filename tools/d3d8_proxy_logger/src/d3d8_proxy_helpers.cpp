//// d3d8_proxy_helpers.cpp — String conversion helpers + logging utilities

#include "d3d8_proxy.h"

FILE* g_logfile = nullptr;
HMODULE g_real_d3d8 = nullptr;
D3DC8_FN g_real_Direct3DCreate8 = nullptr;

void proxy_log_open() {
    g_logfile = fopen("hb_d3d8_trace.jsonl", "w");
    if (!g_logfile) {
        // Try temp path
        g_logfile = fopen("hb_d3d8_trace.jsonl", "ab");  // append if can't write fresh
        if (!g_logfile) return;
    }
    fprintf(g_logfile, "{\"event\":\"session_start\",\"timestamp\":%lld}\n",
            (long long)GetTickCount());
    fflush(g_logfile);
}

void proxy_log_close() {
    if (g_logfile) {
        proxy_log("session", "end");
        fclose(g_logfile);
        g_logfile = nullptr;
    }
}

void proxy_log(const char* category, const char* fmt, ...) {
    if (!g_logfile) return;
    fprintf(g_logfile, "{\"t\":%lld,\"cat\":\"%s\",\"msg\":\"",
            (long long)GetTickCount(), category);

    va_list args;
    va_start(args, fmt);
    vfprintf(g_logfile, fmt, args);
    va_end(args);

    fprintf(g_logfile, "\"}\n");
    fflush(g_logfile);
}

void proxy_log_raw(const char* json_line) {
    if (!g_logfile) return;
    fprintf(g_logfile, "%s\n", json_line);
    fflush(g_logfile);
}

//// Real DLL loading ////

bool load_real_d3d8() {
    char syspath[MAX_PATH];
    GetSystemDirectoryA(syspath, MAX_PATH);
    strcat(syspath, "\\d3d8.dll");
    g_real_d3d8 = LoadLibraryA(syspath);
    if (!g_real_d3d8) {
        // Try just loading by name (for Wine)
        g_real_d3d8 = LoadLibraryA("d3d8.dll");
    }
    if (!g_real_d3d8) {
        proxy_log("fatal", "Cannot load real d3d8.dll");
        return false;
    }
    g_real_Direct3DCreate8 = (D3DC8_FN)GetProcAddress(g_real_d3d8, "Direct3DCreate8");
    if (!g_real_Direct3DCreate8) {
        proxy_log("fatal", "Cannot find Direct3DCreate8 in real d3d8.dll");
        return false;
    }
    proxy_log("loader", "Loaded real d3d8.dll from %s", syspath);
    return true;
}

void free_real_d3d8() {
    if (g_real_d3d8) {
        FreeLibrary(g_real_d3d8);
        g_real_d3d8 = nullptr;
    }
}

//// Format string helpers ////

const char* d3dformat_str(D3DFORMAT fmt) {
    switch (fmt) {
        case D3DFMT_UNKNOWN: return "UNKNOWN";
        case D3DFMT_R8G8B8: return "R8G8B8";
        case D3DFMT_A8R8G8B8: return "A8R8G8B8";
        case D3DFMT_X8R8G8B8: return "X8R8G8B8";
        case D3DFMT_R5G6B5: return "R5G6B5";
        case D3DFMT_X1R5G5B5: return "X1R5G5B5";
        case D3DFMT_A1R5G5B5: return "A1R5G5B5";
        case D3DFMT_A4R4G4B4: return "A4R4G4B4";
        case D3DFMT_R3G3B2: return "R3G3B2";
        case D3DFMT_A8: return "A8";
        case D3DFMT_A8R3G3B2: return "A8R3G3B2";
        case D3DFMT_X4R4G4B4: return "X4R4G4B4";
        case D3DFMT_A8P8: return "A8P8";
        case D3DFMT_P8: return "P8";
        case D3DFMT_L8: return "L8";
        case D3DFMT_A8L8: return "A8L8";
        case D3DFMT_A4L4: return "A4L4";
        case D3DFMT_V8U8: return "V8U8";
        case D3DFMT_L6V5U5: return "L6V5U5";
        case D3DFMT_X8L8V8U8: return "X8L8V8U8";
        case D3DFMT_Q8W8V8U8: return "Q8W8V8U8";
        case D3DFMT_V16U16: return "V16U16";
        case D3DFMT_A2B10G10R10: return "A2B10G10R10";
        case D3DFMT_G16R16: return "G16R16";
        case D3DFMT_DXT1: return "DXT1";
        case D3DFMT_DXT2: return "DXT2";
        case D3DFMT_DXT3: return "DXT3";
        case D3DFMT_DXT4: return "DXT4";
        case D3DFMT_DXT5: return "DXT5";
        case D3DFMT_D16_LOCKABLE: return "D16_LOCKABLE";
        case D3DFMT_D32: return "D32";
        case D3DFMT_D15S1: return "D15S1";
        case D3DFMT_D24S8: return "D24S8";
        case D3DFMT_D16: return "D16";
        case D3DFMT_D24X8: return "D24X8";
        case D3DFMT_D24X4S4: return "D24X4S4";
        default: {
            static char buf[32];
            snprintf(buf, sizeof(buf), "0x%08X", (unsigned)fmt);
            return buf;
        }
    }
}

const char* d3ddevtype_str(D3DDEVTYPE t) {
    switch (t) {
        case D3DDEVTYPE_HAL:         return "HAL";
        case D3DDEVTYPE_REF:         return "REF";
        case D3DDEVTYPE_SW:          return "SW";
        default: {
            static char buf[16];
            snprintf(buf, sizeof(buf), "%d", (int)t);
            return buf;
        }
    }
}

const char* d3dpool_str(D3DPOOL p) {
    switch (p) {
        case D3DPOOL_DEFAULT:     return "DEFAULT";
        case D3DPOOL_MANAGED:     return "MANAGED";
        case D3DPOOL_SYSTEMMEM:   return "SYSTEMMEM";
        case D3DPOOL_SCRATCH:     return "SCRATCH";
        default: {
            static char buf[16];
            snprintf(buf, sizeof(buf), "%d", (int)p);
            return buf;
        }
    }
}

const char* d3dprim_str(D3DPRIMITIVETYPE t) {
    switch (t) {
        case D3DPT_POINTLIST:     return "POINTLIST";
        case D3DPT_LINELIST:      return "LINELIST";
        case D3DPT_LINESTRIP:     return "LINESTRIP";
        case D3DPT_TRIANGLELIST:  return "TRIANGLELIST";
        case D3DPT_TRIANGLESTRIP: return "TRIANGLESTRIP";
        case D3DPT_TRIANGLEFAN:   return "TRIANGLEFAN";
        default: {
            static char buf[16];
            snprintf(buf, sizeof(buf), "%d", (int)t);
            return buf;
        }
    }
}

const char* d3drs_str(D3DRENDERSTATETYPE rs) {
    switch (rs) {
        case D3DRS_ZENABLE:              return "ZENABLE";
        case D3DRS_FILLMODE:             return "FILLMODE";
        case D3DRS_SHADEMODE:            return "SHADEMODE";
        case D3DRS_ZWRITEENABLE:         return "ZWRITEENABLE";
        case D3DRS_ALPHATESTENABLE:      return "ALPHATESTENABLE";
        case D3DRS_LASTPIXEL:            return "LASTPIXEL";
        case D3DRS_SRCBLEND:             return "SRCBLEND";
        case D3DRS_DESTBLEND:            return "DESTBLEND";
        case D3DRS_CULLMODE:             return "CULLMODE";
        case D3DRS_ZFUNC:                return "ZFUNC";
        case D3DRS_ALPHAREF:             return "ALPHAREF";
        case D3DRS_ALPHAFUNC:            return "ALPHAFUNC";
        case D3DRS_DITHERENABLE:         return "DITHERENABLE";
        case D3DRS_ALPHABLENDENABLE:     return "ALPHABLENDENABLE";
        case D3DRS_FOGENABLE:            return "FOGENABLE";
        case D3DRS_SPECULARENABLE:       return "SPECULARENABLE";
        case D3DRS_FOGCOLOR:             return "FOGCOLOR";
        case D3DRS_FOGTABLEMODE:         return "FOGTABLEMODE";
        case D3DRS_FOGSTART:             return "FOGSTART";
        case D3DRS_FOGEND:               return "FOGEND";
        case D3DRS_FOGDENSITY:           return "FOGDENSITY";
        case D3DRS_RANGEFOGENABLE:       return "RANGEFOGENABLE";
        case D3DRS_STENCILENABLE:        return "STENCILENABLE";
        case D3DRS_STENCILFAIL:          return "STENCILFAIL";
        case D3DRS_STENCILZFAIL:         return "STENCILZFAIL";
        case D3DRS_STENCILPASS:          return "STENCILPASS";
        case D3DRS_STENCILFUNC:          return "STENCILFUNC";
        case D3DRS_STENCILREF:           return "STENCILREF";
        case D3DRS_STENCILMASK:          return "STENCILMASK";
        case D3DRS_STENCILWRITEMASK:     return "STENCILWRITEMASK";
        case D3DRS_TEXTUREFACTOR:        return "TEXTUREFACTOR";
        case D3DRS_WRAP0:                return "WRAP0";
        case D3DRS_WRAP1:                return "WRAP1";
        case D3DRS_WRAP2:                return "WRAP2";
        case D3DRS_WRAP3:                return "WRAP3";
        case D3DRS_WRAP4:                return "WRAP4";
        case D3DRS_WRAP5:                return "WRAP5";
        case D3DRS_WRAP6:                return "WRAP6";
        case D3DRS_WRAP7:                return "WRAP7";
        case D3DRS_CLIPPING:             return "CLIPPING";
        case D3DRS_LIGHTING:             return "LIGHTING";
        case D3DRS_AMBIENT:              return "AMBIENT";
        case D3DRS_FOGVERTEXMODE:        return "FOGVERTEXMODE";
        case D3DRS_COLORVERTEX:          return "COLORVERTEX";
        case D3DRS_LOCALVIEWER:          return "LOCALVIEWER";
        case D3DRS_NORMALIZENORMALS:     return "NORMALIZENORMALS";
        case D3DRS_DIFFUSEMATERIALSOURCE: return "DIFFUSEMATERIALSOURCE";
        case D3DRS_SPECULARMATERIALSOURCE: return "SPECULARMATERIALSOURCE";
        case D3DRS_AMBIENTMATERIALSOURCE:  return "AMBIENTMATERIALSOURCE";
        case D3DRS_EMISSIVEMATERIALSOURCE: return "EMISSIVEMATERIALSOURCE";
        case D3DRS_VERTEXBLEND:           return "VERTEXBLEND";
        case D3DRS_CLIPPLANEENABLE:       return "CLIPPLANEENABLE";
        case D3DRS_POINTSIZE:             return "POINTSIZE";
        case D3DRS_POINTSIZE_MIN:         return "POINTSIZE_MIN";
        case D3DRS_POINTSPRITEENABLE:     return "POINTSPRITEENABLE";
        case D3DRS_POINTSCALEENABLE:      return "POINTSCALEENABLE";
        case D3DRS_POINTSCALE_A:          return "POINTSCALE_A";
        case D3DRS_POINTSCALE_B:          return "POINTSCALE_B";
        case D3DRS_POINTSCALE_C:          return "POINTSCALE_C";
        case D3DRS_MULTISAMPLEANTIALIAS:  return "MULTISAMPLEANTIALIAS";
        case D3DRS_MULTISAMPLEMASK:       return "MULTISAMPLEMASK";
        case D3DRS_PATCHEDGESTYLE:        return "PATCHEDGESTYLE";
        case D3DRS_DEBUGMONITORTOKEN:     return "DEBUGMONITORTOKEN";
        case D3DRS_POINTSIZE_MAX:         return "POINTSIZE_MAX";
        case D3DRS_INDEXEDVERTEXBLENDENABLE: return "INDEXEDVERTEXBLENDENABLE";
        case D3DRS_COLORWRITEENABLE:      return "COLORWRITEENABLE";
        case D3DRS_TWEENFACTOR:           return "TWEENFACTOR";
        case D3DRS_BLENDOP:               return "BLENDOP";
        case D3DRS_POSITIONORDER:         return "POSITIONORDER";
        case D3DRS_NORMALORDER:           return "NORMALORDER";
        default: {
            static char buf[32];
            snprintf(buf, sizeof(buf), "RS_0x%X", (unsigned)rs);
            return buf;
        }
    }
}

const char* d3dtss_str(D3DTEXTURESTAGESTATETYPE tss) {
    switch (tss) {
        case D3DTSS_COLOROP:          return "COLOROP";
        case D3DTSS_COLORARG1:        return "COLORARG1";
        case D3DTSS_COLORARG2:        return "COLORARG2";
        case D3DTSS_ALPHAOP:          return "ALPHAOP";
        case D3DTSS_ALPHAARG1:        return "ALPHAARG1";
        case D3DTSS_ALPHAARG2:        return "ALPHAARG2";
        case D3DTSS_BUMPENVMAT00:     return "BUMPENVMAT00";
        case D3DTSS_BUMPENVMAT01:     return "BUMPENVMAT01";
        case D3DTSS_BUMPENVMAT10:     return "BUMPENVMAT10";
        case D3DTSS_BUMPENVMAT11:     return "BUMPENVMAT11";
        case D3DTSS_TEXCOORDINDEX:    return "TEXCOORDINDEX";
        case D3DTSS_ADDRESSU:         return "ADDRESSU";
        case D3DTSS_ADDRESSV:         return "ADDRESSV";
        case D3DTSS_BORDERCOLOR:      return "BORDERCOLOR";
        case D3DTSS_MAGFILTER:        return "MAGFILTER";
        case D3DTSS_MINFILTER:        return "MINFILTER";
        case D3DTSS_MIPFILTER:        return "MIPFILTER";
        case D3DTSS_MIPMAPLODBIAS:    return "MIPMAPLODBIAS";
        case D3DTSS_MAXMIPLEVEL:      return "MAXMIPLEVEL";
        case D3DTSS_MAXANISOTROPY:    return "MAXANISOTROPY";
        case D3DTSS_BUMPENVLSCALE:    return "BUMPENVLSCALE";
        case D3DTSS_BUMPENVLOFFSET:   return "BUMPENVLOFFSET";
        case D3DTSS_TEXTURETRANSFORMFLAGS: return "TEXTURETRANSFORMFLAGS";
        case D3DTSS_ADDRESSW:         return "ADDRESSW";
        case D3DTSS_COLORARG0:        return "COLORARG0";
        case D3DTSS_ALPHAARG0:        return "ALPHAARG0";
        case D3DTSS_RESULTARG:        return "RESULTARG";
        default: {
            static char buf[32];
            snprintf(buf, sizeof(buf), "TSS_0x%X", (unsigned)tss);
            return buf;
        }
    }
}

const char* d3dtop_str(D3DTEXTUREOP op) {
    switch (op) {
        case D3DTOP_DISABLE:              return "DISABLE";
        case D3DTOP_SELECTARG1:           return "SELECTARG1";
        case D3DTOP_SELECTARG2:           return "SELECTARG2";
        case D3DTOP_MODULATE:             return "MODULATE";
        case D3DTOP_MODULATE2X:           return "MODULATE2X";
        case D3DTOP_MODULATE4X:           return "MODULATE4X";
        case D3DTOP_ADD:                  return "ADD";
        case D3DTOP_ADDSIGNED:            return "ADDSIGNED";
        case D3DTOP_ADDSIGNED2X:          return "ADDSIGNED2X";
        case D3DTOP_SUBTRACT:             return "SUBTRACT";
        case D3DTOP_ADDSMOOTH:            return "ADDSMOOTH";
        case D3DTOP_BLENDDIFFUSEALPHA:    return "BLENDDIFFUSEALPHA";
        case D3DTOP_BLENDTEXTUREALPHA:    return "BLENDTEXTUREALPHA";
        case D3DTOP_BLENDFACTORALPHA:     return "BLENDFACTORALPHA";
        case D3DTOP_BLENDTEXTUREALPHAPM:  return "BLENDTEXTUREALPHAPM";
        case D3DTOP_BLENDCURRENTALPHA:    return "BLENDCURRENTALPHA";
        case D3DTOP_PREMODULATE:          return "PREMODULATE";
        case D3DTOP_MODULATEALPHA_ADDCOLOR:  return "MODULATEALPHA_ADDCOLOR";
        case D3DTOP_MODULATECOLOR_ADDALPHA:  return "MODULATECOLOR_ADDALPHA";
        case D3DTOP_MODULATEINVALPHA_ADDCOLOR: return "MODULATEINVALPHA_ADDCOLOR";
        case D3DTOP_MODULATEINVCOLOR_ADDALPHA: return "MODULATEINVCOLOR_ADDALPHA";
        case D3DTOP_BUMPENVMAP:           return "BUMPENVMAP";
        case D3DTOP_BUMPENVMAPLUMINANCE:  return "BUMPENVMAPLUMINANCE";
        case D3DTOP_DOTPRODUCT3:          return "DOTPRODUCT3";
        default: {
            static char buf[16];
            snprintf(buf, sizeof(buf), "OP_%d", (int)op);
            return buf;
        }
    }
}

const char* d3dblend_str(D3DBLEND b) {
    switch (b) {
        case D3DBLEND_ZERO:           return "ZERO";
        case D3DBLEND_ONE:            return "ONE";
        case D3DBLEND_SRCCOLOR:       return "SRCCOLOR";
        case D3DBLEND_INVSRCCOLOR:    return "INVSRCCOLOR";
        case D3DBLEND_SRCALPHA:       return "SRCALPHA";
        case D3DBLEND_INVSRCALPHA:    return "INVSRCALPHA";
        case D3DBLEND_DESTALPHA:      return "DESTALPHA";
        case D3DBLEND_INVDESTALPHA:   return "INVDESTALPHA";
        case D3DBLEND_DESTCOLOR:      return "DESTCOLOR";
        case D3DBLEND_INVDESTCOLOR:   return "INVDESTCOLOR";
        case D3DBLEND_SRCALPHASAT:    return "SRCALPHASAT";
        case D3DBLEND_BOTHSRCALPHA:   return "BOTHSRCALPHA";
        case D3DBLEND_BOTHINVSRCALPHA: return "BOTHINVSRCALPHA";
        default: {
            static char buf[16];
            snprintf(buf, sizeof(buf), "%d", (int)b);
            return buf;
        }
    }
}

const char* d3dcull_str(D3DCULL c) {
    switch (c) {
        case D3DCULL_NONE:  return "NONE";
        case D3DCULL_CW:    return "CW";
        case D3DCULL_CCW:   return "CCW";
        default: {
            static char buf[8];
            snprintf(buf, sizeof(buf), "%d", (int)c);
            return buf;
        }
    }
}

const char* d3dfill_str(D3DFILLMODE f) {
    switch (f) {
        case D3DFILL_POINT:     return "POINT";
        case D3DFILL_WIREFRAME: return "WIREFRAME";
        case D3DFILL_SOLID:     return "SOLID";
        default: {
            static char buf[8];
            snprintf(buf, sizeof(buf), "%d", (int)f);
            return buf;
        }
    }
}

const char* d3dcmp_str(D3DCMPFUNC f) {
    switch (f) {
        case D3DCMP_NEVER:        return "NEVER";
        case D3DCMP_LESS:         return "LESS";
        case D3DCMP_EQUAL:        return "EQUAL";
        case D3DCMP_LESSEQUAL:    return "LESSEQUAL";
        case D3DCMP_GREATER:      return "GREATER";
        case D3DCMP_NOTEQUAL:     return "NOTEQUAL";
        case D3DCMP_GREATEREQUAL: return "GREATEREQUAL";
        case D3DCMP_ALWAYS:       return "ALWAYS";
        default: {
            static char buf[8];
            snprintf(buf, sizeof(buf), "%d", (int)f);
            return buf;
        }
    }
}

const char* fvf_flags_str(DWORD fvf) {
    static char buf[256];
    buf[0] = 0;
    if (fvf & D3DFVF_XYZ)          strcat(buf, "XYZ ");
    if (fvf & D3DFVF_XYZRHW)       strcat(buf, "XYZRHW ");
    if (fvf & D3DFVF_XYZB1)        strcat(buf, "XYZB1 ");
    if (fvf & D3DFVF_XYZB2)        strcat(buf, "XYZB2 ");
    if (fvf & D3DFVF_XYZB3)        strcat(buf, "XYZB3 ");
    if (fvf & D3DFVF_XYZB4)        strcat(buf, "XYZB4 ");
    if (fvf & D3DFVF_XYZB5)        strcat(buf, "XYZB5 ");
    if (fvf & D3DFVF_NORMAL)       strcat(buf, "NORMAL ");
    if (fvf & D3DFVF_PSIZE)        strcat(buf, "PSIZE ");
    if (fvf & D3DFVF_DIFFUSE)      strcat(buf, "DIFFUSE ");
    if (fvf & D3DFVF_SPECULAR)     strcat(buf, "SPECULAR ");
    if (fvf & D3DFVF_TEX0)         strcat(buf, "TEX0 ");
    if (fvf & D3DFVF_TEX1)         strcat(buf, "TEX1 ");
    if (fvf & D3DFVF_TEX2)         strcat(buf, "TEX2 ");
    if (fvf & D3DFVF_TEX3)         strcat(buf, "TEX3 ");
    if (fvf & D3DFVF_TEX4)         strcat(buf, "TEX4 ");
    if (fvf & D3DFVF_TEX5)         strcat(buf, "TEX5 ");
    if (fvf & D3DFVF_TEX6)         strcat(buf, "TEX6 ");
    if (fvf & D3DFVF_TEX7)         strcat(buf, "TEX7 ");
    if (fvf & D3DFVF_TEX8)         strcat(buf, "TEX8 ");
    if (buf[0]) buf[strlen(buf)-1] = 0; // trim trailing space
    return buf;
}

const char* d3dswapeffect_str(D3DSWAPEFFECT e) {
    switch (e) {
        case D3DSWAPEFFECT_DISCARD:    return "DISCARD";
        case D3DSWAPEFFECT_FLIP:       return "FLIP";
        case D3DSWAPEFFECT_COPY:       return "COPY";
        default: {
            static char buf[16];
            snprintf(buf, sizeof(buf), "%d", (int)e);
            return buf;
        }
    }
}

const char* d3dmultisample_str(D3DMULTISAMPLE_TYPE t) {
    switch (t) {
        case D3DMULTISAMPLE_NONE:         return "NONE";
        case D3DMULTISAMPLE_2_SAMPLES:    return "2_SAMPLES";
        case D3DMULTISAMPLE_3_SAMPLES:    return "3_SAMPLES";
        case D3DMULTISAMPLE_4_SAMPLES:    return "4_SAMPLES";
        case D3DMULTISAMPLE_5_SAMPLES:    return "5_SAMPLES";
        case D3DMULTISAMPLE_6_SAMPLES:    return "6_SAMPLES";
        case D3DMULTISAMPLE_7_SAMPLES:    return "7_SAMPLES";
        case D3DMULTISAMPLE_8_SAMPLES:    return "8_SAMPLES";
        case D3DMULTISAMPLE_9_SAMPLES:    return "9_SAMPLES";
        case D3DMULTISAMPLE_10_SAMPLES:   return "10_SAMPLES";
        case D3DMULTISAMPLE_11_SAMPLES:   return "11_SAMPLES";
        case D3DMULTISAMPLE_12_SAMPLES:   return "12_SAMPLES";
        case D3DMULTISAMPLE_13_SAMPLES:   return "13_SAMPLES";
        case D3DMULTISAMPLE_14_SAMPLES:   return "14_SAMPLES";
        case D3DMULTISAMPLE_15_SAMPLES:   return "15_SAMPLES";
        case D3DMULTISAMPLE_16_SAMPLES:   return "16_SAMPLES";
        default: {
            static char buf[16];
            snprintf(buf, sizeof(buf), "%d", (int)t);
            return buf;
        }
    }
}

const char* d3dbackbuffer_str(D3DBACKBUFFER_TYPE t) {
    switch (t) {
        case D3DBACKBUFFER_TYPE_MONO:         return "MONO";
        case D3DBACKBUFFER_TYPE_LEFT:         return "LEFT";
        case D3DBACKBUFFER_TYPE_RIGHT:        return "RIGHT";
        default: {
            static char buf[8];
            snprintf(buf, sizeof(buf), "%d", (int)t);
            return buf;
        }
    }
}

void log_matrix(const char* name, const D3DMATRIX* m) {
    if (!g_logfile || !m) return;
    fprintf(g_logfile, "{\"t\":%lld,\"cat\":\"matrix\",\"name\":\"%s\",",
            (long long)GetTickCount(), name);
    fprintf(g_logfile, "\"m\":[%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,"
            "%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f]}\n",
            m->_11,m->_12,m->_13,m->_14,
            m->_21,m->_22,m->_23,m->_24,
            m->_31,m->_32,m->_33,m->_34,
            m->_41,m->_42,m->_43,m->_44);
    fflush(g_logfile);
}
