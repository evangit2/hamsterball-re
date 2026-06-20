
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall Audio_PlayMusicAtSpeed(void *this,char *param_1,float param_2)

{
  *(undefined1 *)((int)this + 0x531) = 1;
  *(undefined1 *)((int)this + 0x530) = 0;
  *(float *)((int)this + 0x52c) =
       ((float)*(int *)(*(int *)(*(int *)((int)this + 4) + 4) + 0x16c) / (param_2 * _DAT_004cf454))
       * (float)_DAT_004cf538;
  Audio_PlayMusic(this,param_1);
  *(undefined4 *)((int)this + 0x528) = 0;
  BASS_ChannelSetAttributes(*(undefined4 *)((int)this + 8),0xffffffff,0,0xffffff9b);
  return;
}

