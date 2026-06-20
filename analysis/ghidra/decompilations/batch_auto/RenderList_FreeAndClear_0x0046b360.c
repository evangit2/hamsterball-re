
void __fastcall RenderList_FreeAndClear(int param_1)

{
  int iVar1;
  int iVar2;
  undefined4 *puVar3;
  
  if (*(char *)(param_1 + 0x420) == '\0') {
    iVar2 = AthenaList_NextIndex(param_1 + 4);
    *(undefined4 *)(param_1 + 0xc + iVar2 * 4) = 0;
    if (*(int *)(param_1 + 8) < 1) {
      puVar3 = (undefined4 *)0x0;
    }
    else {
      puVar3 = (undefined4 *)**(undefined4 **)(param_1 + 0x410);
      *(undefined4 *)(param_1 + 0xc + iVar2 * 4) = 1;
    }
    while (puVar3 != (undefined4 *)0x0) {
      (**(code **)*puVar3)(1);
      iVar1 = *(int *)(param_1 + 0xc + iVar2 * 4);
      if (*(int *)(param_1 + 8) <= iVar1) break;
      puVar3 = *(undefined4 **)(*(int *)(param_1 + 0x410) + iVar1 * 4);
      *(int *)(param_1 + 0xc + iVar2 * 4) = iVar1 + 1;
    }
  }
  AthenaList_Free(param_1 + 4);
  return;
}

