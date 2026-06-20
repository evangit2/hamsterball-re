
void * __thiscall Audio_Init(void *this,int param_1)

{
  int iVar1;
  ulonglong uVar2;
  char acStack_220 [4];
  undefined4 uStack_21c;
  void *pvStack_28;
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cdb5e;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  *(undefined ***)this = &PTR_MusicDevice_DeletingDtor_004da09c;
  uStack_21c = 0x474572;
  AthenaList_Init((void *)((int)this + 0xc),0);
  *(int *)((int)this + 4) = param_1;
  *(undefined4 *)((int)this + 8) = 0x3f800000;
  uStack_21c = *(undefined4 *)(param_1 + 8);
  acStack_220[0] = '\0';
  acStack_220[1] = '\0';
  acStack_220[2] = '\0';
  acStack_220[3] = '\0';
  local_4 = 0;
  iVar1 = BASS_Init(1,0xac44);
  if ((iVar1 == 0) &&
     (iVar1 = BASS_Init(0,0xac44,0,*(undefined4 *)(*(int *)((int)this + 4) + 8),0), iVar1 == 0)) {
    BASS_ErrorGetCode();
    Window_Notify(0x5341d0,(byte *)"Music Initialization Error: %s");
    AthenaString_SprintfToBuffer
              (acStack_220,
               (byte *)
               "The BASS music interface could not initialize because:\n\n[%s]\n\nMusic will not play during the game."
              );
    MessageBoxA((HWND)0x0,acStack_220,"Music Interface Error",0);
  }
  BASS_Start();
  MusicDevice_ReadVolume((int)this);
  uVar2 = __ftol2();
  BASS_SetConfig(6,(int)uVar2);
  ExceptionList = pvStack_28;
  return this;
}

