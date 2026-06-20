/*
 * Function: GameObject_sub_ctor
 * Address: 0x00405d90
 * Signature: void * __thiscall GameObject_sub_ctor(void *this,int param_1)
 *
 * Patterns: ball. Calls: GameObject_sub_ctor, Ball_ctor2. Offsets: 4, Lines: 9
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void * __thiscall GameObject_sub_ctor(void *this,int param_1)

{
  Ball_ctor2(this,param_1);
  *(undefined ***)this = &PTR_GameObject_sub_dtor_004cf494;
  *(undefined4 *)((int)this + 0xc60) = 0x3f800000;
  *(undefined1 *)(*(int *)((int)this + 0x1a4) + 0x14) = 0;
  *(undefined4 *)((int)this + 0x80c) = 0x32;
  return this;
}
