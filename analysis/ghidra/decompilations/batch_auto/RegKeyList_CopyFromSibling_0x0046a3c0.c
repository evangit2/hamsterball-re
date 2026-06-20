
undefined4 * __fastcall RegKeyList_CopyFromSibling(int param_1)

{
  int iVar1;
  undefined4 *this;
  int iVar2;
  undefined4 *puVar3;
  
  this = MusicChannel_LoadAndAppend(*(void **)(param_1 + 4),(char *)(param_1 + 0x424));
  iVar2 = AthenaList_NextIndex(param_1 + 0xc);
  *(undefined4 *)(param_1 + 0x14 + iVar2 * 4) = 0;
  if (*(int *)(param_1 + 0x10) < 1) {
    puVar3 = (undefined4 *)0x0;
  }
  else {
    puVar3 = (undefined4 *)**(undefined4 **)(param_1 + 0x418);
    *(undefined4 *)(param_1 + 0x14 + iVar2 * 4) = 1;
  }
  while( true ) {
    if (puVar3 == (undefined4 *)0x0) {
      return this;
    }
    RegKeyList_AppendStr(this,(char *)*puVar3,puVar3[1]);
    iVar1 = *(int *)(param_1 + 0x14 + iVar2 * 4);
    if (*(int *)(param_1 + 0x10) <= iVar1) break;
    puVar3 = *(undefined4 **)(*(int *)(param_1 + 0x418) + iVar1 * 4);
    *(int *)(param_1 + 0x14 + iVar2 * 4) = iVar1 + 1;
  }
  return this;
}

