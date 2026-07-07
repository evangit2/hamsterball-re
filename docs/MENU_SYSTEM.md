# Hamsterball Menu System

## Overview

Hamsterball's menu system is built on a class hierarchy: **Gadget** → **SimpleMenu** → **UIList** → concrete menus (OptionsMenu, RemapKeyboardMenu, MainMenu, TourneyMenu, etc.). All menu objects live in the same MeshWorld object list as level objects and are managed by the same per-frame update loop.

## Key Functions

| Address | Name | Description |
|---------|------|-------------|
| `0x469CF0` | `GameUpdate` | Per-frame loop: updates all objects, destroys flagged ones |
| `0x469990` | `Scene_AddObject` | Adds any object (menus included) to the MeshWorld list |
| `0x4699D0` | `MeshWorld_RemoveObject` | Removes an object from the MeshWorld list |
| `0x419740` | `Gadget_SetDestroyFlag` | Sets `this+0x2C = 1` (marks object for destruction) |
| `0x448F20` | `SimpleMenu_ctor` | Base menu constructor (calls Gadget_ctor, inits AthenaLists) |
| `0x4431E0` | `RemapKeyboardMenu_ctor` | Constructs the key remapping submenu |
| `0x4434F0` | `OptionsMenu_OnItemSelected` | Handles item selection in Options menu (including "REMAP") |
| `0x442830` | `RemapKeyboard_OnItemSelected` | Handles item selection in Remap menu (keys + "BACK") |
| `0x441800` | `OptionsMenu_Render` | Renders Options menu (skips if `+0xE09` flag set) |
| `0x449B00` | `UIList_Cleanup` | Frees UIList items, AthenaStrings, calls SceneObject_dtor |
| `0x4431C0` | `UIList_DeletingDtor` | Calls UIList_dtor then `_free(this)` |

## Class Hierarchy & Vtables

```
Gadget (vtable @ 0x4D6A70)
  └─ SimpleMenu (inherits Gadget vtable, overrides at end of ctor)
       └─ OptionsMenu (vtable @ 0x4D5E30)
       └─ RemapKeyboardMenu (vtable @ 0x4D5F50)
       └─ MainMenu, TourneyMenu, QuitDialog, etc.
```

### Gadget vtable (0x4D6A70) — key entries

| Slot | Offset | Address | Function |
|------|--------|---------|----------|
| [0] | +0x00 | `0x44AC80` | DeletingDtor (dtor + free) |
| [1] | +0x04 | `0x4494D0` | Update (UIList_ScrollUpdate — input + scroll) |
| [2] | +0x08 | `0x449D40` | Render (UIList_Render) |
| [7] | +0x1C | `0x409D90` | NOP (empty return) |
| [16] | +0x40 | `0x419740` | **SetDestroyFlag** — writes `this+0x2C = 1` |
| [18] | +0x48 | `0x44B840` | OnItemSelected (base: NOP) |

### RemapKeyboardMenu vtable (0x4D5F50) — overridden entries

| Slot | Offset | Address | Function |
|------|--------|---------|----------|
| [0] | +0x00 | `0x443410` | DeletingDtor (calls FUN_00442820 + free) |
| [1] | +0x04 | `0x443430` | Update (key remap input scanner) |
| [8] | +0x20 | `0x442AD0` | Destructor body (sets vtable, calls UIList_Cleanup) |
| [16] | +0x40 | `0x419740` | SetDestroyFlag (inherited from Gadget) |
| [18] | +0x48 | `0x442830` | **OnItemSelected** (handles BACK, UP, DOWN, etc.) |

## Object Lifecycle

### Creating a Submenu (OptionsMenu → RemapKeyboard)

When the user selects "REMAP KEYBOARD CONTROLS" in the Options menu:

**OptionsMenu_OnItemSelected (0x4434F0)** — `__stricmp(param_2, "REMAP") == 0`:

```c
// 1. Allocate
void *mem = operator_new(0xCEC);  // 0xCEC = sizeof(RemapKeyboardMenu)

// 2. Construct
RemapKeyboardMenu *menu = FUN_004431e0(mem, app, this /* parent */);

// 3. Register with scene
Scene_AddObject(app->meshworld /* App+0x184 */, menu);

// 4. Hide parent (stop rendering)
this->e09 = 1;  // OptionsMenu checks this in its Render function
```

**RemapKeyboardMenu_ctor (0x4431E0)** does:
1. Calls `SimpleMenu_ctor(this, app)` — initializes Gadget base, AthenaLists, UI infrastructure
2. Sets vtable to `0x4D5F50` (RemapKeyboardMenu vtable)
3. Sets `this+0xCDC = parent` (stores parent menu pointer for later un-hiding)
4. Sets title: `this+0x888 = "Remap Keyboard"`, `this+0x868 = "Remap Menu"`
5. Adds UI items via `UIList_AddItem`:
   - "Key Up:" / "UP"
   - "Key Down:" / "DOWN"
   - "Key Left:" / "LEFT"
   - "Key Right:" / "RIGHT"
   - "Key Action1:" / "ACTION1"
   - "Key Action2:" / "ACTION2"
   - "BACK" / "BACK"
6. Calls `FUN_00442af0(this)` — updates item text with current key bindings via `AthenaString_SprintfToBuffer` + `UIList_SetTextByName`

**Scene_AddObject (0x469990)** does:
```c
void Scene_AddObject(MeshWorld *this, GameObject *obj) {
    if (!AthenaList_ContainsValue(&this->object_list, obj)) {
        AthenaList_Append(&this->object_list, obj);  // add to list
        obj->parent_meshworld = this;                  // set parent (offset +0x30)
        App_vtable[0x1D](app);                         // update mouse hover state
        obj->vtable[0xE](this);                        // OnAdd callback
    }
}
```

### Destroying a Submenu (RemapKeyboard BACK)

When the user selects "BACK" in the RemapKeyboard menu:

**RemapKeyboard_OnItemSelected (0x442830)** — `__stricmp(param_2, "BACK") == 0`:

```c
// 1. Mark self for destruction
this->vtable[0x10]();  // calls 0x419740 → sets this+0x2C = 1

// 2. Un-hide parent menu
parent_menu->e09 = 0;  // parent pointer at this+0xCDC (accessed as this[0x337])
```

### GameUpdate Per-Frame Loop (0x469CF0)

The main update function runs two passes over the MeshWorld object list each frame:

**Pass 1 — Update:**
```c
for each obj in meshworld->object_list:
    if (obj->destroy_flag == 0 && obj->pause_flag == 0):  // +0x2C and +0x874
        obj->vtable[1]()  // call Update
```

**Pass 2 — Destroy flagged objects:**
```c
for each obj in meshworld->object_list:
    if (obj->destroy_flag != 0):  // +0x2C
        // Clean up references
        if (meshworld->current_obj == obj):
            obj->vtable[0xC]()  // deactivate
        // Remove from list
        AthenaList_Remove(&meshworld->object_list, obj)
        obj->parent = 0
        // Call scene cleanup callback
        scene_vtable[0x1D]()
        // Call OnRemove callback
        obj->vtable[0xF](meshworld)
        // Delete + free
        obj->vtable[0](1)  // DeletingDtor with free flag
```

## Key Offsets

### Gadget/SimpleMenu/UIList base

| Offset | Type | Name | Description |
|--------|------|------|-------------|
| +0x00 | `void**` | vtable | Virtual function table pointer |
| +0x2C | `byte` | destroy_flag | Set to 1 by vtable[16] (0x419740) → GameUpdate destroys object next frame |
| +0x30 | `void*` | parent_meshworld | Set by Scene_AddObject |
| +0x874 | `byte` | pause_flag | When set, GameUpdate skips calling Update on this object |

### SimpleMenu additions

| Offset | Type | Name | Description |
|--------|------|------|-------------|
| +0x868 | `char*` | menu_name | Internal name (e.g. "Options Menu", "Remap Menu") |
| +0x878 | `void*` | app | App pointer (passed to ctor) |
| +0x888 | `char*` | title_text | Display title (e.g. "OPTIONS", "Remap Keyboard") |
| +0x88C | `AthenaList` | items | UI list items (UIListItem pointers) |
| +0xCAC | `int` | item_count | Number of items |
| +0xCDC | `void*` | parent_menu | Parent menu pointer (for un-hiding on BACK) |

### OptionsMenu-specific

| Offset | Type | Name | Description |
|--------|------|------|-------------|
| +0xE08 | `byte` | layout_initialized | Set to 1 after first layout pass |
| +0xE09 | `byte` | submenu_active | 1 = submenu is active, skip rendering this menu |

## The +0xE09 Flag (Submenu Hide)

The `+0xE09` flag controls whether a menu renders itself. It is checked in the **OptionsMenu_Render** function (vtable[2] = `0x441800`):

```c
void OptionsMenu_Render(void *this, void *gfx) {
    if (this->e09 == 0) {           // only render if no submenu active
        UIList_Render(this, gfx);   // base render
        // ... draw sliders, color quality text, etc.
    }
    if (this->e08 == 0) {           // first-time layout flag
        // ... initial layout
    }
}
```

When OptionsMenu creates the RemapKeyboard submenu, it sets `this+0xE09 = 1`, causing the OptionsMenu to stop rendering while the submenu is visible. When RemapKeyboard's BACK is pressed, it clears `parent+0xE09 = 0`, restoring the parent's rendering.

**Important:** The parent menu is NOT destroyed — it stays in the MeshWorld object list and still receives Update calls. It just skips rendering. Input is handled by checking `MeshWorld+0x424` (the "active object" pointer), which is updated when objects are added/removed.

## What 0x469990 (Scene_AddObject) Does

`Scene_AddObject` is the universal object registration function. It is called for **everything** — menus, level objects, score displays, dialog boxes, etc. (50+ call sites).

```c
void __thiscall Scene_AddObject(MeshWorld *this, GameObject *obj) {
    if (!AthenaList_ContainsValue(&this->object_list, obj)) {
        AthenaList_Append(&this->object_list, obj);
        obj->parent_meshworld = this;           // +0x30
        App_vtable[0x1D](app);                   // update mouse hover (App+0x1B8/0x1BC)
        obj->vtable[0xE](this);                 // OnAdd callback
    }
}
```

- `this` = MeshWorld (stored at `App+0x184`)
- `obj` = any game object (menu, level object, etc.)
- `this+0x04` = AthenaList of all objects
- `obj+0x30` = set to MeshWorld pointer (parent)
- App vtable[0x1D] (`0x46C8E0`) updates mouse hover state
- obj vtable[0xE] (offset `+0x38`) = OnAdd callback (varies per object type)

## Is It Safe to Create New Menus This Way?

**Yes — this is exactly how the game does it.** The pattern is:

1. `operator_new(size)` — allocate memory
2. Call constructor (sets vtable, adds UI items, initializes state)
3. `Scene_AddObject(App+0x184, menu)` — register with MeshWorld
4. Set parent's `+0xE09 = 1` — hide parent
5. When done: call `vtable[0x10]()` (sets `+0x2C = 1`), clear parent's `+0xE09 = 0`

GameUpdate will automatically destroy the menu on the next frame after `+0x2C` is set, calling the DeletingDtor which frees all UIList items, AthenaStrings, and the memory itself.

### Creating a New Menu: Minimal Example

```c
// Allocate
MyMenu *menu = operator_new(sizeof_MyMenu);

// Construct (your custom ctor)
MyMenu_ctor(menu, app, parent_menu);

// Register
Scene_AddObject(app->meshworld, menu);

// Hide parent
parent_menu->e09 = 1;
```

Your custom constructor must:
1. Call `SimpleMenu_ctor(this, app)` first
2. Set `this->vtable = my_menu_vtable`
3. Set `this+0xCDC = parent` (so BACK can un-hide parent)
4. Add items via `UIList_AddItem(this, label, name_id, ...)`
5. Set `this+0x888 = "My Menu"` (title)
6. Set `this+0x868 = "MyMenu"` (internal name)

Your OnItemSelected handler (vtable[18]) must:
1. Check `__stricmp(param_2, "BACK") == 0`
2. If BACK: call `this->vtable[0x10]()` (destroy flag), set `parent->e09 = 0`
3. Handle other items as needed

### Hooking 0x4431E0 vs Creating New Menus

**Hooking the RemapKeyboardMenu constructor (0x4431E0)** is simpler if you just want to:
- Add more remappable keys
- Change labels or layout
- Modify existing menu behavior

**Creating new menus via the allocate→construct→Scene_AddObject pattern** is better if you want:
- Entirely new menu screens
- Menus accessible from different places
- Custom menu logic that doesn't fit the remap menu structure

Both approaches are safe. The game's own code uses the same pattern for every menu transition (Options→Remap, Options→MainMenu, Pause→QuitRace, etc.).

## UIList Item Callbacks

Each UIList item has a callback function pointer (8th parameter to `UIList_AddItem`). When the user activates an item (Enter key or click), `UIList_ActivateCurrentItem` (0x449750) is called, which dispatches to `vtable[18]` (OnItemSelected) with the item's name string.

```c
// UIList_AddItem signature:
void UIList_AddItem(void *this, char *display_text, char *name_id,
                   float matrix[16], int callback_funcptr, int flags);
```

The callback function pointer is stored at `UIListItem+0x20` (offset 8th param). When activated, the menu's `vtable[0x48]` (OnItemSelected) is called with the item's name string as parameter.

## Summary

The menu system is a straightforward allocate→register→destroy pattern:
- **Create**: `operator_new` → ctor → `Scene_AddObject` → set parent's `+0xE09`
- **Destroy**: call `vtable[0x10]` (sets `+0x2C`) → clear parent's `+0xE09` → GameUpdate handles cleanup
- **0x469990** (`Scene_AddObject`): universal object registration, used for all game objects
- **0x4431E0** (`RemapKeyboardMenu_ctor`): specific constructor for the key remap submenu
- Both approaches (hooking vs new menus) are safe and used by the game itself
