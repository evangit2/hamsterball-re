
void __cdecl PNG_Close(undefined4 *param_1,undefined4 *param_2,undefined4 *param_3)

{
  undefined4 *puVar1;
  undefined4 local_c;
  undefined4 local_8;
  
  puVar1 = (undefined4 *)0x0;
  local_8 = (undefined4 *)0x0;
  local_c = (undefined4 *)0x0;
  if (param_1 != (undefined4 *)0x0) {
    local_8 = (undefined4 *)*param_1;
  }
  if (param_2 != (undefined4 *)0x0) {
    puVar1 = (undefined4 *)*param_2;
  }
  if (param_3 != (undefined4 *)0x0) {
    local_c = (undefined4 *)*param_3;
  }
  if (local_8 != (undefined4 *)0x0) {
    PNG_Free(local_8,puVar1,local_c);
  }
  if (puVar1 != (undefined4 *)0x0) {
    CRT_FreeIfNotNull(puVar1);
    *param_2 = 0;
  }
  if (local_c != (undefined4 *)0x0) {
    CRT_FreeIfNotNull(local_c);
    *param_3 = 0;
  }
  if (local_8 != (undefined4 *)0x0) {
    CRT_FreeIfNotNull(local_8);
    *param_1 = 0;
  }
  return;
}

