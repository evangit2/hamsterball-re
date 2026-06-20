
void __thiscall Mesh_DecrementVertex(void *this,int param_1)

{
  char *pcVar1;
  int iVar2;
  
  iVar2 = param_1 - *(int *)((int)this + 0x1c);
  Mesh_UnlinkVertex(this,iVar2);
  pcVar1 = (char *)(*(int *)((int)this + 0x18) + 1 + iVar2 * 0xc);
  *pcVar1 = *pcVar1 + -1;
  Mesh_LinkVertexToHead(this,iVar2);
  return;
}

