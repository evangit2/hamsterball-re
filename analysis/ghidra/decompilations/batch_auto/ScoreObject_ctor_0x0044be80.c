
void __thiscall ScoreObject_ctor(void *this,int param_1,undefined4 param_2,undefined4 param_3)

{
  *(int *)((int)this + 4) = param_1;
  *(undefined ***)this = &PTR_LAB_004d6c00;
  *(undefined4 *)((int)this + 8) = param_2;
  *(undefined4 *)((int)this + 0xc) = *(undefined4 *)(param_1 + 0x878);
  *(int *)(param_1 + 0x8b4) = *(int *)(param_1 + 0x8b4) + 1;
  *(undefined4 *)((int)this + 0x10) = 0;
  *(undefined1 *)((int)this + 0x18) = 0;
  *(undefined1 *)((int)this + 0x28) = 0;
  *(undefined1 *)((int)this + 0x29) = 0;
  *(undefined1 *)((int)this + 0x2a) = 0;
  *(undefined1 *)((int)this + 0x19) = 1;
  *(undefined ***)this = &PTR_RaceGoalReached_Render_004d6c70;
  *(undefined4 *)((int)this + 0x14) = 200000;
  *(undefined4 *)((int)this + 0x1c) = 200;
  *(undefined4 *)((int)this + 0x20) = 0x42480000;
  *(undefined4 *)((int)this + 0x24) = 0x43480000;
  *(undefined4 *)((int)this + 0x2c) = param_3;
  return;
}

