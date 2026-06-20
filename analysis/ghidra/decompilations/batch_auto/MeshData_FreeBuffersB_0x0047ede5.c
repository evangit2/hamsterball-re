
void __fastcall MeshData_FreeBuffersB(int param_1)

{
  _free(*(void **)(param_1 + 0x10));
  _free(*(void **)(param_1 + 0x20));
  return;
}

