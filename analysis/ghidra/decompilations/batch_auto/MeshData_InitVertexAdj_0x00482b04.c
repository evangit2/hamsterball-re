
undefined4 __thiscall MeshData_InitVertexAdj(void *this,uint param_1,int param_2,char param_3)

{
  void *pvVar1;
  char cVar2;
  uint uVar3;
  int iVar4;
  int iVar5;
  undefined4 *puVar6;
  
  iVar5 = 0;
  if (*(int *)((int)this + 0x18) == 0) {
    pvVar1 = operator_new(*(int *)((int)this + 0x24) * 0xc);
    *(void **)((int)this + 0x18) = pvVar1;
    if (pvVar1 == (void *)0x0) {
LAB_00482b47:
      _free(*(void **)((int)this + 0x18));
      return 0x8007000e;
    }
    if (param_3 != '\0') {
      pvVar1 = operator_new(*(int *)((int)this + 0x24) << 2);
      *(void **)((int)this + 0x2c) = pvVar1;
      if (pvVar1 == (void *)0x0) goto LAB_00482b47;
    }
  }
  *(undefined4 *)((int)this + 8) = 0xffffffff;
  *(undefined4 *)((int)this + 0xc) = 0xffffffff;
  *(undefined4 *)((int)this + 0x10) = 0xffffffff;
  *(undefined4 *)((int)this + 0x14) = 0xffffffff;
  *(uint *)((int)this + 0x1c) = param_1;
  *(int *)((int)this + 0x20) = param_2;
  if (param_1 < param_1 + param_2) {
    _param_3 = 0;
    iVar4 = param_1 * 0xc;
    do {
      cVar2 = '\0';
      uVar3 = 0;
      do {
        if (*(int *)(*(int *)((int)this + 4) + iVar4 + uVar3 * 4) != -1) {
          cVar2 = cVar2 + '\x01';
        }
        uVar3 = uVar3 + 1;
      } while (uVar3 < 3);
      *(undefined1 *)(iVar5 + *(int *)((int)this + 0x18)) = 0;
      *(char *)(iVar5 + 1 + *(int *)((int)this + 0x18)) = cVar2;
      Mesh_LinkVertexToHead(this,_param_3);
      param_1 = param_1 + 1;
      iVar4 = iVar4 + 0xc;
      iVar5 = iVar5 + 0xc;
      _param_3 = _param_3 + 1;
    } while (param_1 < (uint)(*(int *)((int)this + 0x1c) + param_2));
  }
  puVar6 = *(undefined4 **)((int)this + 0x2c);
  if (puVar6 != (undefined4 *)0x0) {
    for (; param_2 != 0; param_2 = param_2 + -1) {
      *puVar6 = 0;
      puVar6 = puVar6 + 1;
    }
    *(undefined4 *)((int)this + 0x28) = 0;
  }
  return 0;
}

