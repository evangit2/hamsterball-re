
void __thiscall MeshData_VertexRemapFace(void *this,int param_1,int param_2,int param_3)

{
  short *psVar1;
  int iVar2;
  ushort *puVar3;
  int iVar4;
  
  psVar1 = (short *)(*(int *)((int)this + 0x40) + param_1 * 6);
  puVar3 = (ushort *)(*(int *)((int)this + 0x44) + param_1 * 6);
  if (*psVar1 != -1) {
    iVar2 = (int)psVar1 - (int)puVar3;
    iVar4 = 3;
    do {
      *(undefined2 *)(iVar2 + (int)puVar3) =
           *(undefined2 *)(param_2 + (uint)*(ushort *)(iVar2 + (int)puVar3) * 4);
      if ((*(byte *)((int)this + 0xc) & 1) != 0) {
        if (*puVar3 == 0xffff) {
          *puVar3 = 0xffff;
        }
        else {
          *puVar3 = *(ushort *)(param_3 + (uint)*puVar3 * 4);
        }
      }
      puVar3 = puVar3 + 1;
      iVar4 = iVar4 + -1;
    } while (iVar4 != 0);
  }
  return;
}

