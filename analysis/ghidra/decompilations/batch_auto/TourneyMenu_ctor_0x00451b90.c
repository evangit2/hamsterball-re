
void * __thiscall TourneyMenu_ctor(void *this,int param_1)

{
  uint uVar1;
  ulonglong uVar2;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cca76;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  Gadget_ctor(this,param_1);
  local_4 = 0;
  *(undefined ***)this = &PTR_TourneyBoard_ObjectScalarDtor_004d8628;
  SceneObject_EmptyListCtor((undefined4 *)((int)this + 0x888));
  *(int *)((int)this + 0x878) = param_1;
  *(undefined **)((int)this + 0x868) = &DAT_004d8620;
  *(undefined4 *)((int)this + 0x87c) = 0x4b0;
  *(undefined1 *)(*(int *)(param_1 + 0x220) + 0x96) = 1;
  *(undefined1 *)(*(int *)((int)this + 0x878) + 0x850) = 1;
  local_4 = CONCAT31(local_4._1_3_,1);
  Audio_PlayMusicAtSpeed
            (*(void **)(*(int *)((int)this + 0x878) + 0x53c),"Main Theme - No Intro",2.0);
  *(undefined4 *)((int)this + 0x880) = 0;
  *(undefined1 *)((int)this + 0x884) = 0;
  *(undefined4 *)((int)this + 0xcbc) = 0;
  Difficulty_GetTimeModifier(*(void **)((int)this + 0x878),100000.0);
  uVar2 = __ftol2();
  *(int *)((int)this + 0xcb0) = (int)uVar2;
  uVar1 = 0x14 - *(int *)((int)*(void **)((int)this + 0x878) + 0x5f4);
  Difficulty_GetTimeModifier
            (*(void **)((int)this + 0x878),(float)(int)((uVar1 & ((int)uVar1 < 0) - 1) * 1000));
  uVar2 = __ftol2();
  *(int *)((int)this + 0xcb4) = (int)uVar2;
  *(undefined4 *)((int)this + 0xcac) = 0;
  *(undefined4 *)((int)this + 0xca8) = 0;
  *(undefined4 *)((int)this + 0xcb8) = 0x44480000;
  *(undefined4 *)((int)this + 0xcc0) = 0x19;
  *(undefined4 *)((int)this + 0xcc4) = 0x4b;
  *(undefined4 *)((int)this + 0xcc8) = 0xaf;
  *(undefined4 *)((int)this + 0xccc) = 0x113;
  *(undefined4 *)((int)this + 0xcd0) = 0x145;
  *(undefined4 *)((int)this + 0xcd4) = 0x187e4;
  *(undefined4 *)((int)this + 0xcd8) = 0x18816;
  *(undefined4 *)((int)this + 0xcdc) = 0x18848;
  *(undefined4 *)((int)this + 0xce0) = 0x18861;
  *(undefined4 *)((int)this + 0xce4) = 0x188c5;
  *(undefined4 *)((int)this + 0xce8) = 0x18910;
  *(undefined4 *)((int)this + 0xcec) = 0x30faf;
  *(undefined4 *)((int)this + 0xcf0) = 0x30fb9;
  *(undefined4 *)((int)this + 0xcf4) = 0x30fd2;
  *(undefined4 *)((int)this + 0xcf8) = 0x31036;
  CRT_remove("DATA\\tournament.sav");
  ExceptionList = local_c;
  return this;
}

