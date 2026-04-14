
undefined4 * __thiscall Graphics_LoadTexture(void *this,char *param_1,char param_2)

{
  int iVar1;
  int iVar2;
  void *this_00;
  undefined4 *puVar3;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004ccc1b;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  if (param_2 == '\x01') {
    ExceptionList = &local_c;
    iVar1 = AthenaList_GetIndex((int)this + 0x2e4);
    *(undefined4 *)((int)this + iVar1 * 4 + 0x2ec) = 0;
    if (*(int *)((int)this + 0x2e8) < 1) {
      puVar3 = (undefined4 *)0x0;
    }
    else {
      puVar3 = (undefined4 *)**(int **)((int)this + 0x6f0);
      *(undefined4 *)((int)this + iVar1 * 4 + 0x2ec) = 1;
    }
    while (puVar3 != (undefined4 *)0x0) {
      iVar2 = __stricmp((char *)puVar3[2],param_1);
      if (iVar2 == 0) {
        puVar3[4] = puVar3[4] + 1;
        ExceptionList = local_c;
        return puVar3;
      }
      iVar2 = *(int *)((int)this + iVar1 * 4 + 0x2ec);
      if (*(int *)((int)this + 0x2e8) <= iVar2) break;
      puVar3 = *(undefined4 **)(*(int *)((int)this + 0x6f0) + iVar2 * 4);
      *(int *)((int)this + iVar1 * 4 + 0x2ec) = iVar2 + 1;
    }
  }
  this_00 = operator_new(0x74);
  puVar3 = (undefined4 *)0x0;
  local_4 = 0;
  if (this_00 != (void *)0x0) {
    puVar3 = Texture_Create(this_00,(int)this,param_1);
  }
  local_4 = 0xffffffff;
  FUN_00453780((void *)((int)this + 0x2e4),(int)puVar3);
  ExceptionList = local_c;
  return puVar3;
}

