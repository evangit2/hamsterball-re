
void __thiscall
TourneyMenu_DrawHighlightQuad
          (void *this,float param_1,float param_2,float param_3,float param_4,undefined4 param_5,
          undefined4 param_6,undefined4 param_7,undefined4 param_8,undefined4 param_9)

{
  undefined **local_24;
  undefined4 local_20;
  undefined4 local_1c;
  undefined4 local_18;
  undefined4 local_14;
  undefined1 *local_10;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  puStack_8 = &LAB_004cc9b8;
  local_c = ExceptionList;
  local_10 = (undefined1 *)&local_24;
  local_24 = &PTR_Vec3_dtor_004cf300;
  local_20 = param_6;
  local_1c = param_7;
  local_18 = param_8;
  local_14 = param_9;
  local_4 = 0;
  ExceptionList = &local_c;
  Sprite_DrawRotatedQuad(this,param_1,param_2,param_3,param_4);
  local_4 = 0xffffffff;
  local_14 = 0x450a83;
  Matrix_Identity(&param_5);
  ExceptionList = local_c;
  return;
}

