
void * __thiscall Graphics_FindOrCreateTexture(void *this,char *param_1,char param_2)

{
  int iVar1;
  int iVar2;
  void *pvVar3;
  void *pvVar4;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004ccc1b;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  if (param_2 == '\x01') {
    ExceptionList = &local_c;
    iVar1 = AthenaList_NextIndex((int)this + 0x2e4);
    *(undefined4 *)((int)this + iVar1 * 4 + 0x2ec) = 0;
    if (*(int *)((int)this + 0x2e8) < 1) {
      pvVar4 = (void *)0x0;
    }
    else {
      pvVar4 = (void *)**(int **)((int)this + 0x6f0);
      *(undefined4 *)((int)this + iVar1 * 4 + 0x2ec) = 1;
    }
    while (pvVar4 != (void *)0x0) {
      iVar2 = __stricmp(*(char **)((int)pvVar4 + 8),param_1);
      if (iVar2 == 0) {
        *(int *)((int)pvVar4 + 0x10) = *(int *)((int)pvVar4 + 0x10) + 1;
        ExceptionList = local_c;
        return pvVar4;
      }
      iVar2 = *(int *)((int)this + iVar1 * 4 + 0x2ec);
      if (*(int *)((int)this + 0x2e8) <= iVar2) break;
      pvVar4 = *(void **)(*(int *)((int)this + 0x6f0) + iVar2 * 4);
      *(int *)((int)this + iVar1 * 4 + 0x2ec) = iVar2 + 1;
    }
  }
  pvVar4 = operator_new(0x74);
  pvVar3 = (void *)0x0;
  local_4 = 0;
  if (pvVar4 != (void *)0x0) {
    pvVar3 = Texture_LoadWithMips(pvVar4,this,param_1);
  }
  local_4 = 0xffffffff;
  AthenaList_Append((void *)((int)this + 0x2e4),(int)pvVar3);
  ExceptionList = local_c;
  return pvVar3;
}

