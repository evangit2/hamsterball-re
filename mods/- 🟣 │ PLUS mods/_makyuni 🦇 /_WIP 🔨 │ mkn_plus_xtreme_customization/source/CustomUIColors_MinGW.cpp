//
// CustomUIColors_MinGW.cpp -- MinGW cross-compile version for testing.
// Uses manual 17-entry vtable (HB+ v2.0) + nocrt.
// The VS source (CustomUIColors.cpp) is the primary deliverable.
//
// v2: JSONC config parser -- reads .jsonc file with block comments and line comments stripped.
//     For now: Loading Screen section (colors, strings, posX/Y, scaleX/Y, paths, links).
//
#include "nocrt.h"
#include "HamsterballAPI.h"

// Redirect CRT calls to nc_ versions
#define malloc nc_malloc
#define free nc_free
#define memcpy nc_memcpy
#define memset nc_memset
#define strlen nc_strlen
#define strcmp nc_strcmp
#define strncpy nc_strncpy

// ===========================================================================
// Color Site Definitions (unchanged from v1)
// ===========================================================================

#define CAVE_RGB         0
#define CAVE_RGBA        1
#define CAVE_RGBA_CALL   2
#define CAVE_RGBA_FPU    3

#define MATRIX_SCALE_4X4 0x00453150

static const unsigned char FMUL_BYTES[] = { 0xD8, 0x8E, 0x5C, 0x2D, 0x00, 0x00 };

struct ColorSite {
    const char* key;
    DWORD patchAddr;
    DWORD returnAddr;
    int patchSize;
    int caveType;
    int colorIdx;
};

#define NUM_COLOR_SITES 28
static float g_colors[NUM_COLOR_SITES][4];

static ColorSite g_sites[] = {
    {"\"click here\" button - off",  0x0042D5FD, 0x0042D606, 9,  CAVE_RGB,      0},
    {"\"click here\" button - on",   0x0042D624, 0x0042D62D, 9,  CAVE_RGB,      1},
    {"hblogo - l",                   0x0042D375, 0x0042D389, 20, CAVE_RGBA,     2},
    {"hblogo - r",                   0x0042D3D2, 0x0042D3E6, 20, CAVE_RGBA,     3},
    {"hamster - only",               0x0042D472, 0x0042D486, 20, CAVE_RGBA,     4},
    {"hamster + ball",               0x0042D4D3, 0x0042D4DD, 10, CAVE_RGBA_FPU, 5},
    {"loader swirl",                 0x0042D5A0, 0x0042D5A9, 9,  CAVE_RGBA_CALL,6},
    {"background",                   0x0042D2B4, 0x0042D2C8, 20, CAVE_RGBA,     7},
    {"raptisoft logo",               0x0042DA60, 0x0042DA74, 20, CAVE_RGBA,     8},
    {"hbversion",                    0x00426433, 0x00426441, 14, CAVE_RGBA,     9},
    {"hbversion - shadow",           0x0042641A, 0x00426425, 11, CAVE_RGBA,    27},
    {"button #1",                    0x0042DE82, 0x0042DE96, 20, CAVE_RGBA,    10},
    {"button #2",                    0x0042DED8, 0x0042DEEC, 20, CAVE_RGBA,    11},
    {"button #3",                    0x0042DF0D, 0x0042DF21, 20, CAVE_RGBA,    12},
    {"button #4",                    0x0042DF42, 0x0042DF56, 20, CAVE_RGBA,    13},
    {"button #5",                    0x0042E002, 0x0042E016, 20, CAVE_RGBA,    14},
    {"lp button #1",                 0x0042E092, 0x0042E0A6, 20, CAVE_RGBA,    15},
    {"lp button #2",                 0x0042E0E9, 0x0042E0FD, 20, CAVE_RGBA,    16},
    {"lp button #2b",                0x0042E15E, 0x0042E172, 20, CAVE_RGBA,    17},
    {"lp button #3",                 0x0042E12E, 0x0042E142, 20, CAVE_RGBA,    18},
    {"lp button #4",                 0x0042E19C, 0x0042E1B0, 20, CAVE_RGBA,    19},
    {"lp button #5",                 0x0042E1D1, 0x0042E1E5, 20, CAVE_RGBA,    20},
    {"button #1 - resume",           0x0042E722, 0x0042E736, 20, CAVE_RGBA,    21},
    {"button #2 - restart",          0x0042E76F, 0x0042E783, 20, CAVE_RGBA,    22},
    {"button #3 - options",          0x0042E7A4, 0x0042E7B8, 20, CAVE_RGBA,    23},
    {"button #4 - quit",             0x0042E7E2, 0x0042E7F6, 20, CAVE_RGBA,    24},
    {"side strip",                   0x00431A5F, 0x00431A6F, 16, CAVE_RGBA,    25},
    {"side strip r",                 0x00431AA3, 0x00431AB3, 16, CAVE_RGBA,    25},
    {"win strip",                    0x0044D68D, 0x0044D69B, 14, CAVE_RGBA,    26},
};

#define NUM_SITES (sizeof(g_sites) / sizeof(g_sites[0]))

// ===========================================================================
// String Site Definitions (unchanged from v1)
// ===========================================================================

struct StringSite {
    const char* key;
    DWORD address;
    int maxLength;
};

static StringSite g_string_sites[] = {
    {"button - txt",            0x004D3EAC, 19},
    {"string - let's play",     0x004D3F10, 11},
    {"string - options",        0x004D3F00,  7},
    {"string - credits",        0x004D3EF4,  7},
    {"string - exit to desktop",0x004D3EC3, 16},
    {"string - choose a game",  0x004D3FF0, 14},
    {"string - tournament",     0x004D3FE0, 10},
    {"string - time trials",    0x004D3FCC, 11},
    {"string - locked",         0x004D3FBC,  6},
    {"string - mirror tournament",0x004D3FA8,18},
    {"string - party games",    0x004D3F94, 11},
    {"string - previous",       0x004D3F88,  8},
    {"string - choose a time trial", 0x004D4644, 25},
    {"string - previous menu", 0x004D426C, 13},
    {"string - pause - resume", 0x004D410C,  6},
    {"string - pause - restart",0x004D4198, 12},
    {"string - pause - quit",   0x004D4188, 14},
};

#define NUM_STRING_SITES (sizeof(g_string_sites) / sizeof(g_string_sites[0]))

// ===========================================================================
// JSONC Config Parser
//
// Reads a .jsonc file, strips block comments and line comments,
// then parses the active JSON to extract per-element customization values.
//
// JSONC format (per MAKYUNI spec):
//   [
//     {
//       "Element Name": {
//         "color": ["#RRGGBBAA", "#RRGGBBAA"],
//         "posX": 0.0, "posY": 0.0,
//         "scaleX": 0.0, "scaleY": 0.0,
//         "string": "text",
//         "path": "Textures/foo" or ["Textures/a", "Textures/b"],
//         "link": "https://..."
//       }
//     }
//   ]
// ===========================================================================

static char g_config_path[MAX_PATH] = {0};
static unsigned char* g_cave_mem = NULL;
static int g_reload_counter = 0;

// -- JSONC comment stripping ----------------------------------------------

// Strip block comments (slash-star ... star-slash) and line comments (// ...)
// from JSONC text, producing clean JSON.
// Output buffer must be at least as large as input.
// Handles comment markers inside strings (ignores them).
static void strip_jsonc_comments(const char* input, char* output, int input_len)
{
    int in_string = 0;        // inside a quoted string
    int in_block_comment = 0;  // inside block comment
    int in_line_comment = 0;   // inside line comment
    int out_pos = 0;

    for (int i = 0; i < input_len; i++) {
        char c = input[i];
        char next = (i + 1 < input_len) ? input[i + 1] : '\0';

        if (in_block_comment) {
            if (c == '*' && next == '/') {
                in_block_comment = 0;
                i++; // skip the slash
                output[out_pos++] = ' ';
            }
            continue;
        }

        if (in_line_comment) {
            if (c == '\n') {
                in_line_comment = 0;
                output[out_pos++] = c;
            }
            continue;
        }

        if (in_string) {
            output[out_pos++] = c;
            if (c == '\\' && i + 1 < input_len) {
                output[out_pos++] = input[++i];
            } else if (c == '"') {
                in_string = 0;
            }
            continue;
        }

        // Not in string or comment -- check for comment starts
        if (c == '/' && next == '*') {
            in_block_comment = 1;
            i++;
            output[out_pos++] = ' ';
            continue;
        }
        if (c == '/' && next == '/') {
            in_line_comment = 1;
            i++;
            output[out_pos++] = ' ';
            continue;
        }

        if (c == '"') {
            in_string = 1;
        }

        output[out_pos++] = c;
    }

    output[out_pos] = '\0';
}

// -- Minimal JSON tokenizer/parser -----------------------------------------

#define JTK_END      0
#define JTK_LBRACE   1
#define JTK_RBRACE   2
#define JTK_LBRACK   3
#define JTK_RBRACK   4
#define JTK_COLON    5
#define JTK_COMMA    6
#define JTK_STRING    7
#define JTK_NUMBER    8
#define JTK_TRUE     9
#define JTK_FALSE    10
#define JTK_NULL     11

struct JsonToken {
    int type;
    const char* start;
    int length;
    float num_val;
};

struct JsonParser {
    const char* json;
    int pos;
    int len;
};

static void skip_ws(JsonParser* p)
{
    while (p->pos < p->len) {
        char c = p->json[p->pos];
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            p->pos++;
        } else {
            break;
        }
    }
}

static int parse_number(const char* s, int len, float* out)
{
    int i = 0;
    float sign = 1.0f;
    if (i < len && s[i] == '-') { sign = -1.0f; i++; }
    float integer = 0.0f;
    while (i < len && s[i] >= '0' && s[i] <= '9') {
        integer = integer * 10.0f + (float)(s[i] - '0');
        i++;
    }
    float frac = 0.0f;
    if (i < len && s[i] == '.') {
        i++;
        float scale = 0.1f;
        while (i < len && s[i] >= '0' && s[i] <= '9') {
            frac += (float)(s[i] - '0') * scale;
            scale *= 0.1f;
            i++;
        }
    }
    *out = sign * (integer + frac);
    return i;
}

static JsonToken next_token(JsonParser* p)
{
    JsonToken tk;
    tk.type = JTK_END;
    tk.start = NULL;
    tk.length = 0;
    tk.num_val = 0.0f;

    skip_ws(p);

    if (p->pos >= p->len) {
        tk.type = JTK_END;
        return tk;
    }

    char c = p->json[p->pos];

    switch (c) {
    case '{': tk.type = JTK_LBRACE;  p->pos++; return tk;
    case '}': tk.type = JTK_RBRACE;  p->pos++; return tk;
    case '[': tk.type = JTK_LBRACK;  p->pos++; return tk;
    case ']': tk.type = JTK_RBRACK;  p->pos++; return tk;
    case ':': tk.type = JTK_COLON;   p->pos++; return tk;
    case ',': tk.type = JTK_COMMA;   p->pos++; return tk;
    case '"': {
        p->pos++;
        tk.type = JTK_STRING;
        tk.start = p->json + p->pos;
        int start = p->pos;
        while (p->pos < p->len) {
            char ch = p->json[p->pos];
            if (ch == '\\' && p->pos + 1 < p->len) {
                p->pos += 2;
            } else if (ch == '"') {
                break;
            } else {
                p->pos++;
            }
        }
        tk.length = p->pos - start;
        if (p->pos < p->len) p->pos++;
        return tk;
    }
    case 't':
        if (p->pos + 4 <= p->len && p->json[p->pos+1] == 'r' &&
            p->json[p->pos+2] == 'u' && p->json[p->pos+3] == 'e') {
            tk.type = JTK_TRUE; p->pos += 4; return tk;
        }
        break;
    case 'f':
        if (p->pos + 5 <= p->len && p->json[p->pos+1] == 'a' &&
            p->json[p->pos+2] == 'l' && p->json[p->pos+3] == 's' &&
            p->json[p->pos+4] == 'e') {
            tk.type = JTK_FALSE; p->pos += 5; return tk;
        }
        break;
    case 'n':
        if (p->pos + 4 <= p->len && p->json[p->pos+1] == 'u' &&
            p->json[p->pos+2] == 'l' && p->json[p->pos+3] == 'l') {
            tk.type = JTK_NULL; p->pos += 4; return tk;
        }
        break;
    }

    if (c == '-' || (c >= '0' && c <= '9')) {
        int start = p->pos;
        if (c == '-') p->pos++;
        while (p->pos < p->len) {
            char ch = p->json[p->pos];
            if ((ch >= '0' && ch <= '9') || ch == '.' || ch == '-' ||
                ch == '+' || ch == 'e' || ch == 'E') {
                p->pos++;
            } else {
                break;
            }
        }
        tk.type = JTK_NUMBER;
        tk.start = p->json + start;
        tk.length = p->pos - start;
        parse_number(tk.start, tk.length, &tk.num_val);
        return tk;
    }

    tk.type = JTK_END;
    return tk;
}

// Skip an arbitrary JSON value
static void skip_value(JsonParser* p)
{
    JsonToken tk = next_token(p);
    if (tk.type == JTK_LBRACE) {
        int depth = 1;
        while (depth > 0) {
            tk = next_token(p);
            if (tk.type == JTK_LBRACE) depth++;
            else if (tk.type == JTK_RBRACE) depth--;
            else if (tk.type == JTK_END) break;
        }
    } else if (tk.type == JTK_LBRACK) {
        int depth = 1;
        while (depth > 0) {
            tk = next_token(p);
            if (tk.type == JTK_LBRACK) depth++;
            else if (tk.type == JTK_RBRACK) depth--;
            else if (tk.type == JTK_END) break;
        }
    }
}

// -- Color parsing helpers -------------------------------------------------

static int hex_digit(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static int parse_hex_color(const char* text, int textLen, DWORD* out_hex)
{
    int p = 0;
    while (p < textLen && (text[p] == ' ' || text[p] == '\t')) p++;
    if (p < textLen && text[p] == '#') p++;
    else if (p + 1 < textLen && text[p] == '0' && (text[p+1] == 'x' || text[p+1] == 'X')) p += 2;

    char buf[16] = {0};
    int digits = 0;
    while (digits < 8 && p < textLen && hex_digit(text[p]) >= 0) {
        buf[digits++] = text[p++];
    }

    if (digits >= 8) {
        *out_hex = 0;
        for (int i = 0; i < 8; i++)
            *out_hex = (*out_hex << 4) | (DWORD)hex_digit(buf[i]);
        return 1;
    } else if (digits >= 6) {
        *out_hex = 0;
        for (int i = 0; i < 6; i++)
            *out_hex = (*out_hex << 4) | (DWORD)hex_digit(buf[i]);
        *out_hex = (*out_hex << 8) | 0xFF;
        return 1;
    } else if (digits >= 3) {
        DWORD r = (DWORD)hex_digit(buf[0]) * 0x11;
        DWORD g = (DWORD)hex_digit(buf[1]) * 0x11;
        DWORD b = (DWORD)hex_digit(buf[2]) * 0x11;
        *out_hex = (r << 24) | (g << 16) | (b << 8) | 0xFF;
        return 1;
    }
    return 0;
}

static void hex_to_floats(DWORD hex, float* r, float* g, float* b, float* a)
{
    *r = ((hex >> 24) & 0xFF) / 255.0f;
    *g = ((hex >> 16) & 0xFF) / 255.0f;
    *b = ((hex >> 8)  & 0xFF) / 255.0f;
    *a = ( hex        & 0xFF) / 255.0f;
}

// Copy a JSON string token into a null-terminated buffer (handles escapes)
static void copy_json_string(const char* src, int len, char* dst, int maxDst)
{
    int di = 0;
    for (int si = 0; si < len && di < maxDst - 1; si++) {
        if (src[si] == '\\' && si + 1 < len) {
            si++;
            char esc = src[si];
            switch (esc) {
            case 'n': dst[di++] = '\n'; break;
            case 't': dst[di++] = '\t'; break;
            case 'r': dst[di++] = '\r'; break;
            case '"': dst[di++] = '"'; break;
            case '\\': dst[di++] = '\\'; break;
            case '/': dst[di++] = '/'; break;
            default: dst[di++] = esc; break;
            }
        } else {
            dst[di++] = src[si];
        }
    }
    dst[di] = '\0';
}

// -- Config application ---------------------------------------------------

static int json_key_matches(const char* key, int keyLen, const char* needle)
{
    int needleLen = (int)nc_strlen(needle);
    if (needleLen == 0 || keyLen < needleLen) return 0;

    for (int i = 0; i <= keyLen - needleLen; i++) {
        int match = 1;
        for (int j = 0; j < needleLen; j++) {
            char a = key[i + j];
            char b = needle[j];
            if (a >= 'A' && a <= 'Z') a += 32;
            if (b >= 'A' && b <= 'Z') b += 32;
            if (a != b) { match = 0; break; }
        }
        if (match) return 1;
    }
    return 0;
}

static void apply_string(const StringSite* site, const char* text)
{
    DWORD old_protect;
    if (!VirtualProtect((void*)site->address, site->maxLength + 1,
                        PAGE_READWRITE, &old_protect))
        return;
    nc_memcpy((void*)site->address, text, nc_strlen(text) + 1);
    VirtualProtect((void*)site->address, site->maxLength + 1, old_protect, &old_protect);
    FlushInstructionCache(GetCurrentProcess(), (void*)site->address, site->maxLength + 1);
}

static void apply_color_to_site(int site_idx, DWORD hex)
{
    int idx = g_sites[site_idx].colorIdx;
    float r, g, b, a;
    hex_to_floats(hex, &r, &g, &b, &a);
    g_colors[idx][0] = r; g_colors[idx][1] = g;
    g_colors[idx][2] = b; g_colors[idx][3] = a;
}

// -- Element name to site index mapping ------------------------------------

struct ElementMap {
    const char* jsonName;
    int siteIdx;
    int colorCount;
};

static ElementMap g_element_map[] = {
    {"click here",               0, 2},
    {"hamsterball logo",         2, 2},
    {"loader _no_ ball",         4, 1},
    {"loader with ball",         5, 1},
    {"rotating swirl",           6, 1},
    {"raptisoft logo",           8, 1},
};

#define NUM_ELEMENT_MAPS (sizeof(g_element_map) / sizeof(g_element_map[0]))

struct StringElementMap {
    const char* jsonName;
    int stringSiteIdx;
};

static StringElementMap g_string_element_map[] = {
    {"click here",               0},
};

#define NUM_STRING_ELEMENT_MAPS (sizeof(g_string_element_map) / sizeof(g_string_element_map[0]))

// -- Parse a single element's value object ---------------------------------

static void parse_element_values(JsonParser* p, const char* elemName, int elemNameLen)
{
    JsonToken tk = next_token(p);
    if (tk.type != JTK_LBRACE) {
        if (tk.type != JTK_END) skip_value(p);
        return;
    }

    int matched_color_idx = -1;
    int color_count = 0;
    for (int i = 0; i < (int)NUM_ELEMENT_MAPS; i++) {
        if (json_key_matches(elemName, elemNameLen, g_element_map[i].jsonName)) {
            matched_color_idx = g_element_map[i].siteIdx;
            color_count = g_element_map[i].colorCount;
            break;
        }
    }

    int matched_string_idx = -1;
    for (int i = 0; i < (int)NUM_STRING_ELEMENT_MAPS; i++) {
        if (json_key_matches(elemName, elemNameLen, g_string_element_map[i].jsonName)) {
            matched_string_idx = g_string_element_map[i].stringSiteIdx;
            break;
        }
    }

    tk = next_token(p);
    if (tk.type == JTK_RBRACE) return;

    while (tk.type != JTK_RBRACE && tk.type != JTK_END) {
        if (tk.type != JTK_STRING) {
            tk = next_token(p);
            continue;
        }

        char keyBuf[128];
        copy_json_string(tk.start, tk.length, keyBuf, sizeof(keyBuf));

        tk = next_token(p);
        if (tk.type != JTK_COLON) {
            if (tk.type != JTK_END) skip_value(p);
            tk = next_token(p);
            continue;
        }

        tk = next_token(p);

        if (nc_stricmp(keyBuf, "color") == 0 && matched_color_idx >= 0) {
            if (tk.type == JTK_LBRACK) {
                int color_idx = 0;
                tk = next_token(p);
                while (tk.type != JTK_RBRACK && tk.type != JTK_END) {
                    if (tk.type == JTK_STRING) {
                        DWORD hex;
                        if (parse_hex_color(tk.start, tk.length, &hex)) {
                            if (color_idx == 0 && color_count >= 1)
                                apply_color_to_site(matched_color_idx, hex);
                            if (color_idx == 1 && color_count >= 2)
                                apply_color_to_site(matched_color_idx + 1, hex);
                        }
                        color_idx++;
                    }
                    tk = next_token(p);
                    if (tk.type == JTK_COMMA) tk = next_token(p);
                }
            } else if (tk.type == JTK_STRING) {
                DWORD hex;
                if (parse_hex_color(tk.start, tk.length, &hex)) {
                    apply_color_to_site(matched_color_idx, hex);
                }
            } else {
                skip_value(p);
            }
        }
        else if (nc_stricmp(keyBuf, "string") == 0 && matched_string_idx >= 0) {
            if (tk.type == JTK_STRING) {
                char strBuf[256];
                copy_json_string(tk.start, tk.length, strBuf, sizeof(strBuf));
                apply_string(&g_string_sites[matched_string_idx], strBuf);
            } else {
                skip_value(p);
            }
        }
        else if (nc_stricmp(keyBuf, "posX") == 0 ||
                 nc_stricmp(keyBuf, "posY") == 0 ||
                 nc_stricmp(keyBuf, "scaleX") == 0 ||
                 nc_stricmp(keyBuf, "scaleY") == 0) {
            // Number value already consumed -- TODO: apply when hook system ready
        }
        else if (nc_stricmp(keyBuf, "path") == 0) {
            if (tk.type == JTK_LBRACK) {
                int depth = 1;
                while (depth > 0) {
                    tk = next_token(p);
                    if (tk.type == JTK_LBRACK) depth++;
                    else if (tk.type == JTK_RBRACK) depth--;
                    else if (tk.type == JTK_END) break;
                }
            } else if (tk.type == JTK_STRING) {
                // Single path -- TODO: apply when path system ready
            } else {
                skip_value(p);
            }
        }
        else if (nc_stricmp(keyBuf, "link") == 0) {
            if (tk.type == JTK_STRING) {
                // TODO: apply hyperlink when system ready
            } else {
                skip_value(p);
            }
        }
        else {
            if (tk.type == JTK_LBRACE || tk.type == JTK_LBRACK) {
                skip_value(p);
            }
        }

        tk = next_token(p);
        if (tk.type == JTK_COMMA) {
            tk = next_token(p);
        }
    }
}

// -- Config file path and default config -----------------------------------

// Find the DLL's own folder using VirtualQuery on a function pointer.
// GetModuleFileNameA(NULL) returns the game EXE path, not the DLL path.
// The .jsonc config must live next to the DLL in the Mods folder.
static void init_config_path(void)
{
    HMODULE hSelf = NULL;

    // Get the module handle for this DLL by querying a function pointer
    // that lives inside our DLL's code section.
    MEMORY_BASIC_INFORMATION mbi;
    if (VirtualQuery((void*)&init_config_path, &mbi, sizeof(mbi))) {
        hSelf = (HMODULE)mbi.AllocationBase;
    }

    if (hSelf) {
        char dllPath[MAX_PATH];
        DWORD len = GetModuleFileNameA(hSelf, dllPath, MAX_PATH);
        if (len > 0) {
            char* last = NULL;
            char* p = dllPath;
            while (*p) { if (*p == '\\' || *p == '/') last = p; p++; }
            if (last) {
                *(last + 1) = '\0';
                nc_strncpy(g_config_path, dllPath, MAX_PATH - 1);
                nc_strncpy(g_config_path + nc_strlen(g_config_path),
                           "mkn_plus_xtreme_customization.jsonc",
                           MAX_PATH - nc_strlen(g_config_path) - 1);
                g_config_path[MAX_PATH - 1] = '\0';
                return;
            }
        }
    }

    // Fallback: use current directory
    nc_strncpy(g_config_path, "mkn_plus_xtreme_customization.jsonc", MAX_PATH - 1);
}

static const char* DEFAULT_CONFIG =
"[\n"
"  {\n"
"    \"LoadingScreen - Click Here- Button\":               { \"color\": [\"#FFFFFFFF\", \"#FFFFFFFF\"], \"posX\":    0.0, \"posY\":    0.0,                                   \"string\": \"CLICK HERE TO PLAY!\"                                                },\n"
"    \"LoadingScreen - Hamsterball Logo\":                 { \"color\": [\"#FFFFFFFF\", \"#FFFFFFFF\"], \"posX\":    0.0, \"posY\":    0.0, \"scaleX\":   0.0, \"scaleY\":   0.0, \"path\":  [\"Textures/TitleText-Left\", \"Textures/TitleText-Right\"]               },\n"
"    \"LoadingScreen - Hamster Loader _no_ ball\":         { \"color\":  \"#FFFFFFFF\",               \"posX\":    0.0, \"posY\":    0.0, \"scaleX\":   0.0, \"scaleY\":   0.0, \"path\":   \"Textures/Loader(Grey)\"                                              },\n"
"    \"LoadingScreen - Hamster Loader with ball\":         { \"color\":  \"#FFFFFFFF\",               \"posX\":    0.0, \"posY\":    0.0, \"scaleX\":   0.0, \"scaleY\":   0.0, \"path\":   \"Textures/Loader\"                                                    },\n"
"    \"LoadingScreen - Rotating Swirl\":                   { \"color\":  \"#FFFFFFFF\",               \"posX\":    0.0, \"posY\":    0.0, \"scaleX\":   0.0, \"scaleY\":   0.0, \"path\":   \"Textures/LoadingSwirl\"                                              },\n"
"    \"LoadingScreen - Raptisoft Logo\":                   { \"color\":  \"#FFFFFFFF\",               \"posX\":    0.0, \"posY\":    0.0, \"scaleX\":   0.0, \"scaleY\":   0.0, \"path\":   \"Textures/RaptisoftLogo\", \"link\": \"https://www.raptisoft.com\"        },\n"
"  }\n"
"]\n";

static void create_default_config(void)
{
    HANDLE h = CreateFileA(g_config_path, GENERIC_READ, FILE_SHARE_READ, NULL,
                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) {
        h = CreateFileA(g_config_path, GENERIC_WRITE, 0, NULL,
                        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (h != INVALID_HANDLE_VALUE) {
            DWORD written;
            WriteFile(h, DEFAULT_CONFIG, (DWORD)nc_strlen(DEFAULT_CONFIG), &written, NULL);
            CloseHandle(h);
        }
    } else {
        CloseHandle(h);
    }
}

// -- Read and parse the JSONC config file ---------------------------------

static char g_json_buffer[16384];
static char g_clean_json[16384];

static void read_config(void)
{
    HANDLE h = CreateFileA(g_config_path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                           NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return;

    DWORD bytesRead = 0;
    ReadFile(h, g_json_buffer, sizeof(g_json_buffer) - 1, &bytesRead, NULL);
    CloseHandle(h);
    if (bytesRead == 0) return;

    g_json_buffer[bytesRead] = '\0';

    strip_jsonc_comments(g_json_buffer, g_clean_json, (int)bytesRead);

    JsonParser parser;
    parser.json = g_clean_json;
    parser.pos = 0;
    parser.len = (int)nc_strlen(g_clean_json);

    JsonToken tk = next_token(&parser);
    if (tk.type != JTK_LBRACK) return;

    tk = next_token(&parser);
    while (tk.type != JTK_RBRACK && tk.type != JTK_END) {
        if (tk.type == JTK_COMMA) {
            tk = next_token(&parser);
            continue;
        }
        if (tk.type != JTK_LBRACE) {
            tk = next_token(&parser);
            continue;
        }

        tk = next_token(&parser);
        while (tk.type != JTK_RBRACE && tk.type != JTK_END) {
            if (tk.type == JTK_COMMA) {
                tk = next_token(&parser);
                continue;
            }
            if (tk.type != JTK_STRING) {
                tk = next_token(&parser);
                continue;
            }

            const char* elemName = tk.start;
            int elemNameLen = tk.length;

            tk = next_token(&parser);
            if (tk.type != JTK_COLON) {
                if (tk.type != JTK_END) skip_value(&parser);
                tk = next_token(&parser);
                continue;
            }

            parse_element_values(&parser, elemName, elemNameLen);

            tk = next_token(&parser);
            if (tk.type == JTK_COMMA) {
                tk = next_token(&parser);
            }
        }

        tk = next_token(&parser);
        if (tk.type == JTK_COMMA) {
            tk = next_token(&parser);
        }
    }
}

// ===========================================================================
// Code Cave Builders (unchanged from v1)
// ===========================================================================

static int write_push_mem(unsigned char* p, float* val)
{
    p[0] = 0xFF; p[1] = 0x35;
    *(DWORD*)(p + 2) = (DWORD)val;
    return 6;
}

static int write_jmp(unsigned char* p, DWORD target)
{
    p[0] = 0xE9;
    *(DWORD*)(p + 1) = target - ((DWORD)p + 5);
    return 5;
}

static int write_call(unsigned char* p, DWORD target)
{
    p[0] = 0xE8;
    *(DWORD*)(p + 1) = target - ((DWORD)p + 5);
    return 5;
}

static int build_cave(unsigned char* cave, const ColorSite* site)
{
    int idx = site->colorIdx;
    float* r = &g_colors[idx][0];
    float* g = &g_colors[idx][1];
    float* b = &g_colors[idx][2];
    float* a = &g_colors[idx][3];
    int p = 0;

    switch (site->caveType) {
    case CAVE_RGB:
        p += write_push_mem(cave + p, b);
        p += write_push_mem(cave + p, g);
        p += write_push_mem(cave + p, r);
        p += write_jmp(cave + p, site->returnAddr);
        break;
    case CAVE_RGBA:
        p += write_push_mem(cave + p, a);
        p += write_push_mem(cave + p, b);
        p += write_push_mem(cave + p, g);
        p += write_push_mem(cave + p, r);
        p += write_jmp(cave + p, site->returnAddr);
        break;
    case CAVE_RGBA_CALL:
        p += write_push_mem(cave + p, a);
        p += write_push_mem(cave + p, b);
        p += write_push_mem(cave + p, g);
        p += write_push_mem(cave + p, r);
        p += write_call(cave + p, MATRIX_SCALE_4X4);
        p += write_jmp(cave + p, site->returnAddr);
        break;
    case CAVE_RGBA_FPU:
        p += write_push_mem(cave + p, a);
        p += write_push_mem(cave + p, b);
        p += write_push_mem(cave + p, g);
        nc_memcpy(cave + p, (void*)FMUL_BYTES, sizeof(FMUL_BYTES));
        p += sizeof(FMUL_BYTES);
        p += write_push_mem(cave + p, r);
        p += write_jmp(cave + p, site->returnAddr);
        break;
    }
    return p;
}

static void patch_site(const ColorSite* site, unsigned char* cave_addr)
{
    DWORD old_protect;
    if (!VirtualProtect((void*)site->patchAddr, site->patchSize,
                        PAGE_EXECUTE_READWRITE, &old_protect))
        return;
    *(unsigned char*)site->patchAddr = 0xE9;
    *(DWORD*)(site->patchAddr + 1) = (DWORD)cave_addr - (site->patchAddr + 5);
    for (int i = 5; i < site->patchSize; i++)
        *(unsigned char*)(site->patchAddr + i) = 0x90;
    VirtualProtect((void*)site->patchAddr, site->patchSize, old_protect, &old_protect);
    FlushInstructionCache(GetCurrentProcess(), (void*)site->patchAddr, site->patchSize);
}

static void install_patches(void)
{
    int max_cave_size = 0;
    for (int i = 0; i < (int)NUM_SITES; i++) {
        int size = 0;
        switch (g_sites[i].caveType) {
        case CAVE_RGB:        size = 23; break;
        case CAVE_RGBA:       size = 29; break;
        case CAVE_RGBA_CALL:  size = 34; break;
        case CAVE_RGBA_FPU:   size = 35; break;
        }
        if (size > max_cave_size) max_cave_size = size;
    }

    int total = (int)NUM_SITES * (max_cave_size + 4) + 64;
    g_cave_mem = (unsigned char*)VirtualAlloc(NULL, total,
                    MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!g_cave_mem) return;

    int offset = 0;
    for (int i = 0; i < (int)NUM_SITES; i++) {
        unsigned char* cave = g_cave_mem + offset;
        int cave_size = build_cave(cave, &g_sites[i]);
        patch_site(&g_sites[i], cave);
        offset += cave_size + 4;
    }
}

// ===========================================================================
// Manual Vtable (HB+ v2.0, 17 entries)
// ===========================================================================

typedef void* (__thiscall *dtor_t)(void*, int);
typedef const char* (__thiscall *get_str_t)(void*);
typedef int (__thiscall *get_int_t)(void*);
typedef void (__thiscall *init_t)(void*, void*);
typedef void (__thiscall *void_cb_t)(void*);
typedef void (__thiscall *ball_update_t)(void*, void*);
typedef void (__thiscall *button_toggle_t)(void*, const char*, bool);
typedef void (__thiscall *slider_change_t)(void*, const char*, float);
typedef void (__thiscall *cycle_change_t)(void*, const char*, const char*);
typedef void (__thiscall *event_collide_t)(void*, void*, char*);
typedef void (__thiscall *ball_bump_t)(void*, void*, void*);
typedef void (__thiscall *render_apply_t)(void*, void*, float*);

static void* __thiscall sc_dtor(void* thisptr, int flags) {
    if (flags & 1) nc_free(thisptr);
    return thisptr;
}
static const char* __thiscall get_mod_name(void*) { return "Xtreme Customization"; }
static const char* __thiscall get_author(void*) { return "MAKYUNI"; }
static int __thiscall get_version(void*) { return HAMSTERBALL_API_VERSION; }
static const char* __thiscall get_contributors(void*) { return "Hamsterbot"; }

static void __thiscall init_impl(void* thisptr, void* modApi)
{
    *(void**)((char*)thisptr + 4) = modApi;

    for (int i = 0; i < NUM_COLOR_SITES; i++) {
        g_colors[i][0] = 1.0f;
        g_colors[i][1] = 1.0f;
        g_colors[i][2] = 1.0f;
        g_colors[i][3] = 1.0f;
    }

    g_colors[27][0] = 0.0f;
    g_colors[27][1] = 0.0f;
    g_colors[27][2] = 0.0f;
    g_colors[27][3] = 1.0f;

    init_config_path();
    create_default_config();
    read_config();
    install_patches();
}

static void __thiscall game_update_impl(void* thisptr)
{
    if (++g_reload_counter >= 120) {
        g_reload_counter = 0;
        read_config();
    }
}

static void __thiscall ball_update_impl(void*, void*) {}
static void __thiscall render_apply_impl(void*, void*, float*) {}
static void __thiscall button_toggle_impl(void*, const char*, bool) {}
static void __thiscall slider_change_impl(void*, const char*, float) {}
static void __thiscall cycle_change_impl(void*, const char*, const char*) {}
static void __thiscall event_collide_impl(void*, void*, char*) {}
static void __thiscall text_render_impl(void*) {}
static void __thiscall ball_bump_impl(void*, void*, void*) {}
static void __thiscall scene_end_impl(void*) {}
static void __thiscall level_start_impl(void*) {}

static void* g_vtable[17] = {
    (void*)sc_dtor,
    (void*)get_mod_name,
    (void*)get_author,
    (void*)get_version,
    (void*)get_contributors,
    (void*)init_impl,
    (void*)ball_update_impl,
    (void*)render_apply_impl,
    (void*)button_toggle_impl,
    (void*)slider_change_impl,
    (void*)cycle_change_impl,
    (void*)game_update_impl,
    (void*)event_collide_impl,
    (void*)text_render_impl,
    (void*)ball_bump_impl,
    (void*)scene_end_impl,
    (void*)level_start_impl,
};

extern "C" __declspec(dllexport) HamsterballAPI* CreateModInstance() {
    void* obj = nc_malloc(8);
    if (!obj) return NULL;
    *(void**)obj = g_vtable;
    *(void**)((char*)obj + 4) = NULL;
    return (HamsterballAPI*)obj;
}
