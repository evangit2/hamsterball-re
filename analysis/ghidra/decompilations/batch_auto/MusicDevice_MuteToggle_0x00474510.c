
void MusicDevice_MuteToggle(char param_1)

{
  ulonglong uVar1;
  
  if (param_1 == '\0') {
    BASS_SetConfig(6,0);
    return;
  }
  uVar1 = __ftol2();
  BASS_SetConfig(6,(int)uVar1);
  return;
}

