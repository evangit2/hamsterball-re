# App::Run - Game Loop Decomposition (0x0046BD80)

## Function Signature
```c
void App::Run(App* this);  // ECX = App* (0x4FD680)
// Called from WinMain at 0x4278F9
```

## Key App Structure Offsets
```c
// this+0x159: running flag (1 = quit, 0 = running)
// this+0x15A: active flag (1 = game active, 0 = paused/minimized)
// this+0x158: minimized flag
// this+0x164: last_tick (GetTickCount value)
// this+0x168: frame_time (1000 / this->0x170 = ms per frame)
// this+0x170: target_fps (e.g., 60)
// this+0x174: graphics pointer
// this+0x18C: frame counter (for FPS tracking)
// this+0x194: frame_count (for display)
// this+0x1AC: show_fps flag
```

## Pseudocode

```c
void App::Run(App* this) {
    // Initialize timing
    int frame_time = 1000 / this->target_fps;   // this+0x170 = FPS target
    HMODULE kernel32 = GetTickCount;
    int last_tick = GetTickCount();
    this->last_tick = last_tick;
    
    if (this->running) goto shutdown;  // this+0x159
    
    push edi;
    
    // === MAIN GAME LOOP ===
    while (1) {
        Sleep(0);  // Yield to other processes
        
        // Calculate frame timing
        int tick_interval = 1000 / this->fps_counter;  // this+0x16C
        this->debug_str = "Background";  // this+0x208
        this->frame_time_ms = tick_interval;  // this+0x168
        
        int current_tick = GetTickCount();
        
        // FPS counting
        if (current_tick > global_time_limit) {  // 0x5341E4
            if (this->show_fps) {  // this+0x1AC
                sprintf(this->fps_buffer, "%d", this->frame_count);  // this+0x198
            }
            this->frame_count = 0;  // this+0x194
            global_time_limit = GetTickCount() + 1000;
        }
        
        // === MESSAGE PUMP ===
        MSG msg;
        while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (this->running) goto shutdown;
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
        
        if (this->running) goto shutdown;
        
        // === TIMING CHECK ===
        // Only update/draw if enough time has passed
        int now = GetTickCount();
        int elapsed = now - this->last_tick;
        int min_interval = this->frame_time_ms - 5;
        
        if (elapsed < min_interval) {
            // Too early - skip frame or count skipped frames
            this->skip_count++;  // var_10h
            if (this->skip_count >= 10) {
                // Too many skipped frames - force update
                goto force_update;
            }
            
            // === UPDATE FRAME ===
            this->frame_counter++;  // this+0x18C
            Graphics::BeginScene(this->graphics);  // fcn.00453B50
            this->vtable->Update(this);              // vtable+0x20
            this->last_tick += this->frame_time_ms;
            // Cap delta time at 1000ms
            if (now - this->last_tick > 1000) {
                this->last_tick = now;  // Reset if too far behind
            }
            this->rendered_frames++;
        } else {
            // === RENDER FRAME ===
            this->debug_str = "Draw";  // this+0x208
            
            if (this->graphics && !this->paused && !this->minimized) {
                this->frame_count++;  // this+0x194
                Graphics::BeginScene(this->graphics);  // fcn.00453B50
                this->vtable->Update(this);              // vtable+0x20
                this->vtable->Draw(this);                // vtable+0x24
                this->vtable->Flip(this);                 // vtable+0x28
                // Present (Graphics::Present or D3D Present)
                Graphics::Present(this->graphics, 1);    // fcn.00455A90
            }
            
            this->last_tick = GetTickCount();
        }
    }
    
shutdown:
    // Cleanup
    destructor(var_34);
    return;
}
```

## Virtual Table Methods
The game uses a vtable at `this->vtable` with key methods:
- `vtable+0x08`: Shutdown (from App::Shutdown at 0x46BA10)
- `vtable+0x20` (32): **Update** — Game logic per frame
- `vtable+0x24` (36): **Draw** — Rendering per frame
- `vtable+0x28` (40): **Flip** — Buffer swap / present
- `vtable+0x2C` (44): Additional render step
- `vtable+0x8C` (140): SetDisplayMode (called during init with 800, 600)

## Key Sub-Functions
| Address | Name | Description |
|---------|------|-------------|
| 0x453B50 | Graphics::BeginScene | Prepares scene for rendering |
| 0x455A90 | Graphics::Present | Presents frame buffer |
| 0x457AD0 | (constructor?) | Called at function start |
| 0x457A40 | (destructor?) | Called at function end |
| 0x4BAE43 | sprintf | Used for FPS display |

## Timing Model
- Target FPS stored at `this+0x170`
- Frame time = 1000 / FPS (stored at `this+0x168`)
- Skip frames if elapsed < frame_time - 5ms
- Force update after 10 consecutive skips (cap at ~10 physics updates per render)
- Global time limit at 0x5341E4 for FPS counter reset

## Frame Rate Control
- `1000 / this->fps` gives ms per frame (typically 16ms for 60fps)
- Skip tolerance: `frame_time - 5` ms (e.g., 11ms at 60fps)
- Maximum 10 skip updates before forcing a render
- Delta time capped at 1000ms to prevent huge physics jumps