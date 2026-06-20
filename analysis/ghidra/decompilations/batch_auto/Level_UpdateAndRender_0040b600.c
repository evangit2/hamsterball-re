/*
 * Function: Level_UpdateAndRender
 * Address: 0x0040b600
 * Signature: Level_UpdateAndRender(...)
 *
 * Patterns: vtable dispatch, rendering, ball, level. Calls: Level_UpdateAndRender, AthenaList_Free, AthenaList_NextIndex, AthenaList_Append, WaypointList_SetNextWaypoint, AthenaList_GetSize, Ball_RenderShadow. Offsets: 23, Lines: 154
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* Level_UpdateAndRender(this, ball): 6-phase render. 1)Build visible_list from ball lists 2)Opaque:
   alpha OFF, ball->vtable[1C] 3)Alpha ON 4)Waypoint arrow if race active 5)visible_list->vtable[8]
   6)Ball_RenderShadow if ripples exist. See decompilations/scene/decomp_level_render.c */

void __thiscall Level_UpdateAndRender(void *this,int param_1)

{
  int iVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int *piVar6;
  
  AthenaList_Free(*(int *)((int)this + 0x3a48));
  iVar3 = (int)this + 0x29d4;
  iVar2 = AthenaList_NextIndex(iVar3);
  *(undefined4 *)((int)this + iVar2 * 4 + 0x29dc) = 0;
  if (*(int *)((int)this + 0x29d8) < 1) {
    iVar4 = 0;
  }
  else {
    iVar4 = **(int **)((int)this + 0x2de0);
    *(undefined4 *)((int)this + iVar2 * 4 + 0x29dc) = 1;
  }
  while (iVar4 != 0) {
    AthenaList_Append(*(void **)((int)this + 0x3a48),iVar4);
    iVar5 = *(int *)((int)this + iVar2 * 4 + 0x29dc);
    if (*(int *)((int)this + 0x29d8) <= iVar5) break;
    iVar4 = *(int *)(*(int *)((int)this + 0x2de0) + iVar5 * 4);
    *(int *)((int)this + iVar2 * 4 + 0x29dc) = iVar5 + 1;
  }
  iVar2 = (int)this + 0x3204;
  iVar4 = AthenaList_NextIndex(iVar2);
  *(undefined4 *)((int)this + iVar4 * 4 + 0x320c) = 0;
  if (*(int *)((int)this + 0x3208) < 1) {
    iVar5 = 0;
  }
  else {
    iVar5 = **(int **)((int)this + 0x3610);
    *(undefined4 *)((int)this + iVar4 * 4 + 0x320c) = 1;
  }
  while (iVar5 != 0) {
    AthenaList_Append(*(void **)((int)this + 0x3a48),iVar5);
    iVar1 = *(int *)((int)this + iVar4 * 4 + 0x320c);
    if (*(int *)((int)this + 0x3208) <= iVar1) break;
    iVar5 = *(int *)(*(int *)((int)this + 0x3610) + iVar1 * 4);
    *(int *)((int)this + iVar4 * 4 + 0x320c) = iVar1 + 1;
  }
  if (*(char *)(param_1 + 0x70c) != '\0') {
    (**(code **)(**(int **)(param_1 + 0x154) + 200))(*(int **)(param_1 + 0x154),0xe,0);
    *(undefined1 *)(param_1 + 0x70c) = 0;
    *(int *)(param_1 + 0x7c8) = *(int *)(param_1 + 0x7c8) + 1;
  }
  iVar4 = AthenaList_NextIndex(iVar3);
  *(undefined4 *)((int)this + iVar4 * 4 + 0x29dc) = 0;
  if (*(int *)((int)this + 0x29d8) < 1) {
    piVar6 = (int *)0x0;
  }
  else {
    piVar6 = (int *)**(undefined4 **)((int)this + 0x2de0);
    *(undefined4 *)((int)this + iVar4 * 4 + 0x29dc) = 1;
  }
  while (piVar6 != (int *)0x0) {
    (**(code **)(*piVar6 + 0x1c))();
    iVar5 = *(int *)((int)this + iVar4 * 4 + 0x29dc);
    if (*(int *)((int)this + 0x29d8) <= iVar5) break;
    piVar6 = *(int **)(*(int *)((int)this + 0x2de0) + iVar5 * 4);
    *(int *)((int)this + iVar4 * 4 + 0x29dc) = iVar5 + 1;
  }
  iVar4 = AthenaList_NextIndex(iVar2);
  *(undefined4 *)((int)this + iVar4 * 4 + 0x320c) = 0;
  if (*(int *)((int)this + 0x3208) < 1) {
    piVar6 = (int *)0x0;
  }
  else {
    piVar6 = (int *)**(undefined4 **)((int)this + 0x3610);
    *(undefined4 *)((int)this + iVar4 * 4 + 0x320c) = 1;
  }
  while (piVar6 != (int *)0x0) {
    (**(code **)(*piVar6 + 0x1c))();
    iVar5 = *(int *)((int)this + iVar4 * 4 + 0x320c);
    if (*(int *)((int)this + 0x3208) <= iVar5) break;
    piVar6 = *(int **)(*(int *)((int)this + 0x3610) + iVar5 * 4);
    *(int *)((int)this + iVar4 * 4 + 0x320c) = iVar5 + 1;
  }
  if (*(char *)(param_1 + 0x70c) != '\x01') {
    (**(code **)(**(int **)(param_1 + 0x154) + 200))(*(int **)(param_1 + 0x154),0xe,1);
    *(undefined1 *)(param_1 + 0x70c) = 1;
    *(int *)(param_1 + 0x7c8) = *(int *)(param_1 + 0x7c8) + 1;
  }
  iVar4 = *(int *)((int)this + 0x878);
  if (((*(char *)(*(int *)(iVar4 + 0x220) + 0x11) != '\0') && (*(char *)(iVar4 + 0x234) == '\0')) &&
     (*(void **)(iVar4 + 0x910) != (void *)0x0)) {
    WaypointList_SetNextWaypoint(*(void **)(iVar4 + 0x910),*(int *)((int)this + 0x361c));
    *(undefined4 *)(*(int *)((int)this + 0x361c) + 0x2fc) = 0x3ee66666;
    (**(code **)(**(int **)((int)this + 0x361c) + 8))();
  }
  iVar5 = AthenaList_NextIndex(*(int *)((int)this + 0x3a48));
  iVar4 = *(int *)((int)this + 0x3a48);
  *(undefined4 *)(iVar4 + 8 + iVar5 * 4) = 0;
  if (*(int *)(iVar4 + 4) < 1) {
    piVar6 = (int *)0x0;
  }
  else {
    piVar6 = (int *)**(undefined4 **)(iVar4 + 0x40c);
    *(undefined4 *)(iVar4 + 8 + iVar5 * 4) = 1;
  }
  while (piVar6 != (int *)0x0) {
    (**(code **)(*piVar6 + 8))();
    iVar4 = *(int *)((int)this + 0x3a48);
    iVar1 = *(int *)(iVar4 + 8 + iVar5 * 4);
    if (*(int *)(iVar4 + 4) <= iVar1) break;
    piVar6 = *(int **)(*(int *)(iVar4 + 0x40c) + iVar1 * 4);
    *(int *)(iVar4 + 8 + iVar5 * 4) = iVar1 + 1;
  }
  iVar4 = AthenaList_GetSize(*(int *)((int)this + 0x8ac) + 0xcb8);
  if (iVar4 != 0) {
    iVar3 = AthenaList_NextIndex(iVar3);
    *(undefined4 *)((int)this + iVar3 * 4 + 0x29dc) = 0;
    if (*(int *)((int)this + 0x29d8) < 1) {
      iVar4 = 0;
    }
    else {
      iVar4 = **(int **)((int)this + 0x2de0);
      *(undefined4 *)((int)this + iVar3 * 4 + 0x29dc) = 1;
    }
    while (iVar4 != 0) {
      Ball_RenderShadow(iVar4);
      iVar5 = *(int *)((int)this + iVar3 * 4 + 0x29dc);
      if (*(int *)((int)this + 0x29d8) <= iVar5) break;
      iVar4 = *(int *)(*(int *)((int)this + 0x2de0) + iVar5 * 4);
      *(int *)((int)this + iVar3 * 4 + 0x29dc) = iVar5 + 1;
    }
    iVar3 = AthenaList_NextIndex(iVar2);
    *(undefined4 *)((int)this + iVar3 * 4 + 0x320c) = 0;
    if (*(int *)((int)this + 0x3208) < 1) {
      iVar2 = 0;
    }
    else {
      iVar2 = **(int **)((int)this + 0x3610);
      *(undefined4 *)((int)this + iVar3 * 4 + 0x320c) = 1;
    }
    while (iVar2 != 0) {
      Ball_RenderShadow(iVar2);
      iVar4 = *(int *)((int)this + iVar3 * 4 + 0x320c);
      if (*(int *)((int)this + 0x3208) <= iVar4) {
        return;
      }
      iVar2 = *(int *)(*(int *)((int)this + 0x3610) + iVar4 * 4);
      *(int *)((int)this + iVar3 * 4 + 0x320c) = iVar4 + 1;
    }
  }
  return;
}
