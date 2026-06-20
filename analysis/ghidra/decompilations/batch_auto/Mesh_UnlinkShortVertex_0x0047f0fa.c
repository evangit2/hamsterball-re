
void __thiscall Mesh_UnlinkShortVertex(void *this,ushort param_1)

{
  int iVar1;
  ushort uVar2;
  ushort uVar3;
  int iVar4;
  
  iVar4 = (uint)param_1 * 6;
  iVar1 = *(int *)((int)this + 0x10) + iVar4;
  uVar2 = *(ushort *)(iVar1 + 2);
  if (uVar2 == 0xffff) {
    uVar2 = *(ushort *)(iVar1 + 4);
    *(ushort *)((int)this + (uint)*(byte *)(iVar1 + 1) * 2 + 8) = uVar2;
    if (uVar2 != 0xffff) {
      *(undefined2 *)(*(int *)((int)this + 0x10) + 2 + (uint)uVar2 * 6) = 0xffff;
    }
  }
  else {
    uVar3 = *(ushort *)(iVar1 + 4);
    *(ushort *)(*(int *)((int)this + 0x10) + 4 + (uint)uVar2 * 6) = uVar3;
    if (uVar3 != 0xffff) {
      *(ushort *)(*(int *)((int)this + 0x10) + 2 + (uint)uVar3 * 6) = uVar2;
    }
  }
  *(undefined2 *)(*(int *)((int)this + 0x10) + 4 + iVar4) = 0xffff;
  *(undefined2 *)(*(int *)((int)this + 0x10) + 2 + iVar4) = 0xffff;
  return;
}

