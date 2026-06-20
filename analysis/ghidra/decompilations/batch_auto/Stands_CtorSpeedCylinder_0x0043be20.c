
void * __thiscall
Stands_CtorSpeedCylinder
          (void *this,undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,
          void *param_5,undefined1 *param_6)

{
  undefined1 *puVar1;
  undefined4 in_stack_ffffffe8;
  undefined4 in_stack_ffffffec;
  undefined4 in_stack_fffffff0;
  
  puVar1 = param_6;
  param_6 = &stack0xffffffe8;
  if ((undefined4 *)&stack0xffffffe8 != &param_2) {
    in_stack_ffffffe8 = param_2;
    in_stack_ffffffec = param_3;
    in_stack_fffffff0 = param_4;
  }
  Stands_CtorCollision
            (this,param_1,in_stack_ffffffe8,in_stack_ffffffec,in_stack_fffffff0,param_5,puVar1);
  *(undefined ***)this = &PTR_Button_DeletingDtor_004d5ca0;
  *(undefined1 *)((int)this + 0x1110) = 1;
  *(undefined4 *)((int)this + 0x10f0) = 0xc2c80000;
  *(undefined4 *)((int)this + 0x10f4) = 2;
  *(undefined4 *)((int)this + 0x1114) = 0x3f800000;
  return this;
}

