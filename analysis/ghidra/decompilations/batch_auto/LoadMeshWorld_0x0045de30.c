
undefined4 __thiscall LoadMeshWorld(void *this,char *param_1)

{
  code *pcVar1;
  int iVar2;
  undefined4 uVar3;
  void *pvVar4;
  char local_10c [248];
  void *pvStack_14;
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cceae;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  AthenaString_SprintfToBuffer(local_10c,(byte *)"%s.meshworld");
  iVar2 = _check_file_access(local_10c,0);
  if (iVar2 == 0) {
    (**(code **)(*(int *)this + 0x38))(local_10c);
    uVar3 = (**(code **)(*(int *)this + 0x3c))(0);
  }
  else {
    pvVar4 = operator_new(0x488);
    local_4 = 0;
    if (pvVar4 == (void *)0x0) {
      pvVar4 = (void *)0x0;
    }
    else {
      pvVar4 = MeshWorld_ctor(pvVar4,*(undefined4 *)((int)this + 4));
    }
    local_4 = 0xffffffff;
    *(void **)((int)this + 8) = pvVar4;
    *(undefined1 *)((int)this + 0xd) = 1;
    uVar3 = MeshWorld_Parse(pvVar4,param_1,'\0');
    if ((char)uVar3 != '\x01') {
      MessageBoxA((HWND)0x0,param_1,"COULD NOT LOAD",0);
      CRT_FlsAlloc(0);
      pcVar1 = (code *)swi(3);
      uVar3 = (*pcVar1)();
      return uVar3;
    }
    *(undefined1 *)((int)this + 0xc) = 1;
    uVar3 = (**(code **)(*(int *)this + 4))();
  }
  ExceptionList = pvStack_14;
  return CONCAT31((int3)((uint)uVar3 >> 8),1);
}

