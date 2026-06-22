// Function: Scene_DestroyScene
// Address: 0x409DA0
// Decompiled: 2026-06-21
// Source: GhidraMCP (create_function + decompile)


void __thiscall Scene_DestroyScene(int *param_1,void *param_2)

{
  Scene_AddAllObjects(param_1,param_2);
  (**(code **)(*param_1 + 0x44))(1);
  return;
}

