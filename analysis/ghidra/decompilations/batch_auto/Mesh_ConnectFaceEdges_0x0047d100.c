
void __thiscall Mesh_ConnectFaceEdges(void *this,int param_1)

{
  int iVar1;
  
  iVar1 = param_1 * 0x24;
  Mesh_ConnectElements
            (this,param_1,*(int *)(*(int *)((int)this + 0x28) + iVar1),
             *(int *)(*(int *)((int)this + 0x28) + 4 + iVar1));
  Mesh_ConnectElements
            (this,param_1,*(int *)(*(int *)((int)this + 0x28) + 4 + iVar1),
             *(int *)(*(int *)((int)this + 0x28) + 8 + iVar1));
  Mesh_ConnectElements
            (this,param_1,*(int *)(*(int *)((int)this + 0x28) + 8 + iVar1),
             *(int *)(*(int *)((int)this + 0x28) + iVar1));
  return;
}

