/*
 * Function: RepeatCall
 * Address: 0x00409d60
 * Signature: void RepeatCall(undefined4 param_1,undefined4 param_2,int param_3,undefined *param_4)
 *
 * Patterns: none identified. Calls: RepeatCall. Offsets: 0, Lines: 10
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void RepeatCall(undefined4 param_1,undefined4 param_2,int param_3,undefined *param_4)

{
  if (-1 < param_3 + -1) {
    do {
      (*(code *)param_4)();
      param_3 = param_3 + -1;
    } while (param_3 != 0);
  }
  return;
}
