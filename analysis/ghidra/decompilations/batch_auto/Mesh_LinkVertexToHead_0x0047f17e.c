
void __thiscall Mesh_LinkVertexToHead(void *this,ushort param_1)

{
  ushort *puVar1;
  ushort uVar2;
  int iVar3;
  
  iVar3 = *(int *)((int)this + 0x10) + (uint)param_1 * 6;
  puVar1 = (ushort *)((int)this + (uint)*(byte *)(iVar3 + 1) * 2 + 8);
  uVar2 = *puVar1;
  *(ushort *)(iVar3 + 4) = uVar2;
  if (uVar2 != 0xffff) {
    *(ushort *)(*(int *)((int)this + 0x10) + 2 + (uint)uVar2 * 6) = param_1;
  }
  *puVar1 = param_1;
  *(undefined2 *)(*(int *)((int)this + 0x10) + 2 + (uint)param_1 * 6) = 0xffff;
  return;
}

