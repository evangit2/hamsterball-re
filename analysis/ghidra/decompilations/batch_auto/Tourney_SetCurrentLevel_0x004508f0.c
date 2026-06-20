
void __thiscall Tourney_SetCurrentLevel(void *this,int param_1)

{
  undefined4 in_stack_ffffffe4;
  undefined4 in_stack_ffffffe8;
  undefined4 in_stack_ffffffec;
  undefined4 in_stack_fffffff0;
  undefined4 in_stack_fffffff4;
  char *pcVar1;
  
  *(undefined4 *)(*(int *)((int)this + 0x878) + 0x5e8) =
       *(undefined4 *)(*(int *)(*(int *)((int)this + 0x878) + 0x220) + 0x18 + param_1 * 4);
  *(float *)(*(int *)((int)this + 0x878) + 0x5e4) =
       (float)*(int *)(*(int *)(*(int *)((int)this + 0x878) + 0x220) + 0x54 + param_1 * 4);
  if (param_1 < 1) {
    pcVar1 = "ROLL";
    Vec3_Init(&stack0xffffffe4,0x3f400000,0x3f400000,0x3f400000);
    UIList_SetColorsByName
              (this,in_stack_ffffffe4,in_stack_ffffffe8,in_stack_ffffffec,in_stack_fffffff0,
               in_stack_fffffff4,pcVar1);
  }
  return;
}

