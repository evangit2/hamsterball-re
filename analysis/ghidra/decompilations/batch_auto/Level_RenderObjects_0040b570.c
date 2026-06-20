/*
 * Function: Level_RenderObjects
 * Address: 0x0040b570
 * Signature: Level_RenderObjects(...)
 *
 * Patterns: vtable dispatch, rendering, level. Calls: Level_RenderObjects, Graphics_BeginFrame, AthenaList_NextIndex. Offsets: 5, Lines: 37
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* Level_RenderObjects(this, gfx): Transparent pass. BeginFrame + level->vtable[4C] mesh render,
   then iterate visible_object_list calling vtable[0C] (RenderTransparent) per object. See
   decompilations/scene/decomp_level_render.c */

void __thiscall Level_RenderObjects(void *this,void *param_1)

{
  int iVar1;
  int iVar2;
  int iVar3;
  int *piVar4;
  
  Graphics_BeginFrame(param_1,0);
  (**(code **)(**(int **)((int)this + 0x8ac) + 0x4c))();
  Graphics_BeginFrame(param_1,0);
  iVar3 = AthenaList_NextIndex(*(int *)((int)this + 0x3a48));
  iVar1 = *(int *)((int)this + 0x3a48);
  *(undefined4 *)(iVar1 + 8 + iVar3 * 4) = 0;
  if (*(int *)(iVar1 + 4) < 1) {
    piVar4 = (int *)0x0;
  }
  else {
    piVar4 = (int *)**(undefined4 **)(iVar1 + 0x40c);
    *(undefined4 *)(iVar1 + 8 + iVar3 * 4) = 1;
  }
  while( true ) {
    if (piVar4 == (int *)0x0) {
      return;
    }
    (**(code **)(*piVar4 + 0xc))();
    iVar1 = *(int *)((int)this + 0x3a48);
    iVar2 = *(int *)(iVar1 + 8 + iVar3 * 4);
    if (*(int *)(iVar1 + 4) <= iVar2) break;
    piVar4 = *(int **)(*(int *)(iVar1 + 0x40c) + iVar2 * 4);
    *(int *)(iVar1 + 8 + iVar3 * 4) = iVar2 + 1;
  }
  return;
}
