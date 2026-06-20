
undefined4 __thiscall
MeshData_InitShortVertexAdj(void *this,ushort param_1,ushort param_2,char param_3)

{
  void *pvVar1;
  short *psVar2;
  uint uVar3;
  char cVar4;
  uint uVar5;
  int iVar6;
  ushort uVar7;
  undefined4 *puVar8;
  
  uVar7 = 0;
  if (*(int *)((int)this + 0x10) == 0) {
    pvVar1 = operator_new((uint)*(ushort *)((int)this + 0x18) * 6);
    *(void **)((int)this + 0x10) = pvVar1;
    if (pvVar1 == (void *)0x0) {
LAB_00482d71:
      _free(*(void **)((int)this + 0x10));
      return 0x8007000e;
    }
    if (param_3 != '\0') {
      pvVar1 = operator_new((uint)*(ushort *)((int)this + 0x18) << 2);
      *(void **)((int)this + 0x20) = pvVar1;
      if (pvVar1 == (void *)0x0) goto LAB_00482d71;
    }
  }
  *(undefined2 *)((int)this + 8) = 0xffff;
  *(undefined2 *)((int)this + 10) = 0xffff;
  *(undefined2 *)((int)this + 0xc) = 0xffff;
  *(undefined2 *)((int)this + 0xe) = 0xffff;
  *(ushort *)((int)this + 0x16) = param_2;
  uVar3 = (uint)param_1;
  uVar5 = (uint)param_2;
  *(ushort *)((int)this + 0x14) = param_1;
  if (uVar3 < uVar3 + uVar5) {
    do {
      psVar2 = (short *)(*(int *)((int)this + 4) + uVar3 * 6);
      cVar4 = '\0';
      iVar6 = 3;
      do {
        if (*psVar2 != -1) {
          cVar4 = cVar4 + '\x01';
        }
        psVar2 = psVar2 + 1;
        iVar6 = iVar6 + -1;
      } while (iVar6 != 0);
      *(undefined1 *)((uint)uVar7 * 6 + *(int *)((int)this + 0x10)) = 0;
      *(char *)((uint)uVar7 * 6 + 1 + *(int *)((int)this + 0x10)) = cVar4;
      Mesh_LinkVertexToHead(this,uVar7);
      uVar7 = uVar7 + 1;
      uVar3 = (uint)(ushort)(uVar7 + param_1);
    } while (uVar3 < *(ushort *)((int)this + 0x14) + uVar5);
  }
  puVar8 = *(undefined4 **)((int)this + 0x20);
  if (puVar8 != (undefined4 *)0x0) {
    for (; uVar5 != 0; uVar5 = uVar5 - 1) {
      *puVar8 = 0;
      puVar8 = puVar8 + 1;
    }
    *(undefined4 *)((int)this + 0x1c) = 0;
  }
  return 0;
}

