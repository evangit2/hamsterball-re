/*
 * Function: RumbleBoard_ctor
 * Address: 0x004217b0
 * Signature: RumbleBoard_ctor(...)
 *
 * Patterns: SEH frame, scene, board. Calls: Board, RumbleBoard_ctor, Board_ctor, AthenaList_Init, CONCAT31, RumbleBoard_InitTimer. Offsets: 16, Lines: 37
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* RumbleBoard_ctor - inherits from Board (Gadget+Scene). Size ~0x47D4. Vtable 0x4D1358. Adds:
   AthenaList@0x4394, RumbleTimer@0x47C8, time_limit@0x47AC=6000, name@0x868="RumbleBoard",
   field_47B0-47C5 */

void * __thiscall RumbleBoard_ctor(void *this,int param_1)

{
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004ca606;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  Board_ctor(this,param_1);
  local_4 = 0;
  *(undefined ***)this = &PTR_RumbleBoard_dtor_004d1358;
  AthenaList_Init((void *)((int)this + 0x4394),0);
  local_4 = CONCAT31(local_4._1_3_,1);
  RumbleBoard_InitTimer((undefined4 *)((int)this + 0x47c8));
  *(undefined1 **)((int)this + 0x4344) = &DAT_004d1354;
  *(undefined1 **)((int)this + 0x29b4) = &DAT_004d1354;
  *(undefined1 *)((int)this + 0x4348) = 0;
  *(undefined1 *)((int)this + 0x47b0) = 0;
  *(undefined4 *)((int)this + 0x47b4) = 0;
  *(undefined4 *)((int)this + 0x47b8) = 0;
  *(undefined4 *)((int)this + 0x47bc) = 0;
  *(undefined4 *)((int)this + 0x47c0) = 0;
  *(undefined1 *)((int)this + 0x47c4) = 0;
  *(undefined1 *)((int)this + 0x47c5) = 0;
  *(undefined4 *)((int)this + 0x438c) = 0;
  *(char **)((int)this + 0x868) = "RumbleBoard";
  *(undefined4 *)((int)this + 0x47ac) = 6000;
  *(undefined4 *)((int)this + 0x47d0) = 0x19;
  ExceptionList = local_c;
  return this;
}
