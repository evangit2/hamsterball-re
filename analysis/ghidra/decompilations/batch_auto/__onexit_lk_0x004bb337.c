
/* Library Function - Single Match
    __onexit_lk
   
   Library: Visual Studio 2003 Release */

void __onexit_lk(void)

{
  size_t sVar1;
  void *pvVar2;
  size_t sVar3;
  undefined4 unaff_EDI;
  
  sVar1 = __msize(DAT_00536ae8);
  if (sVar1 < (uint)((int)DAT_00536ae4 + (4 - (int)DAT_00536ae8))) {
    sVar3 = 0x800;
    if (sVar1 < 0x800) {
      sVar3 = sVar1;
    }
    pvVar2 = _realloc(DAT_00536ae8,sVar3 + sVar1);
    if (pvVar2 == (void *)0x0) {
      pvVar2 = _realloc(DAT_00536ae8,sVar1 + 0x10);
      if (pvVar2 == (void *)0x0) {
        return;
      }
    }
    DAT_00536ae4 = (undefined4 *)((int)pvVar2 + ((int)DAT_00536ae4 - (int)DAT_00536ae8 >> 2) * 4);
    DAT_00536ae8 = pvVar2;
  }
  *DAT_00536ae4 = unaff_EDI;
  DAT_00536ae4 = DAT_00536ae4 + 1;
  return;
}

