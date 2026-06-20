
void __thiscall MusicDevice_SetVolume(void *this,undefined4 param_1)

{
  ulonglong uVar1;
  
  *(undefined4 *)((int)this + 8) = param_1;
  uVar1 = __ftol2();
  BASS_SetConfig(6,(int)uVar1);
  return;
}

