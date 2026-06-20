
void * __thiscall MeshWorld_ctor(void *this,int param_1,int param_2,int *param_3)

{
  int *piVar1;
  char *pcVar2;
  int iVar3;
  void *pvVar4;
  int *piVar5;
  void *local_10;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  piVar5 = param_3;
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cd79b;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  *(undefined4 *)this = 0;
  *(undefined1 *)((int)this + 4) = 0;
  local_10 = this;
  if (param_3 != (int *)0x0) {
    piVar1 = param_3;
    do {
      iVar3 = *piVar1;
      piVar1 = (int *)((int)piVar1 + 1);
    } while ((char)iVar3 != '\0');
    pcVar2 = operator_new((uint)((int)piVar1 + (1 - ((int)param_3 + 1))));
    *(char **)this = pcVar2;
    do {
      iVar3 = *piVar5;
      piVar5 = (int *)((int)piVar5 + 1);
      *pcVar2 = (char)iVar3;
      pcVar2 = pcVar2 + 1;
    } while ((char)iVar3 != '\0');
    iVar3 = _check_file_access(*(LPCSTR *)this,0);
    if (iVar3 != -1) {
      iVar3 = FID_conflict___open(*(char **)this,0x8000);
      param_3 = (int *)0x0;
      __read(iVar3,&param_3,4);
      __read(iVar3,&local_10,4);
      __close(iVar3);
      if (param_3 == (int *)0xbeef) {
        *(undefined1 *)((int)this + 4) = 1;
      }
      if ((float)local_10 != *(float *)(*(int *)(param_1 + 0x5c) + 0x40)) {
        Window_Notify(0x5341d0,(byte *)"Bad Cache Version");
        *(undefined1 *)((int)this + 4) = 0;
      }
    }
  }
  iVar3 = param_2;
  pvVar4 = operator_new(param_2 << 5);
  *(void **)((int)this + 0xc) = pvVar4;
  pvVar4 = operator_new(iVar3 * 2);
  *(void **)((int)this + 0x10) = pvVar4;
  param_3 = operator_new(iVar3 * 0x1c + 4);
  local_4 = 0;
  if (param_3 == (int *)0x0) {
    piVar5 = (int *)0x0;
  }
  else {
    piVar5 = param_3 + 1;
    *param_3 = iVar3;
    _eh_vector_constructor_iterator_
              (piVar5,0x1c,iVar3,D3DXSkinMesh_InitTimerDefaults,StreamReaderVtbl_Init);
  }
  *(int **)((int)this + 0x14) = piVar5;
  local_4 = 0xffffffff;
  pvVar4 = operator_new((iVar3 / 3) * 4);
  piVar5 = (int *)((int)this + 0x1c);
  *(void **)((int)this + 0x18) = pvVar4;
  *piVar5 = 0;
  piVar1 = (int *)((int)this + 0x20);
  *piVar1 = 0;
  *(undefined4 *)((int)this + 0x28) = 0;
  *(undefined4 *)((int)this + 0x24) = 0;
  *(int *)((int)this + 0x2c) = param_1;
  *(int *)((int)this + 8) = iVar3;
  if (*(char *)(param_1 + 0x7d1) == '\0') {
    *(undefined1 *)((int)this + 4) = 0;
  }
  if ((*(char **)this != (char *)0x0) && (*(char *)((int)this + 4) != '\0')) {
    iVar3 = FID_conflict___open(*(char **)this,0x8000);
    __read(iVar3,&param_1,4);
    __read(iVar3,&param_2,4);
    __read(iVar3,piVar5,4);
    __read(iVar3,*(void **)((int)this + 0xc),*piVar5 << 5);
    __read(iVar3,piVar1,4);
    __read(iVar3,*(void **)((int)this + 0x10),*piVar1 << 1);
    __read(iVar3,*(void **)((int)this + 0x18),(*piVar1 / 3) * 4);
    __close(iVar3);
  }
  ExceptionList = local_c;
  return this;
}

