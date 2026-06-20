
int __fastcall MeshEdge_GetFirstVertex(int param_1)

{
  uint uVar1;
  
  uVar1 = 0;
  do {
    if (*(short *)(param_1 + 8 + (uVar1 & 0xffff) * 2) != -1) {
      return (uint)*(ushort *)(param_1 + 8 + (uVar1 & 0xffff) * 2) +
             (uint)*(ushort *)(param_1 + 0x14);
    }
    uVar1 = uVar1 + 1;
  } while ((ushort)uVar1 < 4);
  return CONCAT22((short)(uVar1 >> 0x10),0xffff);
}

