// Function: App_OnMouseDown
// Address: 0x0046C290
// Calling Convention: __thiscall (this = App object)
// Parameters: param_1, param_2 (window message params), int param_3 (button index: 0=left, 1=right, 2=middle)
// Xrefs: Referenced from data 0x004D26BC and 0x004D97AC (window proc table / event handler table)
//
// PURPOSE: Window message handler for mouse button down events. Manages mouse capture,
// UI hit-testing, and event delegation. This is the entry point for ALL mouse click
// processing in the game.
//
// APP OBJECT FIELDS:
//   this+0x08   → HWND (window handle, used for SetCapture)
//   this+0x190   → mouse click counter (incremented every click)
//   this+0x184   → UIWidget root (for hit-testing)
//   this+0x188   → currently clicked widget (set by hit-test)
//   this+0x1B0   → mouse interceptor callback (if set, intercepts ALL clicks)
//   this+0x1B8   → mouse X position (from __ftol2 of float param)
//   this+0x1BC   → mouse Y position (from __ftol2 of float param)
//   this+0x1C8   → left mouse button down flag
//   this+0x1C9   → right mouse button down flag
//   this+0x1CA   → middle mouse button down flag
//   this+0x20C   → current operation string (debug, "Mouse Interceptor" or widget name)
//   this+0x210   → current event name string (debug, "MouseDown")
//
// UIWIDGET FIELDS (returned by hit-test):
//   widget+0x14   → vtable method for click dispatch (called with x, y, button)
//   widget+0x868  → widget name string (for debug logging)
//   widget+0x86C  → left button down flag
//   widget+0x86D  → right button down flag
//   widget+0x86E  → middle button down flag
//
// ALGORITHM:
// 1. Increment click counter, call SetCapture(hwnd) to capture mouse
// 2. Set button-down flag based on param_3 (0=left, 1=right, 2=middle)
// 3. Store mouse X/Y (converted from float via __ftol2)
// 4. If mouse interceptor exists (this+0x1B0): delegate to it and return
//    (used for modal dialogs, cutscenes, etc.)
// 5. Otherwise: call UIWidget_HitTest(root_widget, x, y) to find clicked widget
// 6. If a widget was hit:
//    a. Set debug strings ("MouseDown", widget name)
//    b. Set widget's button-down flag
//    c. Call widget->vtable[+0x14](x, y, button) to dispatch click event

void __thiscall App_OnMouseDown(void *this, undefined4 param_1, undefined4 param_2, int param_3)
{
  // ... decompiled body ...
}
