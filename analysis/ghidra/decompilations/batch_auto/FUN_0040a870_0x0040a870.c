/*
 * Function: FUN_0040a870
 * Address: 0x0040a870
 *
 * Description:
 *
Simple wrapper that calls AthenaList_Init(param_1, 0) — initializes an
AthenaList with 0 initial capacity. Referenced from 7 vtables as a
virtual init function.

Cross-refs: 0 calls, 7 data refs (vtable entries).

 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */


void __fastcall FUN_0040a870(void *param_1)
{
  AthenaList_Init(param_1,0);
  return;
}

