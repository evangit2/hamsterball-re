
undefined4 * SpriteAnim_InitFromCalcTexCoords(undefined4 *param_1)

{
  undefined4 local_28;
  undefined4 local_24;
  undefined4 local_20;
  undefined4 local_1c;
  undefined4 local_18;
  undefined4 local_14;
  undefined4 local_10;
  void *local_c;
  undefined1 *puStack_8;
  uint local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cd7cf;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  D3DXSkinMesh_InitTimerDefaults(&local_28);
  local_4 = 1;
  Sprite_CalcTexCoords((int)&local_28);
  Sprite_CalcTexCoords((int)&local_28);
  Sprite_CalcTexCoords((int)&local_28);
  Sprite_CalcTexCoords((int)&local_28);
  Sprite_CalcTexCoords((int)&local_28);
  Sprite_CalcTexCoords((int)&local_28);
  Sprite_CalcTexCoords((int)&local_28);
  Sprite_CalcTexCoords((int)&local_28);
  param_1[1] = local_24;
  param_1[2] = local_20;
  param_1[3] = local_1c;
  *param_1 = &PTR_LAB_004d9c44;
  param_1[4] = local_18;
  param_1[5] = local_14;
  param_1[6] = local_10;
  local_4 = local_4 & 0xffffff00;
  StreamReaderVtbl_Init(&local_28);
  ExceptionList = local_c;
  return param_1;
}

