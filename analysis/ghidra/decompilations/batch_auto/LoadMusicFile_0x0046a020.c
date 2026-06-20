
uint __thiscall LoadMusicFile(void *this,char *param_1)

{
  char cVar1;
  char *pcVar2;
  int iVar3;
  uint uVar4;
  
  pcVar2 = param_1;
  do {
    cVar1 = *pcVar2;
    pcVar2[(int)this + (0x424 - (int)param_1)] = cVar1;
    pcVar2 = pcVar2 + 1;
  } while (cVar1 != '\0');
  iVar3 = BASS_MusicLoad(0,param_1,0,0,4,0);
  *(int *)((int)this + 8) = iVar3;
  if (iVar3 != 0) {
    return CONCAT31((int3)((uint)iVar3 >> 8),1);
  }
  BASS_ErrorGetCode();
  uVar4 = Window_Notify(0x5341d0,(byte *)"Music Initialization Error: %s");
  return uVar4 & 0xffffff00;
}

