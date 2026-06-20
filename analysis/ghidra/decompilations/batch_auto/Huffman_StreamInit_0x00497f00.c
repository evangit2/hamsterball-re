
void __cdecl Huffman_StreamInit(undefined4 *param_1,undefined4 *param_2)

{
  int iVar1;
  undefined4 *puVar2;
  
  puVar2 = param_1;
  for (iVar1 = 6; iVar1 != 0; iVar1 = iVar1 + -1) {
    *puVar2 = 0;
    puVar2 = puVar2 + 1;
  }
  param_1[3] = param_2;
  param_1[4] = param_2;
  param_1[5] = 0;
  param_1[1] = param_2[1] + *(int *)*param_2;
  param_1[2] = param_2[2];
  Huffman_TreeAdvance();
  return;
}

