
void __cdecl Deflate_InitStream(int param_1)

{
  undefined4 *puVar1;
  int iVar2;
  int iVar3;
  undefined4 *puVar4;
  
  puVar1 = (undefined4 *)(*(code *)**(undefined4 **)(param_1 + 4))(param_1,1,0x50);
  *(undefined4 **)(param_1 + 0x194) = puVar1;
  *puVar1 = Deflate_BuildHuffmanTables;
  puVar1[0xe] = 0;
  puVar1[0xf] = 0;
  puVar1[0x10] = 0;
  puVar1[0x11] = 0;
  puVar1 = (undefined4 *)
           (*(code *)**(undefined4 **)(param_1 + 4))(param_1,1,*(int *)(param_1 + 0x20) << 8);
  iVar3 = 0;
  *(undefined4 **)(param_1 + 0x8c) = puVar1;
  if (0 < *(int *)(param_1 + 0x20)) {
    do {
      iVar3 = iVar3 + 1;
      puVar4 = puVar1;
      for (iVar2 = 0x40; iVar2 != 0; iVar2 = iVar2 + -1) {
        *puVar4 = 0xffffffff;
        puVar4 = puVar4 + 1;
      }
      puVar1 = puVar1 + 0x40;
    } while (iVar3 < *(int *)(param_1 + 0x20));
  }
  return;
}

