
void __fastcall MeshData_FreeBuffersA(int param_1)

{
  _free(*(void **)(param_1 + 0x18));
  _free(*(void **)(param_1 + 0x2c));
  return;
}

