/*
 * Function: Board_Up_HandleRaceEnd
 * Address: 0x00420660
 * Signature: void __fastcall Board_Up_HandleRaceEnd(int param_1)
 *
 * Patterns: vtable dispatch, scene, board. Calls: Board_Up_HandleRaceEnd, Scene_HandleRaceEnd, AthenaList_NextIndex. Offsets: 4, Lines: 28
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __fastcall Board_Up_HandleRaceEnd(int param_1)

{
  int iVar1;
  int iVar2;
  undefined4 *puVar3;
  
  Scene_HandleRaceEnd(param_1);
  iVar2 = AthenaList_NextIndex(param_1 + 0x436c);
  *(undefined4 *)(param_1 + 0x4374 + iVar2 * 4) = 0;
  if (*(int *)(param_1 + 0x4370) < 1) {
    puVar3 = (undefined4 *)0x0;
  }
  else {
    puVar3 = (undefined4 *)**(undefined4 **)(param_1 + 0x4778);
    *(undefined4 *)(param_1 + 0x4374 + iVar2 * 4) = 1;
  }
  while( true ) {
    if (puVar3 == (undefined4 *)0x0) {
      return;
    }
    (**(code **)*puVar3)();
    iVar1 = *(int *)(param_1 + 0x4374 + iVar2 * 4);
    if (*(int *)(param_1 + 0x4370) <= iVar1) break;
    puVar3 = *(undefined4 **)(*(int *)(param_1 + 0x4778) + iVar1 * 4);
    *(int *)(param_1 + 0x4374 + iVar2 * 4) = iVar1 + 1;
  }
  return;
}
