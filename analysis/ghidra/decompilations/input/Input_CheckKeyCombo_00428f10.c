// Function: Input_CheckKeyCombo
// Address: 0x00428F10
// Decompiled: 2026-06-21
// Source: GhidraMCP native tool (mcp_ghidra_mcp_batch_decompile)

undefined4 __thiscall Input_CheckKeyCombo(void *this,int param_1)
{
  int iVar1;
  uint in_EAX;
  undefined4 *puVar2;
  
  // param_1 == 2: special case — joystick/keyboard combo check
  if (param_1 == 2) {
    // Get KeyboardDevice via InputHandler chain: this+0x180 → +0x434
    iVar1 = *(int *)(*(int *)((int)this + 0x180) + 0x434);
    // Check key state at KeyboardDevice+0xC+iVar1 (256-byte DIK buffer)
    // >> 7 extracts high bit (0x80 = key down)
    in_EAX = (uint)(int)*(char *)(*(int *)(iVar1 + 0x51c) + 0xc + iVar1) >> 7;
    // Also check joystick button state at iVar1+0x520
    if ((*(char *)(*(int *)(iVar1 + 0x520) + 0xc + iVar1) < '\0' || (in_EAX & 1) != 0) &&
       (in_EAX = *(uint *)((int)this + 0x560), in_EAX == 0)) {
      // Set debounce timer to 0x32 (50 frames)
      *(undefined4 *)((int)this + 0x560) = 0x32;
      return 1;
    }
  }
  // For param_1 < 4: check key bindings at this+0x550 + param_1*4
  if (param_1 < 4) {
    puVar2 = (undefined4 *)((int)this + param_1 * 4 + 0x550);
    do {
      // Check debounce timer at puVar2[4] (this+0x560+param_1*4)
      in_EAX = puVar2[4];
      if (in_EAX == 0) {
        // No debounce active — check if key is down
        in_EAX = Input_IsKeyDown((void *)*puVar2,-1);
        if ((char)in_EAX != '\0') {
          // Key pressed — set debounce timer and return 1
          *(undefined4 *)((int)this + param_1 * 4 + 0x560) = 0x32;
          return CONCAT31((int3)(in_EAX >> 8),1);
        }
      }
      param_1 = param_1 + 1;
      puVar2 = puVar2 + 1;
    } while (param_1 < 4);
  }
  return in_EAX & 0xffffff00;
}
