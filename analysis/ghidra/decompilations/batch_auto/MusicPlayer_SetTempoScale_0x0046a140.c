
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall MusicPlayer_SetTempoScale(void *this,float param_1)

{
  int iVar1;
  
  iVar1 = *(int *)(*(int *)(*(int *)((int)this + 4) + 4) + 0x16c);
  *(undefined1 *)((int)this + 0x530) = 1;
  *(undefined1 *)((int)this + 0x531) = 0;
  *(float *)((int)this + 0x52c) = ((float)iVar1 / (param_1 * _DAT_004cf454)) * (float)_DAT_004cf538;
  return;
}

