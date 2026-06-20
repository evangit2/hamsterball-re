
/* Library Function - Single Match
    ___onexitinit
   
   Library: Visual Studio 2003 Release */

undefined4 ___onexitinit(void)

{
  DAT_00536ae8 = _malloc(0x80);
  if (DAT_00536ae8 == (undefined4 *)0x0) {
    return 0x18;
  }
  *DAT_00536ae8 = 0;
  DAT_00536ae4 = DAT_00536ae8;
  return 0;
}

