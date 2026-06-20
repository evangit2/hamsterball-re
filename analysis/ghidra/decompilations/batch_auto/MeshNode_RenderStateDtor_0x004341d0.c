
void __fastcall MeshNode_RenderStateDtor(undefined4 *param_1)

{
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  puStack_8 = &LAB_004cba48;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  *param_1 = &PTR_MeshNode_RenderState_DeleteDtor_004d4ed8;
  local_4 = 0;
  thunk_Gfx_SetRenderState((void *)(*(int *)(param_1[0x434] + 0x8b0) + 0x18),param_1[0x435]);
  if ((undefined4 *)param_1[0x435] != (undefined4 *)0x0) {
    (*(code *)**(undefined4 **)param_1[0x435])(1);
  }
  param_1[0x435] = 0;
  local_4 = 0xffffffff;
  Level_Cleanup(param_1);
  ExceptionList = pvStack_c;
  return;
}

