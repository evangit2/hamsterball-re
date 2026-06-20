/*
 * Function: Scene_UpdateBallsAndState
 * Address: 0x0041b540
 * Signature: void __fastcall Scene_UpdateBallsAndState(void *param_1)
 *
 * Patterns: vtable dispatch, rendering, camera, ball, scene. Calls: Scene_UpdateBallsAndState, AthenaList_NextIndex, Scene_SetCamera, Ball_FindClosestRespawnPoint, thunk_Gfx_SetRenderState, WaypointList_AppendCurrent, AthenaList_GetSize. Offsets: 18, Lines: 76
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __fastcall Scene_UpdateBallsAndState(void *param_1)

{
  int iVar1;
  int iVar2;
  int *piVar3;
  
  iVar1 = AthenaList_NextIndex((int)param_1 + 0x29d4);
  *(undefined4 *)((int)param_1 + iVar1 * 4 + 0x29dc) = 0;
  if (*(int *)((int)param_1 + 0x29d8) < 1) {
    piVar3 = (int *)0x0;
  }
  else {
    piVar3 = (int *)**(undefined4 **)((int)param_1 + 0x2de0);
    *(undefined4 *)((int)param_1 + iVar1 * 4 + 0x29dc) = 1;
  }
  while (piVar3 != (int *)0x0) {
    Scene_SetCamera(param_1,piVar3,'\x01');
    (**(code **)(*piVar3 + 0x10))();
    if ((char)piVar3[0xba] != '\0') {
      if ((((char)piVar3[0xc9] == '\0') && (piVar3[6] != -1)) ||
         (*(char *)(*(int *)((int)param_1 + 0x878) + 0x237) != '\0')) {
        Ball_FindClosestRespawnPoint((int)piVar3);
      }
      else {
        thunk_Gfx_SetRenderState((void *)((int)param_1 + 0x29d4),(int)piVar3);
        (**(code **)*piVar3)(1);
      }
    }
    iVar2 = *(int *)((int)param_1 + iVar1 * 4 + 0x29dc);
    if (*(int *)((int)param_1 + 0x29d8) <= iVar2) break;
    piVar3 = *(int **)(*(int *)((int)param_1 + 0x2de0) + iVar2 * 4);
    *(int *)((int)param_1 + iVar1 * 4 + 0x29dc) = iVar2 + 1;
  }
  iVar1 = AthenaList_NextIndex((int)param_1 + 0x3204);
  *(undefined4 *)((int)param_1 + iVar1 * 4 + 0x320c) = 0;
  if (*(int *)((int)param_1 + 0x3208) < 1) {
    piVar3 = (int *)0x0;
  }
  else {
    piVar3 = (int *)**(undefined4 **)((int)param_1 + 0x3610);
    *(undefined4 *)((int)param_1 + iVar1 * 4 + 0x320c) = 1;
  }
  while (piVar3 != (int *)0x0) {
    Scene_SetCamera(param_1,piVar3,'\x01');
    (**(code **)(*piVar3 + 0x10))();
    if ((char)piVar3[0xba] != '\0') {
      if (((char)piVar3[0xc9] == '\0') && (piVar3[6] != -1)) {
        Ball_FindClosestRespawnPoint((int)piVar3);
      }
      else {
        thunk_Gfx_SetRenderState((void *)((int)param_1 + 0x3204),(int)piVar3);
        (**(code **)*piVar3)(1);
      }
    }
    iVar2 = *(int *)((int)param_1 + iVar1 * 4 + 0x320c);
    if (*(int *)((int)param_1 + 0x3208) <= iVar2) break;
    piVar3 = *(int **)(*(int *)((int)param_1 + 0x3610) + iVar2 * 4);
    *(int *)((int)param_1 + iVar1 * 4 + 0x320c) = iVar2 + 1;
  }
  iVar1 = *(int *)((int)param_1 + 0x878);
  if ((*(char *)(*(int *)(iVar1 + 0x220) + 0x11) != '\0') && (*(char *)(iVar1 + 0x234) == '\0')) {
    WaypointList_AppendCurrent(*(void **)(iVar1 + 0x90c),*(int *)(iVar1 + 0x5dc));
    iVar1 = *(int *)(*(int *)((int)param_1 + 0x878) + 0x910);
    if (iVar1 != 0) {
      piVar3 = (int *)(iVar1 + 0x41c);
      *piVar3 = *piVar3 + 1;
      iVar1 = *(int *)(*(int *)((int)param_1 + 0x878) + 0x910);
      iVar2 = AthenaList_GetSize(*(int *)(*(int *)((int)param_1 + 0x878) + 0x910) + 4);
      if (iVar2 <= *(int *)(iVar1 + 0x41c)) {
        iVar1 = AthenaList_GetSize(iVar1 + 4);
        *(int *)(*(int *)(*(int *)((int)param_1 + 0x878) + 0x910) + 0x41c) = iVar1 + -1;
      }
    }
  }
  return;
}
