
void __fastcall Audio_StopChannel(int param_1)

{
  BASS_ChannelStop(*(undefined4 *)(param_1 + 8));
  return;
}

