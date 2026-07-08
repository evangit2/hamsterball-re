# Hamsterball Menu System

## Overview

Hamsterball's menu system is built on a class hierarchy: **Gadget** → **SimpleMenu** → concrete menus (OptionsMenu, RemapKeyboardMenu, MainMenu, TourneyMenu, etc.). All menu objects live in the same MeshWorld object list as level objects and are managed by the same per-frame update loop (`GameUpdate` at `0x469CF0`).

Menus are NOT special-cased by the engine — they are regular game objects that happen to render UI elements and process keyboard/mouse input. The same `Scene_AddObject` call that adds a menu also adds level geometry, score displays, and dialog boxes.

## Key Functions

| Address | Name | Calling Convention | Description |
|---------|------|-------------------|-------------|
| `0x469CF0` | `GameUpdate` | `__fastcall(int meshworld)` | Per-frame loop: updates all objects, destroys flagged ones |
| `0x469990` | `Scene_AddObject` | `__thiscall(MeshWorld *this, GameObject *obj)` `RET 4` | Adds any object to the MeshWorld list |
| `0x4699D0` | `MeshWorld_RemoveObject` | `__thiscall(MeshWorld *this, GameObject *obj)` | Removes an object from the list |
| `0x419740` | `Gadget_SetDestroyFlag` | `__fastcall(int this)` | Sets `this+0x2C = 1` (marks for destruction) |
| `0x4690F0` | `Gadget_ctor` | `__thiscall(void *this, int app)` | Base Gadget constructor |
| `0x448F20` | `SimpleMenu_ctor` | `__thiscall(void *this, int app)` | Base menu constructor (calls Gadget_ctor) |
| `0x4431E0` | `RemapKeyboardMenu_ctor` | `__thiscall(void *this, int app, void *parent)` `RET 0xC` | Constructs the key remapping submenu |
| `0x442CE0` | `OptionsMenu_ctor` | `__thiscall(void *this, int app, char *back_dest)` `RET 0x8` | Constructs the options menu |
| `0x4434F0` | `OptionsMenu_OnItemSelected` | `__thiscall(int *this, char *item_name)` | Handles item selection in Options menu |
| `0x442830` | `RemapKeyboard_OnItemSelected` | `__thiscall(int *this, char *item_name)` | Handles item selection in Remap menu |
| `0x441800` | `OptionsMenu_Render` | `__thiscall(void *this, void *gfx)` | Renders Options menu (skips if `+0xE09`) |
| `0x449B00` | `UIList_Cleanup` | `__fastcall(undefined4 *this)` | Frees UIList items, strings, calls SceneObject_dtor |
| `0x4431C0` | `UIList_DeletingDtor` | `__thiscall(void *this, byte flags)` `RET 4` | Calls UIList_dtor then `_free(this)` |
| `0x443430` | `RemapKeyboard_Update` | `__fastcall(int *this)` | Per-frame update: scans keyboard during remap mode |
| `0x442AF0` | `RemapKeyboard_UpdateKeyText` | `__fastcall(void *this)` | Refreshes item labels with current key binding names |

## Class Hierarchy & Vtables

```
Gadget (vtable @ 0x4D6A70, ctor @ 0x4690F0)
  └─ SimpleMenu (ctor @ 0x448F20, calls Gadget_ctor)
       ├─ OptionsMenu     (vtable @ 0x4D5E30, ctor @ 0x442CE0, alloc 0xE0C)
       ├─ RemapKeyboardMenu (vtable @ 0x4D5F50, ctor @ 0x4431E0, alloc 0xCEC)
       ├─ MainMenu         (ctor @ 0x42DE50, alloc 0xCDC)
       ├─ TourneyMenu      (ctor alloc 0x111C)
       ├─ QuitRaceMenu     (alloc 0xCDC)
       ├─ PauseArenaMenu   (ctor alloc 0xCDC)
       └─ QuitDialog       (render @ 0x443FC0)
```

### Gadget_ctor (0x4690F0) — Base Initialization

```c
void __thiscall Gadget_ctor(void *this, int app) {
    this->vtable = 0x4D9170;          // base SceneObject vtable
    this->scene_list = app;           // +0x14 = App pointer
    this->destroy_flag = 0;           // +0x2C
    this->parent_meshworld = 0;       // +0x30
    this->object_list_1 = {0};       // +0x34..0x44 (AthenaList_Init)
    this->object_list_2 = {0};       // +0x44C (AthenaList_Init)
    this->active_child = 0;          // +0x864
    this->name = "Generic Gadget";   // +0x868
    this->pause_flag = 0;            // +0x874
}
```

### SimpleMenu_ctor (0x448F20) — Menu Base Constructor

SimpleMenu extends Gadget by adding:
- A second vtable override (`0x4D6A70` — the Gadget/UIList vtable)
- An item list (AthenaList at `+0x88C`)
- Two AthenaString objects for text rendering (`+0xCA4`, `+0xCA8`)
- UI layout parameters (scroll position, item width, cursor state)

```c
void __thiscall SimpleMenu_ctor(void *this, int app) {
    Gadget_ctor(this, app);
    this->vtable = 0x4D6A70;          // Gadget/UIList vtable
    AthenaList_Init(this + 0x88C);    // item list
    this->name = "Simple Menu";       // +0x868
    this->app = app;                  // +0x878
    // ... layout defaults, scroll state, cursor ...
    this->title = NULL;               // +0x888
    // allocate two AthenaString objects for text measurement
    this->str_a = AthenaString_Init(operator_new(0x1C));  // +0xCA4
    this->str_b = AthenaString_Init(operator_new(0x1C));  // +0xCA8
}
```

### Complete Vtable Comparison

All three vtables share 22 entries. Overridden slots are marked with **bold**:

| Slot | Offset | Gadget (0x4D6A70) | OptionsMenu (0x4D5E30) | RemapKeyboard (0x4D5F50) | Purpose |
|------|--------|-------------------|------------------------|---------------------------|---------|
| [0] | +0x00 | `0x44AC80` | **`0x4431C0`** | **`0x443410`** | DeletingDtor |
| [1] | +0x04 | `0x4494D0` | **`0x441E70`** | **`0x443430`** | Update (per-frame) |
| [2] | +0x08 | `0x449D40` | **`0x441800`** | `0x449D40` | Render |
| [3] | +0x0C | `0x449C20` | `0x449C20` | **`0x442CA0`** | HandleKeyNav |
| [4] | +0x10 | `0x469220` | `0x469220` | `0x469220` | UpdateChild (calls child vtable[4]) |
| [5] | +0x14 | `0x449750` | `0x449750` | **`0x442CC0`** | ActivateCurrentItem |
| [6] | +0x18 | `0x469280` | `0x469280` | `0x469280` | DeactivateChild |
| [7] | +0x1C | `0x409D90` | `0x409D90` | `0x409D90` | NOP (empty return) |
| [8] | +0x20 | `0x4499D0` | `0x4499D0` | **`0x442AD0`** | Dtor body (cleanup before free) |
| [9] | +0x24 | `0x44B840` | `0x44B840` | `0x44B840` | NOP |
| [10] | +0x28 | `0x44B840` | `0x44B840` | `0x44B840` | NOP |
| [11] | +0x2C | `0x4692A0` | `0x4692A0` | `0x4692A0` | Shared stub |
| [12] | +0x30 | `0x4692A0` | `0x4692A0` | `0x4692A0` | Shared stub |
| [13] | +0x34 | `0x44B840` | `0x44B840` | `0x44B840` | NOP |
| [14] | +0x38 | `0x4693C0` | `0x4693C0` | `0x4693C0` | OnAdd (Scene_AddAllObjects) |
| [15] | +0x3C | `0x469430` | `0x469430` | `0x469430` | OnRemove (calls MeshWorld_RemoveObject) |
| [16] | +0x40 | `0x419740` | `0x419740` | `0x419740` | **SetDestroyFlag** (writes `+0x2C = 1`) |
| [17] | +0x44 | `0x4692B0` | `0x4692B0` | `0x4692B0` | Shared stub |
| [18] | +0x48 | `0x44B840` | **`0x4434F0`** | **`0x442830`** | **OnItemSelected** |
| [19] | +0x4C | `0x486F20` | **`0x442680`** | `0x486F20` | AdjustSlider |
| [20] | +0x50 | `0x44A570` | `0x44A570` | `0x44A570` | Layout (UIList_Layout) |
| [21] | +0x54 | `0x44B840` | `0x44B840` | `0x44B840` | NOP |

### Key vtable function details

**vtable[16] — `Gadget_SetDestroyFlag` (0x419740)** — The most critical function. Only 5 bytes of code:
```c
void __fastcall Gadget_SetDestroyFlag(int this) {
    *(byte *)(this + 0x2C) = 1;  // destroy_flag = 1
}
```
GameUpdate checks this flag every frame. When set, the object is removed from the MeshWorld list and freed.

**vtable[4] — `UpdateChild` (0x469220)** — If the object has an active child (`+0x864 != 0`), calls `child->vtable[0x10]()` (SetDestroyFlag on the child). This is used for cascading destruction.

**vtable[8] — Navigation handler (0x4499D0)** — Handles Up/Down arrow key navigation through the UI list. Scans for non-null items, wraps around. Param `0x40E` = Up, `0x410` = Down, `0x40D` = Left, `0x40F` = Right.

**vtable[14] — `Scene_AddAllObjects` (0x4693C0)** — Called by Scene_AddObject as the OnAdd callback. Iterates the object's own child list and adds each child to the MeshWorld.

**vtable[15] — OnRemove (0x469430)** — Mirror of vtable[14]. Called during destruction. Iterates children and calls `MeshWorld_RemoveObject` on each.

**vtable[3] — `RemapKeyboard_HandleKeyNav` (0x442CA0)** — Wraps the base `UIList_HandleKeyNav` but blocks navigation when `+0xCE0` (remap-in-progress flag) is set:
```c
void __thiscall RemapKeyboard_HandleKeyNav(void *this, int param_1, int param_2) {
    if (this->ce0 == 0) {  // not in remap mode
        UIList_HandleKeyNav(this, param_1, param_2);
    }
    // else: swallow input
}
```

**vtable[5] — `RemapKeyboard_Activate` (0x442CC0)** — Same pattern: blocks item activation when remap is in progress:
```c
void __thiscall RemapKeyboard_Activate(void *this, int p1, int p2, int p3) {
    if (this->ce0 == 0) {
        UIList_ActivateCurrentItem(this, p1, p2, p3);
    }
}
```

## Object Lifecycle

### Creating a Submenu (OptionsMenu → RemapKeyboard)

When the user selects "REMAP KEYBOARD CONTROLS" in the Options menu, `OptionsMenu_OnItemSelected` (vtable[18] = `0x4434F0`) fires:

```c
// __stricmp(param_2, "REMAP") == 0:
void *mem = operator_new(0xCEC);                    // 1. Allocate
RemapKeyboardMenu *menu = RemapKeyboardMenu_ctor(   // 2. Construct
    mem,                // this
    this->app,          // App pointer (this+0x878)
    this                 // parent menu (for un-hiding on BACK)
);
Scene_AddObject(                                    // 3. Register
    this->app->meshworld,  // App+0x184
    menu
);
this->e09 = 1;                                      // 4. Hide parent (skip rendering)
```

**RemapKeyboardMenu_ctor (0x4431E0)** does:
1. Calls `SimpleMenu_ctor(this, app)` — initializes Gadget base, AthenaLists, UI infrastructure
2. Sets `this->vtable = 0x4D5F50` (RemapKeyboardMenu vtable)
3. Sets `this->parent = parent` at `+0xCDC` (parent OptionsMenu pointer for later un-hiding)
4. Sets title: `this->title = "Remap Keyboard"` (`+0x888`), `this->name = "Remap Menu"` (`+0x868`)
5. Sets item count: `this->cac = 0x15E` (350 — default item width in pixels)
6. Adds UI items via `UIList_AddItem(this, display_text, name_id, matrix, callback, flags)`:

   | Display Text | Name ID | Key Binding Offset | Notes |
   |-------------|---------|---------------------|-------|
   | "Key Up:" | `UP` | KeyboardDevice+0x50C | |
   | "Key Down:" | `DOWN` | KeyboardDevice+0x510 | |
   | "Key Left:" | `LEFT` | KeyboardDevice+0x514 | |
   | "Key Right:" | `RIGHT` | KeyboardDevice+0x518 | |
   | *spacer* | | | 5px gap |
   | "Key Action1:" | `ACTION1` | KeyboardDevice+0x51C | Sets App+0x560 = 0x32 (50-frame delay) |
   | "Key Action2:" | `ACTION2` | KeyboardDevice+0x520 | Sets App+0x560 = 0x32 (50-frame delay) |
   | *spacer* | | | 10px gap |
   | "BACK" | `BACK` | | Scaled 0.75× (smaller) |

7. Initializes remap state: `+0xCE0 = 0` (not remapping), `+0xCE4 = 0`, `+0xCE8 = 0`
8. Calls `RemapKeyboard_UpdateKeyText(this)` (0x442AF0) — refreshes item labels with current key binding names

**RemapKeyboard_UpdateKeyText (0x442AF0)** — For each key binding:
1. Reads the DIK code from `KeyboardDevice[key_binding_offset]` (e.g., `+0x50C` for UP)
2. Looks up the key name string: `KeyboardDevice[0x10C + dik_code * 4]` → string pointer
3. Formats via `AthenaString_SprintfToBuffer(buf, "Key Up:           %s", key_name)`
4. Updates the UI item: `UIList_SetTextByName(this, buf, "UP")`

### Destroying a Submenu (RemapKeyboard BACK)

When the user selects "BACK" in the RemapKeyboard menu, `RemapKeyboard_OnItemSelected` (vtable[18] = `0x442830`) fires:

```c
// __stricmp(param_2, "BACK") == 0:
this->vtable[0x10]();                    // 1. SetDestroyFlag → this+0x2C = 1
parent->e09 = 0;                         // 2. Un-hide parent menu
// parent pointer accessed as this[0x337] = *(int*)(this + 0xCDC)
```

For key items (UP, DOWN, LEFT, RIGHT, ACTION1, ACTION2), the handler:
1. Updates the item text to "...PRESS KEY..."
2. Sets `this->ce0 = 1` (enter remap mode — blocks navigation/activation)
3. Stores the target key binding address in `this->ce4` (e.g., `KeyboardDevice+0x50C` for UP)
4. Sets `this->ce8 = 1` (waiting for key press)
5. For ACTION1/ACTION2: also sets `App+0x560 = 0x32` (50-frame delay before capture)

### Key Remap Input Scanning

When remap mode is active (`+0xCE0 = 1`), `RemapKeyboard_Update` (vtable[1] = `0x443430`) runs instead of the normal `UIList_ScrollUpdate`:

```c
void RemapKeyboard_Update(int *this) {
    if (this->ce0 == 0) {           // not in remap mode
        UIList_ScrollUpdate(this);  // normal navigation
        return;
    }
    // In remap mode — scan keyboard state buffer
    byte *keystate = KeyboardDevice + 0x0C;  // 256-byte DirectInput buffer
    
    if (this->ce8 != 0) {           // Phase 1: wait for key release
        this->ce8 = 0;
        for (int i = 0; i < 256; i++) {
            if (keystate[i] & 0x80) {  // key still held
                this->ce8 = 1;         // keep waiting
                return;
            }
        }
        return;
    }
    // Phase 2: wait for new key press
    for (int i = 0; i < 256; i++) {
        if (keystate[i] & 0x80) {      // key pressed!
            *(int*)this->ce4 = i;       // store DIK code in target binding
            this->ce0 = 0;              // exit remap mode
            RemapKeyboard_UpdateKeyText(this);  // refresh labels
            App->input_delay = 0x32;   // 50-frame delay
            return;
        }
    }
}
```

### GameUpdate Per-Frame Loop (0x469CF0)

The main update function runs two passes over the MeshWorld object list each frame:

**Pass 1 — Update (skip destroyed/paused objects):**
```c
for each obj in meshworld->object_list:
    if (obj->destroy_flag == 0 && obj->pause_flag == 0):
        // +0x2C and +0x874 both must be 0
        meshworld->app->current_id = obj->id;  // for debugging
        obj->vtable[1]()   // call Update
```

**Pass 2 — Destroy flagged objects:**
```c
for each obj in meshworld->object_list:
    if (obj->destroy_flag != 0):   // +0x2C set
        meshworld->app->debug_label = "Remove Object"
        
        // Clean up active-object references
        if (meshworld->current_obj == obj):
            obj->vtable[0xC]()     // deactivate
            meshworld->current_obj = 0
        
        if (meshworld->secondary_obj == obj):
            meshworld->secondary_obj = 0
        
        if (meshworld->app->hovered_obj == obj):
            meshworld->app->vtable[0x18]()  // clear hover
        
        // Call OnRemove
        obj->vtable[0xF](meshworld)
        
        // Update active object pointer
        if (meshworld->active_obj == obj):
            meshworld->active_obj = meshworld->object_list[0]  // first remaining
        
        // Remove from AthenaList
        AthenaList_Remove(&meshworld->object_list, obj)
        obj->parent = 0
        
        // Scene cleanup callback
        meshworld->app->vtable[0x1D]()
        
        // OnRemove callback
        obj->vtable[0xF](meshworld)
        
        // Delete + free (vtable[0] with free flag = 1)
        obj->vtable[0](1)    // DeletingDtor → dtor + _free(this)
        
        meshworld->app->debug_label = "Update"
```

## Key Offsets

### Gadget base (set by Gadget_ctor at 0x4690F0)

| Offset | Type | Name | Description |
|--------|------|------|-------------|
| +0x00 | `void**` | vtable | Virtual function table pointer (22 entries, 88 bytes) |
| +0x04 | `AthenaList` | child_list_1 | First child object list (initialized by AthenaList_Init) |
| +0x14 | `void*` | app | App pointer (passed to ctor as param_1) |
| +0x2C | `byte` | destroy_flag | Set to 1 by vtable[16] → GameUpdate destroys object next frame |
| +0x30 | `void*` | parent_meshworld | Set by Scene_AddObject to the MeshWorld that owns this object |
| +0x864 | `void*` | active_child | Current child object being interacted with (or NULL) |
| +0x868 | `char*` | name | Internal name (e.g. "Generic Gadget", "Options Menu", "Remap Menu") |
| +0x870 | `void*` | app_field | Copied from App+0x1DC during Gadget_ctor |
| +0x874 | `byte` | pause_flag | When set, GameUpdate skips calling Update on this object |

### SimpleMenu additions (set by SimpleMenu_ctor at 0x448F20)

| Offset | Type | Name | Description |
|--------|------|------|-------------|
| +0x44C | `AthenaList` | child_list_2 | Second child object list (UI elements, sprites) |
| +0x878 | `void*` | app | App pointer (same as Gadget +0x14, duplicated for convenience) |
| +0x87C | `void*` | font | Font pointer (copied from App+0x318 during ctor) |
| +0x884 | `int` | visible | Visibility flag (1 = visible) |
| +0x888 | `char*` | title | Display title text (e.g. "OPTIONS", "Remap Keyboard") |
| +0x88C | `AthenaList` | items | UI list items (UIListItem pointers) |
| +0xCAC | `int` | item_width | Default item width (0x15E = 350 pixels) |
| +0xCB0 | `int` | max_item_width | Widest item (updated during layout) |
| +0xCB4 | `int` | total_item_height | Sum of all item heights |
| +0xCBC | `byte` | needs_layout | Set to 1 when items change (triggers UIList_Layout) |
| +0xCA4 | `AthenaString*` | str_a | Text measurement string object 1 |
| +0xCA8 | `AthenaString*` | str_b | Text measurement string object 2 |
| +0xCC0 | `void*` | current_item | Currently selected UIListItem pointer |
| +0xCC8 | `int` | scroll_max | Maximum scroll position |
| +0xCCC | `int` | scroll_pos | Current scroll position |
| +0xCD0 | `byte` | keyboard_nav | Whether keyboard navigation is enabled |

### OptionsMenu-specific (alloc size 0xE0C)

| Offset | Type | Name | Description |
|--------|------|------|-------------|
| +0xCDC | `char*` | back_destination | String identifying where BACK should go (NOT a pointer — see below) |
| +0xCE0 | `byte` | resolution_changed | |
| +0xCE1 | `byte` | fullscreen | |
| +0xCE4 | `int` | resolution_x | |
| +0xCE8 | `int` | resolution_y | |
| +0xCEC | `byte` | safe_mode | |
| +0xCED | `byte` | color_quality | |
| +0xE08 | `byte` | layout_initialized | Set to 1 after first layout pass |
| +0xE09 | `byte` | submenu_active | 1 = submenu is active, skip rendering this menu |

### RemapKeyboardMenu-specific (alloc size 0xCEC)

| Offset | Type | Name | Description |
|--------|------|------|-------------|
| +0xCDC | `void*` | parent_menu | Pointer to parent OptionsMenu (for un-hiding on BACK) |
| +0xCE0 | `byte` | remap_in_progress | 1 = currently capturing a key press (blocks nav + activation) |
| +0xCE4 | `int*` | target_binding | Pointer to KeyboardDevice key binding DWORD being remapped |
| +0xCE8 | `byte` | waiting_for_release | 1 = waiting for previous key to be released before capture |

> **Note on +0xCDC dual usage:** In OptionsMenu, `+0xCDC` stores a `char*` string like `"MAIN"` or `"PAUSETT"` that determines which menu to create on BACK. In RemapKeyboardMenu, `+0xCDC` stores a `void*` pointer to the parent OptionsMenu object. The offset is reused with different types depending on the menu class.

## OptionsMenu BACK Destinations

The `+0xCDC` field on OptionsMenu stores a string identifying where to navigate when BACK is pressed. All 6 known values (verified from call sites that call `OptionsMenu_ctor`):

| Back-Destination String | Allocation | Constructor Called | Context |
|------------------------|------------|-------------------|---------|
| `"MAIN"` | 0xCDC | `MainMenu_ctor` (0x42DE50) | Options opened from main menu |
| `"PAUSETOURNEY"` | 0xCDC | `FUN_0042e4b0` | Pause menu during tournament race |
| `"PAUSEPRACTICE"` | 0xCDC | `QuitRaceMenu` | Pause menu during practice race |
| `"PAUSETT"` | 0xCDC | `QuitRace` (0x42E5B0 area) | Pause menu during time trial |
| `"PAUSERR"` | 0xCDC | `PauseArenaMenu_ctor` | Pause menu during rodent rumble |
| `"TOURNAMENT"` | 0x111C | `TourneyMenu_ctor` | Options opened from tournament menu |

When BACK is pressed in OptionsMenu:
1. Calls `FUN_00441700` — saves settings (resolution, fullscreen, color quality, safe mode) to App struct
2. Calls `this->vtable[0x10]()` → SetDestroyFlag (`+0x2C = 1`) — marks self for destruction
3. Reads `this->cdc` as a string pointer
4. Compares against the 6 known strings
5. Allocates and constructs the matching menu
6. Calls `Scene_AddObject` to register the new menu

## The +0xE09 Flag (Submenu Hide)

The `+0xE09` flag controls whether a menu renders itself. It is checked in **OptionsMenu_Render** (vtable[2] = `0x441800`):

```c
void OptionsMenu_Render(void *this, void *gfx) {
    if (this->e09 == 0) {           // only render if no submenu active
        UIList_Render(this, gfx);   // base UIList render (items, scroll bar)
        // ... draw resolution text, sliders, color quality, etc.
    }
    if (this->e08 == 0) {           // first-time layout
        // ... initial layout pass (set slider positions, etc.)
        this->e08 = 1;
    }
}
```

When OptionsMenu creates the RemapKeyboard submenu, it sets `this+0xE09 = 1`, causing the OptionsMenu to stop rendering while the submenu is visible. When RemapKeyboard's BACK is pressed, it clears `parent+0xE09 = 0`, restoring the parent's rendering.

**Important:** The parent menu is NOT destroyed — it stays in the MeshWorld object list and still receives Update calls. It just skips rendering. Input is handled by checking `MeshWorld+0x424` (the "active object" pointer), which determines which object receives keyboard/mouse events.

## What 0x469990 (Scene_AddObject) Does

`Scene_AddObject` is the universal object registration function. It is called for **everything** — menus, level objects, score displays, dialog boxes, etc. (50+ call sites in the binary).

```c
void __thiscall Scene_AddObject(MeshWorld *this, GameObject *obj) {
    if (!AthenaList_ContainsValue(&this->object_list, obj)) {
        AthenaList_Append(&this->object_list, obj);
        obj->parent_meshworld = this;           // +0x30
        App->vtable[0x1D](app);                  // update mouse hover state
        obj->vtable[0xE](this);                 // OnAdd callback (Scene_AddAllObjects)
    }
}
```

Disassembly (verified):
```asm
MOV ECX, [EDI + 0x844]    ; ECX = MeshWorld->app (App pointer)
MOV EAX, [ECX]             ; EAX = App->vtable
CALL [EAX + 0x74]          ; App->vtable[0x1D] (update hover)
MOV EDX, [ESI]             ; EDX = obj->vtable
PUSH EDI                   ; push MeshWorld*
MOV ECX, ESI               ; ECX = obj (this)
CALL [EDX + 0x38]          ; obj->vtable[0xE] (OnAdd)
```

- `this` = MeshWorld (stored at `App+0x184`, allocated as 0x848 bytes in App_Ctor)
- `obj` = any game object (menu, level object, etc.)
- `MeshWorld+0x04` = AthenaList of all objects
- `obj+0x30` = set to MeshWorld pointer (parent)
- `App->vtable[0x1D]` (`0x46C8E0`) updates mouse hover state (converts cursor position to object)
- `obj->vtable[0xE]` (offset `+0x38`) = OnAdd callback (calls `Scene_AddAllObjects` which adds child objects)

## MeshWorld Struct Layout

The MeshWorld (scene manager) is allocated in `App_Ctor` and stored at `App+0x184`:

| Offset | Type | Name | Description |
|--------|------|------|-------------|
| +0x00 | `void*` | vtable_ptr | Points to deleting destructor (not a traditional vtable) |
| +0x04 | `AthenaList` | object_list | All game objects (menus, level geometry, dialogs) |
| +0x08 | `int` | object_count | Number of objects in list |
| +0x0C | `int[]` | iterators | AthenaList internal iterator array |
| +0x410 | `void**` | data_array | AthenaList internal data pointer |
| +0x41C | `void*` | current_obj | Current object for update/render dispatch |
| +0x420 | `void*` | secondary_obj | Secondary object pointer (purpose TBD) |
| +0x424 | `void*` | active_obj | Active object (receives keyboard/mouse input) |
| +0x428 | `SceneList` | scene_list | Level geometry/scene objects |
| +0x844 | `void*` | app | App pointer (back-reference, set in ctor) |

## Keyboard Device & Key Bindings

The keyboard device is accessed via: `App+0x180 → InputDevice → +0x434 → KeyboardDevice*`

| Offset | Size | Name | Description |
|--------|------|------|-------------|
| +0x08 | 4 | `dinput_device` | DirectInput8Device COM interface pointer |
| +0x0C | 256 | `keystate` | Live keyboard state (0x80 bit = key pressed) |
| +0x10C | 1024 | `key_names` | 256 × 4-byte string pointers (key name lookup table) |
| +0x50C | 4 | `bind_up` | DIK code for UP (e.g., 0xC8 = DIK_UP) |
| +0x510 | 4 | `bind_down` | DIK code for DOWN |
| +0x514 | 4 | `bind_left` | DIK code for LEFT |
| +0x518 | 4 | `bind_right` | DIK code for RIGHT |
| +0x51C | 4 | `bind_action1` | DIK code for ACTION1 |
| +0x520 | 4 | `bind_action2` | DIK code for ACTION2 |

Key name resolution: `key_names[dik_code]` returns a `char*` string (e.g., `"UP ARROW"`, `"SPACE BAR"`).

`InputDevice_PollAndRelease` (0x46EBD0) calls `IDirectInputDevice8::GetDeviceState(256, &keystate)` each frame to update the keyboard buffer.

## UIList Item System

### UIList_AddItem (0x4497F0)

```c
void __thiscall UIList_AddItem(
    void *this,
    char *display_text,    // visible label (heap-allocated copy)
    char *name_id,         // internal identifier (heap-allocated copy)
    float matrix[16],     // transform matrix (usually identity)
    void *callback,       // callback function pointer (or NULL)
    int flags             // 0 = normal, affects layout
);
```

Each call:
1. Allocates a `UIListItem` (0x444 bytes) via `operator_new`
2. Copies `display_text` and `name_id` to heap
3. Creates an `AthenaString` for text measurement
4. Stores callback at `UIListItem+0x1C`
5. Appends to `this->items` (AthenaList at `+0x88C`)
6. Updates `this->max_item_width` if this item is wider

### UIList_ActivateCurrentItem (0x449750)

When the user presses Enter or clicks an item:
1. Calls `Gadget_Activate` (0x469240) — forwards to active child if one exists
2. If the current item is the scroll-up or scroll-down handle, scrolls by 0x28A (650) or 0x32 (50) pixels
3. Otherwise, calls `this->vtable[0x48]` (OnItemSelected) with the item's `name_id` string:
   ```c
   this->vtable[0x18]( *(char**)(current_item + 0x04) );  // name_id string
   ```

### UIList_Cleanup (0x449B00)

Called during destruction. For each UIListItem:
1. If item has a callback object (`+0x1C`), calls its vtable[0] with flag=1 (destroy)
2. Frees the display text string (`item[0]`)
3. Frees the name_id string (`item[1]`)
4. Frees the Vec3List (`item+0x28`)
5. Frees the UIListItem itself

Then:
6. Clears the AthenaList at `+0x88C`
7. If `+0xCA4` (str_a) is non-null, calls its vtable[0] with flag=1
8. If `+0xCA8` (str_b) is non-null, calls its vtable[0] with flag=1
9. Calls `SceneObject_dtor` (base class destructor)

## Is It Safe to Create New Menus This Way?

**Yes — this is exactly how the game does it.** The pattern is:

1. `operator_new(size)` — allocate memory
2. Call constructor (sets vtable, adds UI items, initializes state)
3. `Scene_AddObject(App+0x184, menu)` — register with MeshWorld
4. Set parent's `+0xE09 = 1` — hide parent (if applicable)
5. When done: call `vtable[0x10]()` (sets `+0x2C = 1`), clear parent's `+0xE09 = 0`

GameUpdate will automatically destroy the menu on the next frame after `+0x2C` is set, calling the DeletingDtor which frees all UIList items, AthenaStrings, and the memory itself.

### Creating a New Menu: Minimal Example

```c
// Allocate
MyMenu *menu = operator_new(sizeof_MyMenu);

// Construct (your custom ctor)
MyMenu_ctor(menu, app, parent_menu);

// Register
Scene_AddObject(app->meshworld, menu);  // app->meshworld = *(void**)(app + 0x184)

// Hide parent
parent_menu->e09 = 1;  // byte at parent + 0xE09
```

Your custom constructor must:
1. Call `SimpleMenu_ctor(this, app)` first
2. Set `this->vtable = my_menu_vtable` (your vtable, 22 entries)
3. Set `this+0xCDC = parent` (so BACK can un-hide parent)
4. Add items via `UIList_AddItem(this, label, name_id, matrix, callback, flags)`
5. Set `this+0x888 = "My Menu"` (title)
6. Set `this+0x868 = "MyMenu"` (internal name)

Your OnItemSelected handler (vtable[18]) must:
1. Check `__stricmp(param_2, "BACK") == 0`
2. If BACK: call `this->vtable[0x10]()` (destroy flag), set `parent->e09 = 0`
3. Handle other items as needed

### Hooking 0x4431E0 vs Creating New Menus

**Hooking the RemapKeyboardMenu constructor (0x4431E0)** is simpler if you just want to:
- Add more remappable keys (add more `UIList_AddItem` calls)
- Change labels or layout
- Modify existing menu behavior

**Creating new menus via the allocate→construct→Scene_AddObject pattern** is better if you want:
- Entirely new menu screens
- Menus accessible from different places
- Custom menu logic that doesn't fit the remap menu structure

Both approaches are safe. The game's own code uses the same pattern for every menu transition (Options→Remap, Options→MainMenu, Pause→QuitRace, etc.).

## Summary

The menu system is a straightforward allocate→register→destroy pattern:

| Phase | What Happens | Key Functions |
|-------|-------------|---------------|
| **Create** | `operator_new` → ctor → `Scene_AddObject` → set parent's `+0xE09` | ctor, `0x469990` |
| **Run** | GameUpdate calls `vtable[1]` (Update) and `vtable[2]` (Render) each frame | `0x469CF0` |
| **Destroy** | `vtable[0x10]` sets `+0x2C` → GameUpdate removes from list, calls `vtable[0](1)` (free) | `0x419740`, `0x469CF0` |

- **0x469990** (`Scene_AddObject`): universal object registration, 50+ call sites
- **0x4431E0** (`RemapKeyboardMenu_ctor`): specific constructor for the key remap submenu
- **0x419740** (`Gadget_SetDestroyFlag`): 5-byte function that marks any object for destruction
- **0x469CF0** (`GameUpdate`): two-pass loop — update all alive objects, then destroy all flagged ones
- Both approaches (hooking vs new menus) are safe and used by the game itself
