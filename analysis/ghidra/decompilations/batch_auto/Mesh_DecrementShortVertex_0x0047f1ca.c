
void __thiscall Mesh_DecrementShortVertex(void *this,short param_1)

{
  char *pcVar1;
  ushort uVar2;
  
  uVar2 = param_1 - *(short *)((int)this + 0x14);
  Mesh_UnlinkShortVertex(this,uVar2);
  pcVar1 = (char *)(*(int *)((int)this + 0x10) + 1 + (uint)uVar2 * 6);
  *pcVar1 = *pcVar1 + -1;
  Mesh_LinkVertexToHead(this,uVar2);
  return;
}

