
void __cdecl Huffman_ResetTable(undefined4 *param_1)

{
  int iVar1;
  
  if ((void *)param_1[8] != (void *)0x0) {
    _free((void *)param_1[8]);
  }
  if ((void *)param_1[2] != (void *)0x0) {
    _free((void *)param_1[2]);
  }
  for (iVar1 = 9; iVar1 != 0; iVar1 = iVar1 + -1) {
    *param_1 = 0;
    param_1 = param_1 + 1;
  }
  return;
}

