
void __fastcall SceneList_Reset(undefined4 *param_1)

{
  *param_1 = &PTR_SceneList_DeletingDtor_004da6b4;
  Vec3List_Free(param_1 + 1);
  return;
}

