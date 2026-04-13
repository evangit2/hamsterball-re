
void __fastcall FUN_00405100(void *param_1)

{
  undefined4 in_stack_ffffffe4;
  float in_stack_ffffffe8;
  float in_stack_ffffffec;
  float in_stack_fffffff0;
  float fVar1;
  
  fVar1 = 5.906542e-39;
  FUN_004015b0((int)param_1);
  FUN_00453180(&stack0xffffffe4,0,0,0);
  FUN_00403850(param_1,in_stack_ffffffe4,in_stack_ffffffe8,in_stack_ffffffec,in_stack_fffffff0,fVar1
              );
  *(undefined4 *)((int)param_1 + 0x18) = 0xffffffff;
  *(undefined4 *)((int)param_1 + 0x278) = 0x3f000000;
  *(undefined4 *)((int)param_1 + 0x27c) = 0x3e4ccccd;
  *(undefined4 *)((int)param_1 + 0x284) = 0x420c0000;
  *(undefined4 *)((int)param_1 + 0x1a0) = 0x3e4ccccd;
  *(undefined4 *)((int)param_1 + 0x188) = 0x40c00000;
  *(undefined4 *)((int)param_1 + 0xc78) = 0;
  *(undefined4 *)((int)param_1 + 0xc7c) = 0x42480000;
  *(undefined4 *)((int)param_1 + 0xc6c) = 0x44160000;
  *(undefined4 *)((int)param_1 + 0xc70) = 0x44960000;
  return;
}
