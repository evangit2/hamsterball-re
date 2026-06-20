
void __fastcall MeshBuffer_Cleanup(undefined4 *param_1)

{
  *param_1 = &PTR_SpriteAnim_DeletingDtor_004d9c48;
  if (*(char *)((int)param_1 + 0xd) != '\0') {
    if ((undefined4 *)param_1[2] != (undefined4 *)0x0) {
      (*(code *)**(undefined4 **)param_1[2])(1);
    }
    param_1[2] = 0;
  }
  if (*(char *)((int)param_1 + 0xe) != '\0') {
    if ((void *)param_1[4] != (void *)0x0) {
      _free((void *)param_1[4]);
    }
    param_1[4] = 0;
  }
  return;
}

