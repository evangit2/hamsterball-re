
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall SoundDevice_UpdateChannels(int param_1)

{
  float fVar1;
  int iVar2;
  int iVar3;
  float *_Memory;
  
  iVar3 = AthenaList_NextIndex(param_1 + 0x41c);
  *(undefined4 *)(param_1 + 0x424 + iVar3 * 4) = 0;
  if (*(int *)(param_1 + 0x420) < 1) {
    _Memory = (float *)0x0;
  }
  else {
    _Memory = (float *)**(undefined4 **)(param_1 + 0x828);
    *(undefined4 *)(param_1 + 0x424 + iVar3 * 4) = 1;
  }
  while( true ) {
    if (_Memory == (float *)0x0) {
      return;
    }
    fVar1 = _Memory[2];
    _Memory[2] = (float)((int)fVar1 + -1);
    if ((int)fVar1 + -1 < 1) {
      _Memory[2] = _Memory[3];
      Sound_PlayChannel((int)_Memory[1]);
      fVar1 = *_Memory - _DAT_004cf374;
      *_Memory = fVar1;
      if (fVar1 < _DAT_004cf368 != (fVar1 == _DAT_004cf368)) {
        thunk_Gfx_SetRenderState((void *)(param_1 + 0x41c),(int)_Memory);
        _free(_Memory);
      }
    }
    iVar2 = *(int *)(param_1 + 0x424 + iVar3 * 4);
    if (*(int *)(param_1 + 0x420) <= iVar2) break;
    _Memory = *(float **)(*(int *)(param_1 + 0x828) + iVar2 * 4);
    *(int *)(param_1 + 0x424 + iVar3 * 4) = iVar2 + 1;
  }
  return;
}

