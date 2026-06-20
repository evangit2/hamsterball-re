
void __thiscall MeshData_UpdateAttrIndex(void *this,int param_1,int param_2)

{
  ushort *puVar1;
  
  if ((*(byte *)((int)this + 0xc) & 2) != 0) {
    puVar1 = (ushort *)(*(int *)((int)this + 0x38) + param_1 * 2);
    *puVar1 = *(ushort *)(param_2 + (uint)*puVar1 * 4);
  }
  return;
}

