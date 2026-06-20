
void __fastcall Sound_LoadOggOrWav(void *param_1)

{
  int iVar1;
  char local_100 [256];
  
  AthenaString_SprintfToBuffer(local_100,(byte *)"%s.ogg");
  iVar1 = _check_file_access(local_100,0);
  if (iVar1 == 0) {
    Sound_LoadOgg(param_1,local_100);
    return;
  }
  AthenaString_SprintfToBuffer(local_100,(byte *)"%s.wav");
  iVar1 = _check_file_access(local_100,0);
  if (iVar1 == 0) {
    SoundList_LoadWAV(param_1,local_100);
  }
  return;
}

