
void __fastcall Sprite_Reset(undefined4 *param_1)

{
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  puStack_8 = &LAB_004cce1b;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  *param_1 = &PTR_Sprite_ScalarDtor_004d8f84;
  local_4 = 0;
  if ((*(char *)(param_1 + 0x34) != '\0') && ((undefined4 *)param_1[0x14] != (undefined4 *)0x0)) {
    Texture_RemoveRef((void *)param_1[1],(undefined4 *)param_1[0x14]);
    param_1[0x14] = 0;
  }
  local_4 = 0xffffffff;
  Matrix4_Identity(param_1 + 2);
  ExceptionList = local_c;
  return;
}

