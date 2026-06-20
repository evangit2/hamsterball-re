
void __thiscall MeshData_RemoveFace(void *this,int param_1)

{
  int iVar1;
  int iVar2;
  uint uVar3;
  int iVar4;
  
  iVar4 = param_1 - *(int *)((int)this + 0x1c);
  *(undefined1 *)(*(int *)((int)this + 0x18) + iVar4 * 0xc) = 1;
  Mesh_UnlinkVertex(this,iVar4);
  iVar1 = *(int *)((int)this + 4);
  uVar3 = 0;
  do {
    iVar2 = *(int *)(iVar1 + param_1 * 0xc + uVar3 * 4);
    if ((iVar2 != -1) &&
       (*(char *)(*(int *)((int)this + 0x18) + (iVar2 - *(int *)((int)this + 0x1c)) * 0xc) == '\0'))
    {
      Mesh_DecrementVertex(this,iVar2);
    }
    uVar3 = uVar3 + 1;
  } while (uVar3 < 3);
  if (*(int *)((int)this + 0x2c) != 0) {
    *(undefined4 *)(*(int *)((int)this + 0x2c) + iVar4 * 4) = 0xffffffff;
  }
  return;
}

