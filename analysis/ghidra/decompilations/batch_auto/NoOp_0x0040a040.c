/*
 * Function: NoOp
 * Address: 0x0040a040
 *
 * Description:
 *
Empty function that does nothing and returns immediately. Used as a placeholder
in vtables (for unused virtual slots) and as a default callback where no action
is needed. One of the most referenced functions in the engine.

Cross-refs: 19 calls, 39 data refs — appears in nearly every vtable as a
placeholder for optional virtual methods.

 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */


void NoOp(void)
{
  return;
}

