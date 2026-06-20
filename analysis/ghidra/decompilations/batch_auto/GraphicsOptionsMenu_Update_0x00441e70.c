
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall GraphicsOptionsMenu_Update(int *param_1)

{
  int *this;
  bool bVar1;
  float fVar2;
  undefined4 uVar3;
  int iVar4;
  float fStack_78;
  float fStack_74;
  float fStack_70;
  float fStack_6c;
  undefined4 uStack_68;
  float fStack_64;
  int iStack_60;
  float fStack_5c;
  float fStack_58;
  undefined **ppuStack_54;
  float fStack_50;
  int iStack_4c;
  float fStack_48;
  float fStack_44;
  undefined4 uStack_40;
  float fStack_3c;
  int iStack_38;
  float fStack_34;
  float fStack_30;
  undefined4 uStack_2c;
  float fStack_28;
  int iStack_24;
  float fStack_20;
  float fStack_1c;
  void *pvStack_14;
  undefined1 *puStack_10;
  undefined4 uStack_c;
  
  uStack_c = 0xffffffff;
  puStack_10 = &LAB_004cbeb8;
  pvStack_14 = ExceptionList;
  ExceptionList = &pvStack_14;
  if ((char)param_1[0x32f] != '\0') {
    ExceptionList = &pvStack_14;
    (**(code **)(*param_1 + 0x50))();
    this = param_1 + 0x113;
    thunk_Gfx_SetRenderState(this,(int)(param_1 + 0x33c));
    ppuStack_54 = &PTR_LAB_004cf584;
    fStack_44 = 0.0;
    fStack_48 = 0.0;
    iStack_4c = 0;
    fStack_50 = 0.0;
    uStack_c = 0;
    Scene_FindTextureByName(param_1,&uStack_68,"SV");
    uStack_c._0_1_ = 1;
    Scene_FindTextureDimensions(param_1,&uStack_40,"SV");
    param_1[0x33e] = iStack_60;
    fStack_50 = fStack_34 + fStack_3c + _DAT_004cf55c;
    iStack_4c = iStack_60;
    uStack_c = CONCAT31(uStack_c._1_3_,2);
    fStack_48 = (fStack_5c + fStack_64) - fStack_50;
    fStack_44 = fStack_58 - _DAT_004cf48c;
    param_1[0x340] = (int)fStack_44;
    param_1[0x33d] = (int)fStack_50;
    param_1[0x33f] = (int)fStack_48;
    param_1[0x344] = *(int *)(*(int *)(param_1[0x21e] + 0x178) + 0x838);
    param_1[0x343] = param_1[0x21e];
    param_1[0x345] = 0x42a00000;
    AthenaList_Append(this,(int)(param_1 + 0x33c));
    thunk_Gfx_SetRenderState(this,(int)(param_1 + 0x34a));
    Scene_FindTextureByName(param_1,&uStack_2c,"MV");
    iStack_60 = iStack_24;
    fStack_64 = fStack_28;
    fStack_5c = fStack_20;
    fStack_58 = fStack_1c;
    Scene_FindTextureDimensions(param_1,&uStack_2c,"MV");
    fStack_50 = fStack_20 + fStack_28 + _DAT_004cf55c;
    fStack_3c = fStack_28;
    iStack_38 = iStack_24;
    fStack_34 = fStack_20;
    fStack_30 = fStack_1c;
    param_1[0x34c] = iStack_60;
    fStack_48 = (fStack_5c + fStack_64) - fStack_50;
    iStack_4c = iStack_60;
    fStack_44 = fStack_58 - _DAT_004cf48c;
    param_1[0x34e] = (int)fStack_44;
    param_1[0x34b] = (int)fStack_50;
    param_1[0x34d] = (int)fStack_48;
    param_1[0x352] = *(int *)(*(int *)(param_1[0x21e] + 0x17c) + 8);
    param_1[0x351] = param_1[0x21e];
    param_1[0x353] = 0x42a00000;
    AthenaList_Append(this,(int)(param_1 + 0x34a));
    thunk_Gfx_SetRenderState(this,(int)(param_1 + 0x358));
    Scene_FindTextureByName(param_1,&uStack_2c,"TQ");
    iStack_60 = iStack_24;
    fStack_64 = fStack_28;
    fStack_5c = fStack_20;
    fStack_58 = fStack_1c;
    Scene_FindTextureDimensions(param_1,&uStack_2c,"TQ");
    fStack_50 = fStack_20 + fStack_28 + _DAT_004cf55c;
    fStack_3c = fStack_28;
    iStack_38 = iStack_24;
    fStack_34 = fStack_20;
    fStack_30 = fStack_1c;
    iStack_4c = iStack_60;
    fStack_48 = (fStack_5c + fStack_64) - fStack_50;
    param_1[0x35a] = iStack_60;
    fStack_44 = fStack_58 - _DAT_004cf48c;
    param_1[0x35c] = (int)fStack_44;
    param_1[0x359] = (int)fStack_50;
    param_1[0x35b] = (int)fStack_48;
    fVar2 = _DAT_004cf310 - *(float *)(*(int *)(param_1[0x21e] + 0x174) + 0x184);
    param_1[0x35f] = param_1[0x21e];
    param_1[0x361] = 0x42a00000;
    param_1[0x360] = (int)fVar2;
    AthenaList_Append(this,(int)(param_1 + 0x358));
    thunk_Gfx_SetRenderState(this,(int)(param_1 + 0x374));
    Scene_FindTextureByName(param_1,&uStack_2c,"MS");
    iStack_60 = iStack_24;
    fStack_64 = fStack_28;
    fStack_5c = fStack_20;
    fStack_58 = fStack_1c;
    Scene_FindTextureDimensions(param_1,&uStack_2c,"MS");
    fStack_50 = fStack_20 + fStack_28 + _DAT_004cf55c;
    fStack_3c = fStack_28;
    iStack_38 = iStack_24;
    fStack_34 = fStack_20;
    fStack_30 = fStack_1c;
    param_1[0x376] = iStack_60;
    fStack_48 = (fStack_5c + fStack_64) - fStack_50;
    iStack_4c = iStack_60;
    fStack_44 = fStack_58 - _DAT_004cf48c;
    param_1[0x378] = (int)fStack_44;
    param_1[0x375] = (int)fStack_50;
    param_1[0x377] = (int)fStack_48;
    iVar4 = *(int *)(param_1[0x21e] + 0x84c);
    param_1[0x37b] = param_1[0x21e];
    param_1[0x37c] = iVar4;
    param_1[0x37d] = 0x42a00000;
    AthenaList_Append(this,(int)(param_1 + 0x374));
  }
  uStack_c = 0xffffffff;
  if ((char)param_1[0x347] != '\0') {
    SoundDevice_Play3DAll(*(void **)(param_1[0x21e] + 0x178),param_1[0x344]);
    *(undefined1 *)(param_1 + 0x347) = 0;
  }
  if ((char)param_1[0x355] != '\0') {
    MusicDevice_SetVolume(*(void **)(param_1[0x21e] + 0x17c),param_1[0x352]);
    *(undefined1 *)(param_1 + 0x355) = 0;
  }
  if ((char)param_1[0x363] != '\0') {
    iVar4 = *(int *)(param_1[0x21e] + 0x174);
    fVar2 = _DAT_004cf310 - (float)param_1[0x360];
    fStack_78 = fVar2 * _DAT_004cf41c - _DAT_004cf48c;
    (**(code **)(**(int **)(iVar4 + 0x154) + 0xfc))(*(int **)(iVar4 + 0x154),0,0x13,fStack_78);
    *(int *)(iVar4 + 0x7c8) = *(int *)(iVar4 + 0x7c8) + 1;
    *(float *)(iVar4 + 0x184) = fVar2;
    *(undefined1 *)(param_1 + 0x363) = 0;
  }
  if ((char)param_1[0x37f] != '\0') {
    *(int *)(param_1[0x21e] + 0x84c) = param_1[0x37c];
    *(undefined1 *)(param_1 + 0x37f) = 0;
  }
  if (*(int **)(*(int *)(param_1[0x21e] + 0x184) + 0x424) == param_1) {
    uVar3 = Input_CheckKeyCombo((void *)param_1[0x21e],2);
    if ((char)uVar3 != '\0') {
      iVar4 = AthenaList_FindByValue(param_1 + 0x223,param_1[0x330]);
      if (iVar4 != -1) {
        (**(code **)(*param_1 + 0x48))(*(undefined4 *)(param_1[0x330] + 4));
      }
    }
    if (param_1[0x32e] != 0) {
      bVar1 = true;
      iVar4 = 0x550;
      do {
        if (*(int *)((int)*(void **)(iVar4 + param_1[0x21e]) + 8) != 2) {
          Ball_GetInputForce(*(void **)(iVar4 + param_1[0x21e]),&fStack_78);
          if (_DAT_004cf368 < fStack_74) {
            bVar1 = false;
          }
          if (fStack_74 < _DAT_004cf368) {
            bVar1 = false;
          }
          if (fStack_78 < _DAT_004cf368) {
            bVar1 = false;
          }
          if (_DAT_004cf368 < fStack_78) {
            bVar1 = false;
          }
        }
        iVar4 = iVar4 + 4;
      } while (iVar4 < 0x560);
      if (bVar1) {
        param_1[0x32e] = 0;
      }
    }
    iVar4 = param_1[0x32e];
    param_1[0x32e] = iVar4 + -1;
    if (iVar4 + -1 < 1) {
      param_1[0x32e] = 0;
      iVar4 = 0x550;
      do {
        if (*(int *)((int)*(void **)(iVar4 + param_1[0x21e]) + 8) != 2) {
          Ball_GetInputForce(*(void **)(iVar4 + param_1[0x21e]),&fStack_70);
          if (fStack_6c == _DAT_004cf368) {
LAB_004424b5:
            if (fStack_70 == _DAT_004cf368) goto LAB_0044251d;
          }
          else if (fStack_70 == _DAT_004cf368) {
            if (_DAT_004cf368 < fStack_6c) {
              (**(code **)(*param_1 + 0x20))(0x410);
              param_1[0x32e] = 0xf;
            }
            if (fStack_6c < _DAT_004cf368) {
              (**(code **)(*param_1 + 0x20))(0x40e);
              param_1[0x32e] = 0xf;
            }
            goto LAB_004424b5;
          }
          if (fStack_6c == _DAT_004cf368) {
            if (fStack_70 < _DAT_004cf368) {
              param_1[0x32e] = 10;
              (**(code **)(*param_1 + 0x20))(0x40d);
            }
            if (_DAT_004cf368 < fStack_70) {
              param_1[0x32e] = 10;
              (**(code **)(*param_1 + 0x20))(0x40f);
            }
          }
        }
LAB_0044251d:
        iVar4 = iVar4 + 4;
      } while (iVar4 < 0x560);
    }
  }
  ExceptionList = pvStack_14;
  return;
}

