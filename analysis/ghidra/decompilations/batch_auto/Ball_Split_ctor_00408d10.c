/*
 * Function: Ball_Split_ctor
 * Address: 0x00408D10
 * Signature: void * __thiscall Ball_Split_ctor(void *this, int param_1)
 *
 * Description:
 * Constructor for split balls (created when Ball_SplitIntoThree divides a ball
 * into three smaller copies). Calls Ball_ctor2 for full initialization, then
 * applies split-specific settings:
 *   1. Sets vtable to PTR_GameObject_sub_dtor_004cf560 (split ball vtable)
 *   2. Clears CollisionMesh+0x14 flag (marks as split ball)
 *   3. Calls Ball_SetSpeed with a computed value:
 *      speed = (_DAT_004D8E00 × _DAT_004CF55C × _DAT_004D8E04) /
 *              (CollisionMesh+0xC78 / _DAT_004D8DFC)
 *      This computes a speed based on global constants and the parent ball's
 *      radius, ensuring split balls move at an appropriate speed for their size.
 *   4. Sets scale (+0xC60) = 5 (visual scale — split balls are scaled down)
 *
 * Cross-references (3 call sites, all from Ball_SplitIntoThree):
 *   - 0x409151: first split ball creation
 *   - 0x40951F: second split ball creation
 *   - 0x408E68: third split ball creation (within Ball_SplitIntoThree itself)
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

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
