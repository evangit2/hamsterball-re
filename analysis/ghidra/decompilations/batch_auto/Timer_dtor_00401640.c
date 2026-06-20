/*
 * Function: Timer_dtor
 * Address: 0x00401640
 * Signature: void * __thiscall Timer_dtor(void *this,byte param_1)
 *
 * Patterns: frees memory. Calls: Timer_dtor, Timer_Cleanup, _free. Offsets: 0, Lines: 8
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void * __thiscall Timer_dtor(void *this,byte param_1)

{
  Timer_Cleanup(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}
