/*
 * Function: GameObject_sub_ctor
 * Address: 0x00405D90
 * Signature: void * __thiscall GameObject_sub_ctor(void *this, int param_1)
 *
 * Description:
 * Constructor for the GameObject_sub class (a lightweight sub-class of Ball).
 * Calls Ball_ctor2 to perform full initialization, then applies sub-class
 * specific settings:
 *   1. Sets vtable to PTR_GameObject_sub_dtor_004cf494 (sub-class vtable)
 *   2. Sets scale at +0xC60 = 1.0f (0x3F800000) — no scaling
 *   3. Clears CollisionMesh+0x14 flag (a sub-class specific flag)
 *   4. Sets +0x80C = 0x32 (50 decimal) — likely a type or category ID
 *
 * Cross-references:
 *   - 0x41EFAA: BoardLevel9_Odd_ctor — constructing Odd Race level objects
 *
 * Struct offsets:
 *   ball+0x80C: Type/category ID (set to 50)
 *   ball+0xC60: Scale (set to 1.0)
 *   CollisionMesh+0x14: Sub-class flag (cleared)
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
