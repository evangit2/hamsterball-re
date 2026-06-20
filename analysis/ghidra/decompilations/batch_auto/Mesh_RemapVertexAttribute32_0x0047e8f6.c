
void __thiscall Mesh_RemapVertexAttribute32(void *this,int param_1,int param_2)

{
  int *piVar1;
  
  if ((*(byte *)((int)this + 0xc) & 2) != 0) {
    piVar1 = (int *)(*(int *)((int)this + 0x38) + param_1 * 4);
    *piVar1 = *(int *)(param_2 + *piVar1 * 4);
  }
  return;
}

