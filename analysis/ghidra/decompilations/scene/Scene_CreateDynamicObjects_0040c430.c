// Function: Scene_CreateDynamicObjects
// Address: 0x0040C430
// Decompiled: 2026-06-21
// Source: GhidraMCP native tool (mcp_ghidra_mcp_batch_decompile)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall Scene_CreateDynamicObjects(int *param_1)
{
  int iVar1;
  int iVar2;
  int iVar3;
  undefined4 *puVar4;
  int *local_58;
  int local_54;
  undefined4 auStack_50 [17];
  void *local_c;
  undefined1 *puStack_8;
  undefined4 uStack_4;
  
  uStack_4 = 0xffffffff;
  puStack_8 = &LAB_004c95d8;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  
  // Get MW parser object from Scene: param_1[0x22b] → +0x480 (MESHWORLD data)
  iVar3 = AthenaList_NextIndex(*(int *)(param_1[0x22b] + 0x480) + 0x894);
  iVar1 = *(int *)(param_1[0x22b] + 0x480);
  *(undefined4 *)(iVar1 + 0x89c + iVar3 * 4) = 0;
  if (*(int *)(iVar1 + 0x898) < 1) {
    puVar4 = (undefined4 *)0x0;
  }
  else {
    puVar4 = (undefined4 *)**(undefined4 **)(iVar1 + 0xca0);
    *(undefined4 *)(iVar1 + 0x89c + iVar3 * 4) = 1;
  }
  while( true ) {
    if (puVar4 == (undefined4 *)0x0) {
      ExceptionList = local_c;
      return;
    }
    local_58 = (int *)0x0;
    local_54 = 0;
    // Call Scene vtable[0x21] (slot 33) — factory dispatch per object type
    // puVar4 = MESHWORLD ref point entry (name, x, y, z, ...)
    (**(code **)(*param_1 + 0x84))(*puVar4, &local_58, &local_54, puVar4);
    
    if (local_58 != (int *)0x0) {
      // Object was created successfully
      Timer_Init(auStack_50);
      uStack_4 = 0;
      // Set object scale/position from MESHWORLD data
      Gfx_ScaleX(_DAT_004cf44c - (float)puVar4[5]);
      Gfx_SetPosition(puVar4[1], puVar4[2], puVar4[3]);
      
      // Add to Scene's dynamic object list (Scene+0x335*4 = Scene+0xCD4)
      AthenaList_Append(param_1 + 0x335, (int)local_58);
      // Add to MW parser's object list
      AthenaList_Append((void *)(*(int *)(param_1[0x22b] + 0x480) + 0x1c), (int)local_58);
      
      // Call object vtable[0x16] (slot 22) — Init/Setup
      (**(code **)(*local_58 + 0x58))();
      // Call object vtable[0x15] (slot 21) — SetTimer
      (**(code **)(*local_58 + 0x54))(auStack_50);
      
      // If a secondary object was created (e.g., trigger zone)
      if (local_54 != 0) {
        // Add to Scene+0x43B*4 = Scene+0x10EC
        AthenaList_Append(param_1 + 0x43b, local_54);
        // Add to Scene[0x22c]+0x18 (physics/collision list)
        AthenaList_Append((void *)(param_1[0x22c] + 0x18), local_54);
      }
      uStack_4 = 0xffffffff;
      Timer_Cleanup(auStack_50);
    }
    
    // Advance to next MESHWORLD ref point
    iVar1 = *(int *)(param_1[0x22b] + 0x480);
    iVar2 = *(int *)(iVar1 + 0x89c + iVar3 * 4);
    if (*(int *)(iVar1 + 0x898) <= iVar2) break;
    puVar4 = *(undefined4 **)(*(int *)(iVar1 + 0xca0) + iVar2 * 4);
    *(int *)(iVar1 + 0x89c + iVar3 * 4) = iVar2 + 1;
  }
  ExceptionList = local_c;
  return;
}
