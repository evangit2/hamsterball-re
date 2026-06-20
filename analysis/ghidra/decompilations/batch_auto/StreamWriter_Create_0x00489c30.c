
undefined4 * StreamWriter_Create(void)

{
  undefined4 *puVar1;
  void *pvVar2;
  int iVar3;
  undefined4 *puVar4;
  
  puVar1 = _calloc(1,0x1c);
  puVar4 = puVar1;
  for (iVar3 = 7; iVar3 != 0; iVar3 = iVar3 + -1) {
    *puVar4 = 0;
    puVar4 = puVar4 + 1;
  }
  pvVar2 = _calloc(1,0x10);
  *puVar1 = pvVar2;
  return puVar1;
}

