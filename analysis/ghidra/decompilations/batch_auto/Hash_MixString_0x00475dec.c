
uint __fastcall Hash_MixString(undefined4 param_1,undefined4 param_2,uint param_3,byte *param_4)

{
  uint in_EAX;
  undefined2 uVar1;
  undefined4 extraout_ECX;
  undefined4 extraout_EDX;
  
  uVar1 = (undefined2)(in_EAX >> 0x10);
  if (param_4 != (byte *)0x0) {
    for (; uVar1 = (undefined2)(in_EAX >> 0x10), *param_4 != 0; param_4 = param_4 + 1) {
      in_EAX = Hash_MixKey(param_1,param_2,param_3,*param_4);
      param_1 = extraout_ECX;
      param_2 = extraout_EDX;
      param_3 = in_EAX;
    }
  }
  return CONCAT22(uVar1,(undefined2)param_3);
}

