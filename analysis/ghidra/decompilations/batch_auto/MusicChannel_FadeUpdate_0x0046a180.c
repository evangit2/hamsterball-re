
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall MusicChannel_FadeUpdate(int param_1)

{
  float fVar1;
  ulonglong uVar2;
  undefined4 uVar3;
  
  if (*(char *)(param_1 + 0x531) != '\0') {
    fVar1 = *(float *)(param_1 + 0x52c) + *(float *)(param_1 + 0x528);
    *(float *)(param_1 + 0x528) = fVar1;
    if (_DAT_004cf310 <= fVar1) {
      *(undefined1 *)(param_1 + 0x531) = 0;
      *(undefined4 *)(param_1 + 0x528) = 0x3f800000;
    }
    uVar3 = 0xffffff9b;
    *(undefined4 *)(param_1 + 0x528) = *(undefined4 *)(param_1 + 0x528);
    uVar2 = __ftol2();
    BASS_ChannelSetAttributes(*(undefined4 *)(param_1 + 8),0xffffffff,(int)uVar2,uVar3);
  }
  if (*(char *)(param_1 + 0x530) != '\0') {
    fVar1 = *(float *)(param_1 + 0x528) - *(float *)(param_1 + 0x52c);
    *(float *)(param_1 + 0x528) = fVar1;
    if ((!NAN(fVar1) && !NAN(_DAT_004cf368)) && fVar1 < _DAT_004cf368 != (fVar1 == _DAT_004cf368)) {
      *(undefined1 *)(param_1 + 0x530) = 0;
      *(undefined4 *)(param_1 + 0x528) = 0;
      BASS_ChannelStop(*(undefined4 *)(param_1 + 8));
    }
    uVar3 = 0xffffff9b;
    *(undefined4 *)(param_1 + 0x528) = *(undefined4 *)(param_1 + 0x528);
    uVar2 = __ftol2();
    BASS_ChannelSetAttributes(*(undefined4 *)(param_1 + 8),0xffffffff,(int)uVar2,uVar3);
  }
  return;
}

