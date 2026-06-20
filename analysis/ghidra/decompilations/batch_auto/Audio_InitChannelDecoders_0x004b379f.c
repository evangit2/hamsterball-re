
void __cdecl Audio_InitChannelDecoders(undefined4 *param_1)

{
  undefined4 *puVar1;
  undefined4 *puVar2;
  undefined4 *puVar3;
  int iVar4;
  int local_8;
  
  puVar1 = param_1;
  puVar2 = (undefined4 *)(**(code **)param_1[1])(param_1,1,0x54);
  local_8 = 0;
  param_1[0x66] = puVar2;
  *puVar2 = Audio_SetChannelDecoder;
  if (0 < (int)param_1[8]) {
    puVar2 = puVar2 + 0xb;
    param_1 = (undefined4 *)(param_1[0x31] + 0x50);
    do {
      puVar3 = (undefined4 *)(**(code **)puVar1[1])(puVar1,1,0x100);
      *param_1 = puVar3;
      for (iVar4 = 0x40; iVar4 != 0; iVar4 = iVar4 + -1) {
        *puVar3 = 0;
        puVar3 = puVar3 + 1;
      }
      *puVar2 = 0xffffffff;
      local_8 = local_8 + 1;
      puVar2 = puVar2 + 1;
      param_1 = param_1 + 0x15;
    } while (local_8 < (int)puVar1[8]);
  }
  return;
}

