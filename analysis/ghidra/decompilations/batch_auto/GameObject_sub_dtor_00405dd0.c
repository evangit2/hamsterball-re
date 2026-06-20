/*
 * Function: GameObject_sub_dtor
 * Address: 0x00405dd0
 * Signature: void * __thiscall GameObject_sub_dtor(void *this,byte param_1)
 *
 * Patterns: frees memory. Calls: GameObject_sub_dtor, GameObject_dtor, _free. Offsets: 0, Lines: 8
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void * __thiscall GameObject_sub_dtor(void *this,byte param_1)

{
  GameObject_dtor(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}
