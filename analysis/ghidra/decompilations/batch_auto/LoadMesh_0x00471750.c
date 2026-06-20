
uint __thiscall LoadMesh(void *this,char *param_1,char param_2)

{
  void *pvVar1;
  int iVar2;
  undefined4 uVar3;
  uint uVar4;
  char local_10c [256];
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cd8ce;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  pvVar1 = operator_new(0x488);
  local_4 = 0;
  if (pvVar1 == (void *)0x0) {
    pvVar1 = (void *)0x0;
  }
  else {
    pvVar1 = MeshWorld_ctor(pvVar1,*(undefined4 *)((int)this + 4));
  }
  local_4 = 0xffffffff;
  *(void **)((int)this + 8) = pvVar1;
  *(undefined1 *)((int)this + 0xd) = 1;
  AthenaString_SprintfToBuffer(local_10c,(byte *)"%s.mesh");
  iVar2 = _check_file_access(local_10c,0);
  if (iVar2 == 0) {
    (**(code **)(**(int **)((int)this + 8) + 4))(local_10c);
    uVar3 = (**(code **)(*(int *)this + 8))();
    uVar4 = CONCAT31((int3)((uint)uVar3 >> 8),1);
  }
  else {
    uVar4 = MeshWorld_Parse(*(int **)((int)this + 8),param_1,param_2);
    if ((char)uVar4 == '\x01') {
      uVar3 = (**(code **)(*(int *)this + 4))();
      *(undefined1 *)((int)this + 0xc) = 1;
      uVar4 = CONCAT31((int3)((uint)uVar3 >> 8),1);
    }
    else {
      uVar4 = uVar4 & 0xffffff00;
    }
  }
  ExceptionList = local_c;
  return uVar4;
}

