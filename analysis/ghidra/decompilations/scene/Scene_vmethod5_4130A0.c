// Function: Scene_vmethod5
// Address: 0x4130A0
// Decompiled: 2026-06-21
// Source: GhidraMCP (create_function + decompile)


void __thiscall Scene_vmethod5(void *param_1,undefined4 param_2,undefined4 param_3,int param_4)

{
  if (((param_4 == 1) && (*(char *)(*(int *)((int)param_1 + 0x878) + 0x238) != '\0')) &&
     (*(char *)(*(int *)(*(int *)((int)param_1 + 0x878) + 0x220) + 0x95) == '\0')) {
    Scene_CreateGameOverMenu(param_1,'\x01');
  }
  return;
}

