
/* Library Function - Single Match
    ___sbh_heap_init
   
   Library: Visual Studio 2003 Release */

undefined4 __cdecl ___sbh_heap_init(undefined4 param_1)

{
  DAT_005369ac = HeapAlloc(DAT_005369c0,0,0x140);
  if (DAT_005369ac == (LPVOID)0x0) {
    return 0;
  }
  DAT_005369a4 = 0;
  DAT_005369a8 = 0;
  DAT_005369b4 = DAT_005369ac;
  DAT_005369b0 = param_1;
  DAT_005369b8 = 0x10;
  return 1;
}

