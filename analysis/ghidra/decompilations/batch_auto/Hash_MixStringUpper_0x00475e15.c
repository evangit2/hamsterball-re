
uint __fastcall
Hash_MixStringUpper(undefined4 param_1,undefined4 param_2,uint param_3,byte *param_4,byte *param_5)

{
  byte bVar1;
  uint in_EAX;
  uint uVar2;
  undefined2 uVar3;
  undefined4 extraout_ECX;
  undefined4 extraout_ECX_00;
  undefined4 extraout_ECX_01;
  undefined4 extraout_EDX;
  undefined4 extraout_EDX_00;
  undefined4 extraout_EDX_01;
  
  if (param_4 != (byte *)0x0) {
    for (; bVar1 = *param_4, in_EAX = CONCAT31((int3)(in_EAX >> 8),bVar1), bVar1 != 0;
        param_4 = param_4 + 1) {
      uVar2 = Char_ToUpper(bVar1);
      in_EAX = Hash_MixKey(extraout_ECX,extraout_EDX,param_3,(byte)uVar2);
      param_1 = extraout_ECX_00;
      param_2 = extraout_EDX_00;
      param_3 = in_EAX;
    }
  }
  uVar3 = (undefined2)(in_EAX >> 0x10);
  if (param_5 != (byte *)0x0) {
    for (; uVar3 = (undefined2)(in_EAX >> 0x10), *param_5 != 0; param_5 = param_5 + 1) {
      in_EAX = Hash_MixKey(param_1,param_2,param_3,*param_5);
      param_1 = extraout_ECX_01;
      param_2 = extraout_EDX_01;
      param_3 = in_EAX;
    }
  }
  return CONCAT22(uVar3,(undefined2)param_3);
}

