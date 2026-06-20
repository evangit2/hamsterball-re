
void __thiscall Mesh_AddFace(void *this,float *param_1)

{
  int iVar1;
  
  iVar1 = Mesh_FindElement(this,param_1);
  *(int *)(*(int *)((int)this + 0x28) + *(int *)((int)this + 0x24) * 0x24) = iVar1;
  iVar1 = Mesh_FindElement(this,param_1 + 8);
  *(int *)(*(int *)((int)this + 0x28) + 4 + *(int *)((int)this + 0x24) * 0x24) = iVar1;
  iVar1 = Mesh_FindElement(this,param_1 + 0x10);
  *(int *)(*(int *)((int)this + 0x28) + 8 + *(int *)((int)this + 0x24) * 0x24) = iVar1;
  *(undefined4 *)(*(int *)((int)this + 0x28) + 0x1c + *(int *)((int)this + 0x24) * 0x24) = 0;
  *(undefined4 *)(*(int *)((int)this + 0x28) + 0xc + *(int *)((int)this + 0x24) * 0x24) = 0;
  Mesh_ConnectFaceEdges(this,*(int *)((int)this + 0x24));
  *(int *)((int)this + 0x24) = *(int *)((int)this + 0x24) + 1;
  return;
}

