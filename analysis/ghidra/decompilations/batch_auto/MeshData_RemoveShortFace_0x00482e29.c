
void __thiscall MeshData_RemoveShortFace(void *this,ushort param_1)

{
  ushort uVar1;
  ushort uVar2;
  ushort *puVar3;
  
  uVar2 = param_1 - *(short *)((int)this + 0x14);
  *(undefined1 *)(*(int *)((int)this + 0x10) + (uint)uVar2 * 6) = 1;
  Mesh_UnlinkShortVertex(this,uVar2);
  puVar3 = (ushort *)(*(int *)((int)this + 4) + (uint)param_1 * 6);
  _param_1 = 3;
  do {
    uVar1 = *puVar3;
    if ((uVar1 != 0xffff) &&
       (*(char *)(*(int *)((int)this + 0x10) +
                 ((uint)uVar1 - (uint)*(ushort *)((int)this + 0x14)) * 6) == '\0')) {
      Mesh_DecrementShortVertex(this,uVar1);
    }
    puVar3 = puVar3 + 1;
    _param_1 = _param_1 + -1;
  } while (_param_1 != 0);
  if (*(int *)((int)this + 0x20) != 0) {
    *(undefined4 *)(*(int *)((int)this + 0x20) + (uint)uVar2 * 4) = 0xffffffff;
  }
  return;
}

