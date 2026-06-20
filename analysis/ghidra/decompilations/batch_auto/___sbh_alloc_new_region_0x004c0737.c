
/* Library Function - Single Match
    ___sbh_alloc_new_region
   
   Library: Visual Studio 2003 Release */

undefined4 * ___sbh_alloc_new_region(void)

{
  undefined4 *puVar1;
  LPVOID pvVar2;
  
  if (DAT_005369a8 == DAT_005369b8) {
    pvVar2 = HeapReAlloc(DAT_005369c0,0,DAT_005369ac,(DAT_005369b8 * 5 + 0x50) * 4);
    if (pvVar2 == (LPVOID)0x0) {
      return (undefined4 *)0x0;
    }
    DAT_005369b8 = DAT_005369b8 + 0x10;
    DAT_005369ac = pvVar2;
  }
  puVar1 = (undefined4 *)((int)DAT_005369ac + DAT_005369a8 * 0x14);
  pvVar2 = HeapAlloc(DAT_005369c0,8,0x41c4);
  puVar1[4] = pvVar2;
  if (pvVar2 != (LPVOID)0x0) {
    pvVar2 = VirtualAlloc((LPVOID)0x0,0x100000,0x2000,4);
    puVar1[3] = pvVar2;
    if (pvVar2 != (LPVOID)0x0) {
      puVar1[2] = 0xffffffff;
      *puVar1 = 0;
      puVar1[1] = 0;
      DAT_005369a8 = DAT_005369a8 + 1;
      *(undefined4 *)puVar1[4] = 0xffffffff;
      return puVar1;
    }
    HeapFree(DAT_005369c0,0,(LPVOID)puVar1[4]);
  }
  return (undefined4 *)0x0;
}

