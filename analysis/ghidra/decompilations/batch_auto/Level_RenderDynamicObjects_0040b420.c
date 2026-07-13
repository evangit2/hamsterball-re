/*
 * Function: Level_RenderDynamicObjects
 * Address: 0x0040b420
 * Signature: Level_RenderDynamicObjects(...)
 *
 * Patterns: vtable dispatch, SEH frame, rendering, level. Calls: Level_RenderDynamicObjects, AthenaList_NextIndex, Timer_Init, Gfx_SetPositionAndRender, Gfx_ScaleX, Gfx_ScaleZ, Gfx_SetPosition, FlagWaver_Render. Offsets: 16, Lines: 57
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */
/* Level_RenderDynamicObjects(this): 3-phase. 1)Sky: skybox vtable[48] or skydome 2)Flag waves:
   iterate list, Gfx_SetPosition+ScaleXZ+FlagWaver_Render 3)dynamic_object->vtable[8]. See
   decompilations/scene/decomp_level_render.c */

void __fastcall Level_RenderDynamicObjects(int param_1)

{
  int iVar1;
  int iVar2;
  int iVar3;
  void *pvStack_18;
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 uStack_4;
  
  uStack_4 = 0xffffffff;
  puStack_8 = &LAB_004c95d8;
  pvStack_c = ExceptionList;
  if (*(char *)(param_1 + 0x3a44) == '\0') {
    ExceptionList = &pvStack_c;
    (**(code **)(**(int **)(param_1 + 0x8ac) + 0x48))();
  }
  else {
    ExceptionList = &pvStack_c;
    (**(code **)(**(int **)(param_1 + 0x8b0) + 0x48))(1,1);
  }
  iVar2 = AthenaList_NextIndex(param_1 + 0x2160);
  *(undefined4 *)(param_1 + 0x2168 + iVar2 * 4) = 0;
  if (*(int *)(param_1 + 0x2164) < 1) {
    iVar3 = 0;
  }
  else {
    iVar3 = **(int **)(param_1 + 0x256c);
    *(undefined4 *)(param_1 + 0x2168 + iVar2 * 4) = 1;
  }
  while (iVar3 != 0) {
    Timer_Init((undefined4 *)&stack0xffffffa8);
    pvStack_c = (void *)0x0;
    Gfx_SetPositionAndRender
              (*(undefined4 *)(iVar3 + 0x1c),*(undefined4 *)(iVar3 + 0x20),
               *(undefined4 *)(iVar3 + 0x24));
    Gfx_ScaleX(_DAT_004cf44c - *(float *)(iVar3 + 0x14));
    Gfx_ScaleZ(-*(float *)(iVar3 + 0x10));
    Gfx_SetPosition(*(undefined4 *)(iVar3 + 4),*(undefined4 *)(iVar3 + 8),
                    *(undefined4 *)(iVar3 + 0xc));
    FlagWaver_Render(*(void **)(param_1 + 0x3f18),(float)&stack0xffffffa8);
    pvStack_c = (void *)0xffffffff;
    Timer_Cleanup((undefined4 *)&stack0xffffffa8);
    iVar1 = *(int *)(param_1 + 0x2168 + iVar2 * 4);
    if (*(int *)(param_1 + 0x2164) <= iVar1) break;
    iVar3 = *(int *)(*(int *)(param_1 + 0x256c) + iVar1 * 4);
    *(int *)(param_1 + 0x2168 + iVar2 * 4) = iVar1 + 1;
  }
  (**(code **)(*(int *)(param_1 + 0x3afc) + 8))(uStack_4);
  ExceptionList = pvStack_18;
  return;
}
