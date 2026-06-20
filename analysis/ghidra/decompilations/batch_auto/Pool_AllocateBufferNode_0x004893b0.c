
void Pool_AllocateBufferNode(void)

{
  int in_EAX;
  undefined4 *puVar1;
  undefined4 *puVar2;
  
  puVar1 = Pool_AllocateBuffer();
  puVar2 = *(undefined4 **)(in_EAX + 4);
  *(int *)(in_EAX + 8) = *(int *)(in_EAX + 8) + 1;
  if (puVar2 != (undefined4 *)0x0) {
    *(undefined4 *)(in_EAX + 4) = puVar2[3];
    puVar2[1] = 0;
    puVar2[2] = 0;
    puVar2[3] = 0;
    *puVar2 = puVar1;
    return;
  }
  puVar2 = _malloc(0x10);
  puVar2[1] = 0;
  puVar2[2] = 0;
  puVar2[3] = 0;
  *puVar2 = puVar1;
  return;
}

