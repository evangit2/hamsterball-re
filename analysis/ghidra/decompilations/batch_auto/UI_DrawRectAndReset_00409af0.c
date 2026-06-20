/*
 * Function: UI_DrawRectAndReset
 * Address: 0x00409af0
 * Signature: void __thiscall UI_DrawRectAndReset(void *this)
 *
 * Patterns: SEH frame, matrix math, rendering. Calls: UI_DrawRectAndReset, __ftol2, Graphics_DrawScreenRect, Matrix_Identity. Offsets: 0, Lines: 27
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __thiscall UI_DrawRectAndReset(void *this)

{
  int iVar1;
  int iVar2;
  int iVar3;
  ulonglong uVar4;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  puStack_8 = &LAB_004c94e0;
  local_c = ExceptionList;
  local_4 = 1;
  ExceptionList = &local_c;
  uVar4 = __ftol2();
  iVar1 = (int)uVar4;
  uVar4 = __ftol2();
  iVar2 = (int)uVar4;
  uVar4 = __ftol2();
  iVar3 = (int)uVar4;
  uVar4 = __ftol2();
  Graphics_DrawScreenRect(this,(int)uVar4,iVar3,iVar2,iVar1);
  local_4 = 0xffffffff;
  Matrix_Identity((undefined4 *)&stack0x00000018);
  ExceptionList = local_c;
  return;
}
