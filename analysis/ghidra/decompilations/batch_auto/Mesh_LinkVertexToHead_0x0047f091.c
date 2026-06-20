
void __thiscall Mesh_LinkVertexToHead(void *this,int param_1)

{
  int iVar1;
  int *piVar2;
  int iVar3;
  
  iVar1 = *(int *)((int)this + 0x18) + param_1 * 0xc;
  piVar2 = (int *)((int)this + (uint)*(byte *)(iVar1 + 1) * 4 + 8);
  iVar3 = *piVar2;
  *(int *)(iVar1 + 8) = iVar3;
  if (iVar3 != -1) {
    *(int *)(*(int *)((int)this + 0x18) + 4 + iVar3 * 0xc) = param_1;
  }
  *piVar2 = param_1;
  *(undefined4 *)(*(int *)((int)this + 0x18) + 4 + param_1 * 0xc) = 0xffffffff;
  return;
}

