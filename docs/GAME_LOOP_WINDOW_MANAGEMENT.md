# Game Loop & Window Management

## Overview
The game loop (App_Run at 0x46BD80) implements a fixed-timestep update with
variable rendering. The WinMain (0x4278E0) sets up the window and enters
the main loop.

## App_Run — Game Loop (0x46BD80)

```c
void App_Run(App *app) {
    Timer_Init(timer);
    fps_target = app->fps_divisor;  // +0x170 — frames per second divisor
    frame_time = 0;
    last_tick = 0;
    last_render_tick = 0;
    app->last_tick = GetTickCount();
    
    while (!app->quit_flag) {  // +0x159 = quit flag
        frame_count = 0;
        Sleep(0);  // Yield to other processes
        app->phase_name = "Background";  // +0x210
        
        // Calculate milliseconds per frame
        app->ms_per_frame = 1000 / app->fps_divisor;  // +0x168
        
        // FPS counter update (every 1 second)
        now = GetTickCount();
        if (fps_counter_target < now) {
            if (app->show_fps) {  // +0x1AC = show FPS flag
                AthenaString_SprintfToBuffer(app->fps_string, format);
            }
            app->frame_counter = 0;  // +0x194
            fps_counter_target = now + 1000;
        }
        
        // Process Windows messages
        while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (app->quit_flag) goto cleanup;
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
        
        if (app->quit_flag) break;
        
        // Fixed-timestep update
        do {
            now = GetTickCount();
            
            // If we're ahead of schedule OR too many catch-up frames
            if ((now - app->last_tick) < (app->ms_per_frame - 5) || 
                catchup_frames > 9) {
                app->phase_name = "Render";
                catchup_frames = 0;
                
                // Only render if enough time has passed
                if ((1000 / fps_target) - 5 + last_render_tick < now) {
                    if (app->gfx != NULL && 
                        (!app->minimized || !app->active)) {
                        app->frame_counter++;
                        Graphics_BeginFrame(app->gfx, timer);
                        app->vtable[0x24]();  // Pre-render
                        app->vtable[0x28]();  // Render
                        app->vtable[0x2C]();  // Post-render
                        Graphics_PresentOrEnd(app->gfx, TRUE);
                    }
                    last_render_tick = GetTickCount();
                }
                break;
            }
            
            // Logic update (fixed timestep)
            app->update_count++;  // +0x18C
            app->phase_name = "Update";
            Graphics_BeginFrame(app->gfx, timer);
            app->vtable[0x20]();  // Update (game logic)
            
            // Advance tick counter
            prev_tick = app->last_tick;
            app->last_tick = prev_tick + app->ms_per_frame;
            
            // Prevent tick drift (clamp to 1 second behind)
            if (1000 < (GetTickCount() - (prev_tick + app->ms_per_frame))) {
                app->last_tick = GetTickCount() - 1000;
            }
            
            frame_count++;
        } while (frame_count < 1);  // Max 1 update per frame
    }
    
cleanup:
    Timer_Cleanup(timer);
}
```

## App Struct — Game Loop Offsets

| Offset | Type | Description |
|--------|------|-------------|
| +0x159 | byte | Quit flag (non-zero = exit loop) |
| +0x15A | byte | Minimized flag |
| +0x16C | byte | Show FPS flag |
| +0x18C | int | Update count (total updates) |
| +0x194 | int | Frame counter (FPS display) |
| +0x164 | int | ms_per_frame (1000 / fps_divisor) |
| +0x168 | int | FPS divisor (target FPS) |
| +0x170 | int | fps_divisor (backup) |
| +0x1A0 | char* | FPS display string (+0x198?) |
| +0x1AC | byte | show_fps (1 = display FPS counter) |
| +0x210 | char* | Phase name ("Background"/"Update"/"Render") |
| +0x174 | GfxEngine* | Graphics engine pointer |

## Vtable Dispatch Calls

The game uses vtable calls for the 4 main update phases:

| Vtable Offset | Call | Description |
|-------------|------|-------------|
| vtable[0x20] | Update | Game logic (physics, input, AI) |
| vtable[0x24] | Pre-render | Pre-render setup (camera, visibility) |
| vtable[0x28] | Render | Draw all scene objects |
| vtable[0x2C] | Post-render | UI overlays, HUD, menus |

## WinMain (0x4278E0)

Entry point:
1. Register window class (Athena class)
2. Create window (1024x768 default, configurable)
3. Initialize D3D8 device via Graphics_CreateDevice
4. Call App_Initialize_Full (all subsystems)
5. Enter App_Run game loop
6. On exit: LoadOrSaveConfig (saves settings/shutdown)

## Window Management

### Window Procedure
Standard Windows message handler:
- WM_PAINT: Render invalidation
- WM_SIZE: Handle resize/minimize
- WM_ACTIVATE: Pause/unpause on focus change
- WM_CLOSE: Set quit flag
- WM_KEYDOWN/WM_KEYUP: Buffered input

### Minimize Handling
When minimized (app->minimized ≠ 0):
- Game logic still updates (vtable[0x20])
- Rendering is skipped (only render when not minimized)
- Sleep(0) yields CPU during background

## Timer System

### Timer_Init / Timer_Cleanup
High-resolution timer using QueryPerformanceCounter:
- Timer_Init: Initialize performance counter
- Timer_Cleanup: Release timer resources

### Frame Timing
- Target: 1000 / fps_divisor ms per frame (typically ~33ms = 30fps)
- Max catch-up frames: 9 (prevents spiral of death)
- Tick drift clamp: max 1 second behind real time
- Render throttled: only when 1000/fps - 5ms have elapsed

## Graphics_BeginFrame (0x453B50)
Called before both update and render:
1. Clear back buffer
2. Begin D3D scene
3. Set default render states
4. Update timer

## Graphics_PresentOrEnd (0x455A90)
Called after render:
1. End D3D scene
2. Present back buffer (swap chain)
3. Reset render state tracker

## Frame Budget

At 30 FPS:
- Update budget: ~33ms per logic tick
- Render budget: ~28ms (33 - 5ms margin)
- Catch-up limit: 9 frames before skipping
- Background: Sleep(0) between frames

## Key App Offsets (Game State)

| Offset | Type | Description |
|--------|------|-------------|
| +0x178 | Scene* | Current scene |
| +0x184 | Scene* | Loading screen scene |
| +0x1DC | SoundChannel* | Input event sound |
| +0x1E4 | InputDev* | Player 1 device |
| +0x1E8 | InputDev* | Player 2 device |
| +0x708 | int | Game state (3 = racing) |
| +0x174 | GfxEngine* | Graphics engine |
| +0x850 | bool | Mirror mode flag |
| +0x86C | uint8[0x50] | Best time data |
| +0x8BC | uint8[0x50] | Medal data |