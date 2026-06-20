
void __fastcall DSound_SetVolume(undefined4 param_1,undefined4 param_2)

{
  int *in_EAX;
  
  *(undefined4 *)(*in_EAX + 0x14) = 0x35;
  *(undefined4 *)(*in_EAX + 0x18) = param_2;
  (**(code **)*in_EAX)();
  return;
}

