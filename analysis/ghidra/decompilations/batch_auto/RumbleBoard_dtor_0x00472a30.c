
void __fastcall RumbleBoard_dtor(undefined4 *param_1)

{
  *param_1 = &PTR_RumbleBoard_DeletingDtor_004d9e68;
  _free((void *)param_1[0x21e]);
  SceneObject_dtor(param_1);
  return;
}

