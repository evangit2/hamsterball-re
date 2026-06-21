# App::Initialize Function Decomposition (0x00429530)

## Function Signature
```c
void App::Initialize(App* this, int hInstance, int nShowCmd);
// Called from WinMain at 0x4278EF
// Object pointer (this) in ECX = 0x4FD680 (global App instance)
// Stack args: hInstance (arg_4h), nShowCmd (arg_ch)
// Debug string stored at this+0x208 after each step
```

## Initialization Sequence (CONFIRMED from r2 disassembly)

### Step 1: App::Initialize(1) at 0x4D2B24
- Stores debug string "[Initialize(1)]" at this+0x208
- Calls fcn.0046BB40 (base init) with hInstance and nShowCmd
- Gets this->graphics (this+0x174)
- Sets this->graphics->initialized = true (this->graphics+0x7D1)

### Step 3: App::Initialize(3) at 0x4D2B08  
- Loads blank cursor: LoadCursorA(NULL, "BLANKCURSOR")
- Stores cursor handle at this+0x240

### Step 4: App::Initialize(4) at 0x4D2AF8
- Calls virtual function at this->+vtable+0x8C(140) with params 800, 600
  - This is likely Graphics::SetDisplayMode(800, 600)
  - this+0x240 = cursor handle
  - Push 0x258 (600) + Push 0x320 (800)

### Step 5: App::Initialize(5) at 0x4D2AE8
- Checks this->graphics (this+0x174)
- If NULL: sets debug string "** No Graphics **" at 0x4D2AD4
- Checks this->graphics->d3d_device (this->graphics+0x154)  
- If NULL: sets debug string "** No Graphics Device **" at 0x4D2AB8
- If graphics OK: calls d3d_device->vtable+0xC8(200) with params
  - This is likely IDirect3DDevice8::SetRenderState(D3DRS_AMBIENT, ...)
  - Or IDirect3DDevice8::SetTransform(D3DTS_WORLD, ...)
  - Pass 2 or 3 for lighting mode

### Step 6: App::Initialize(6) at 0x4D2A9C
- Loads shadow texture: fcn.00455C50(this->graphics, "shadow.png", 1)
  - fcn.00455C50 is likely Graphics::LoadTexture()
  - Result stored at this+0x278

### Step 7: App::Initialize(7) at 0x4D2A7C
- Loads music: fcn.004743F0("music\music.mo3")
  - fcn.004743F0 is Music::Load("music\music.mo3")
  - Result stored at this->music (this+0x534)
  - this->music_channel1 (this+0x53C) = 0
  - this->music_channel2 (this+0x538) = 0

### Step 8: App::Initialize(8) at 0x4D2A6C
- If music loaded successfully (this->music != 0):
  - Loads Jukebox: fcn.0046A4D0(this->music, "jukebox.xml")
  - Calls fcn.0046A3C0(this->music) -> this->music_channel1 (this+0x53C)
  - Calls fcn.0046A3C0(this->music) -> this->music_channel2 (this+0x538)

### Step 9: App::Initialize(9) at 0x4D2A50  
- Continues music initialization

### Step 10: App::Initialize(10) at 0x4D2A40
- Continues music initialization

### Step 11: App::Initialize(11) at 0x4D2A30
- Music setup continues

### Step 12: App::Initialize(12) at 0x4D2A20
- Calls fcn.00472EC0(this->config) — likely Config::Load()

### Step 13: App::Initialize(13) at 0x4D2A10
- Reads "PlayCount" from config: fcn.00473170(this->config, "PlayCount")
  - Check if key exists
  - If exists: read PlayCount value -> this+0x914
  - This is for shareware trial mode

### Continues with more initialization...

## App Object Structure (INFERRED from offsets)
```c
struct App {
    void* vtable;              // +0x00
    HWND hwnd;                  // +0x04
    /* ... */
    void* unknown_08;           // +0x08 through +0x50
    /* ... */
    Graphics* graphics;         // +0x174
    /* ... */
    char debug_str[256];        // +0x208 (stores init step name)
    /* ... */
    HCURSOR blank_cursor;       // +0x240
    /* ... */
    int display_width;          // +0x254 (800)
    int display_height;         // +0x258 (600)
    /* ... */
    void* shadow_texture;       // +0x278
    /* ... */
    MusicPlayer* music;          // +0x534
    int music_channel1;          // +0x53C
    int music_channel2;          // +0x538
    /* ... */
    int play_count;             // +0x914
    /* ... */
    int running;                // +0x159 (set to 1 to quit)
    int target_fps;             // +0x170 (used for Sleep calcs)
    int frame_count;            // +0x164 (for FPS calculation)
    int last_tick;              // +0x168 (for delta time)
    Config* config;             // +0x54
};
```

## Key Sub-Functions
| Address | Likely Name | Notes |
|---------|-------------|-------|
| 0x0046BB40 | App::BaseInit() | Called with hInstance, nShowCmd |
| 0x00455C50 | Graphics::LoadTexture() | Loads shadow.png and textures |
| 0x004743F0 | Music::Load() | Loads music.mo3 |
| 0x0046A4D0 | Music::LoadJukebox() | Loads jukebox.xml |
| 0x0046A3C0 | Music::CreateChannel() | Creates playback channels |
| 0x00472EC0 | Config::Load() | Loads configuration |
| 0x00473170 | Config::HasKey() | Checks if config key exists |
| 0x00473080 | Config::GetInt() | Gets integer config value |
| 0x00455380 | Graphics::Initialize() | Creates D3D8 device |
| 0x0046BD80 | App::Run() | Game loop (PeekMessageA based) |
| 0x0046BA10 | App::Shutdown() | Sets this->running = 1, calls vtable+8 |