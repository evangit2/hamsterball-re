
void __thiscall Mesh_UnlinkVertex(void *this,int param_1)

{
  int iVar1;
  int iVar2;
  int iVar3;
  
  iVar3 = param_1 * 0xc;
  iVar1 = *(int *)((int)this + 0x18) + iVar3;
  iVar2 = *(int *)(iVar1 + 4);
  if (iVar2 == -1) {
    iVar2 = *(int *)(iVar1 + 8);
    *(int *)((int)this + (uint)*(byte *)(iVar1 + 1) * 4 + 8) = iVar2;
    if (iVar2 != -1) {
      *(undefined4 *)(*(int *)((int)this + 0x18) + 4 + iVar2 * 0xc) = 0xffffffff;
    }
  }
  else {
    iVar1 = *(int *)(iVar1 + 8);
    *(int *)(*(int *)((int)this + 0x18) + 8 + iVar2 * 0xc) = iVar1;
    if (iVar1 != -1) {
      *(int *)(*(int *)((int)this + 0x18) + 4 + iVar1 * 0xc) = iVar2;
    }
  }
  *(undefined4 *)(*(int *)((int)this + 0x18) + 8 + iVar3) = 0xffffffff;
  *(undefined4 *)(*(int *)((int)this + 0x18) + 4 + iVar3) = 0xffffffff;
  return;
}

