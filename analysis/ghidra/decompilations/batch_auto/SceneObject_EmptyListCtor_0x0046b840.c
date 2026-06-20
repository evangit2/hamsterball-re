
undefined4 * __fastcall SceneObject_EmptyListCtor(undefined4 *param_1)

{
  *param_1 = &PTR_SceneObject_DeletingDtor_004d9368;
  AthenaList_Init(param_1 + 1,0);
  return param_1;
}

