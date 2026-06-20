/*
 * Function: Ball_Split_ctor
 * Address: 0x00408d10
 * Signature: Ball_Split_ctor(...)
 *
 * Patterns: ball. Calls: Ball_Split_ctor, Ball_ctor2, Ball_SetSpeed. Offsets: 4, Lines: 13
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void * __thiscall Ball_Split_ctor(void *this,int param_1)

{
  Ball_ctor2(this,param_1);
  *(undefined ***)this = &PTR_GameObject_sub_dtor_004cf560;
  *(undefined1 *)(*(int *)((int)this + 0x1a4) + 0x14) = 0;
  Ball_SetSpeed(*(void **)((int)this + 0x1a4),
                (_DAT_004d8e00 * _DAT_004cf55c * _DAT_004d8e04) /
                (*(float *)((int)*(void **)((int)this + 0x1a4) + 0xc78) / _DAT_004d8dfc));
  *(undefined4 *)((int)this + 0xc60) = 5;
  return this;
}
