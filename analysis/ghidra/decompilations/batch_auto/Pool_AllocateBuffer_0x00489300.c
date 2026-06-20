
undefined4 * Pool_AllocateBuffer(void)

{
  void *pvVar1;
  undefined4 *puVar2;
  size_t _Size;
  int *unaff_EBX;
  size_t unaff_EDI;
  
  puVar2 = (undefined4 *)*unaff_EBX;
  unaff_EBX[2] = unaff_EBX[2] + 1;
  if (puVar2 == (undefined4 *)0x0) {
    puVar2 = _malloc(0x10);
    _Size = 0x10;
    if (0xf < (int)unaff_EDI) {
      _Size = unaff_EDI;
    }
    pvVar1 = _malloc(_Size);
    *puVar2 = pvVar1;
    puVar2[1] = unaff_EDI;
  }
  else {
    *unaff_EBX = puVar2[3];
    if ((int)puVar2[1] < (int)unaff_EDI) {
      pvVar1 = _realloc((void *)*puVar2,unaff_EDI);
      *puVar2 = pvVar1;
      puVar2[1] = unaff_EDI;
      puVar2[2] = 1;
      puVar2[3] = unaff_EBX;
      return puVar2;
    }
  }
  puVar2[2] = 1;
  puVar2[3] = unaff_EBX;
  return puVar2;
}

