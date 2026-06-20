
void __cdecl PNG_Free(undefined4 *param_1,undefined4 *param_2,undefined4 *param_3)

{
  undefined4 uVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  int iVar4;
  undefined4 *puVar5;
  int iVar6;
  undefined4 *puVar7;
  undefined4 local_44 [16];
  
  iVar6 = 0;
  if (param_2 != (undefined4 *)0x0) {
    Color_ZeroVertexBlock(param_1,param_2);
  }
  if (param_3 != (undefined4 *)0x0) {
    Color_ZeroVertexBlock(param_1,param_3);
  }
  Zlib_FreeIf((int)param_1,(void *)param_1[0x27]);
  Zlib_FreeIf((int)param_1,(void *)param_1[0x37]);
  Zlib_FreeIf((int)param_1,(void *)param_1[0x36]);
  Zlib_FreeIf((int)param_1,(void *)param_1[0x5d]);
  Zlib_FreeIf((int)param_1,(void *)param_1[0x5e]);
  Zlib_FreeIf((int)param_1,(void *)param_1[0x4e]);
  if ((*(byte *)((int)param_1 + 0x5d) & 0x10) != 0) {
    Zlib_FreeIf((int)param_1,(void *)param_1[0x41]);
  }
  if ((*(byte *)((int)param_1 + 0x5d) & 0x20) != 0) {
    Zlib_FreeIf((int)param_1,(void *)param_1[0x57]);
  }
  if (param_1[0x51] != 0) {
    iVar4 = 1 << (8U - (char)param_1[0x4b] & 0x1f);
    if (0 < iVar4) {
      do {
        Zlib_FreeIf((int)param_1,*(void **)(param_1[0x51] + iVar6 * 4));
        iVar6 = iVar6 + 1;
      } while (iVar6 < iVar4);
    }
    Zlib_FreeIf((int)param_1,(void *)param_1[0x51]);
  }
  Audio_CodecDestroyContext((int)(param_1 + 0x19));
  uVar1 = param_1[0x12];
  uVar2 = param_1[0x10];
  puVar5 = param_1;
  puVar7 = local_44;
  for (iVar6 = 0x10; iVar6 != 0; iVar6 = iVar6 + -1) {
    *puVar7 = *puVar5;
    puVar5 = puVar5 + 1;
    puVar7 = puVar7 + 1;
  }
  uVar3 = param_1[0x11];
  puVar5 = param_1;
  for (iVar6 = 0x67; iVar6 != 0; iVar6 = iVar6 + -1) {
    *puVar5 = 0;
    puVar5 = puVar5 + 1;
  }
  param_1[0x11] = uVar3;
  puVar5 = local_44;
  puVar7 = param_1;
  for (iVar6 = 0x10; iVar6 != 0; iVar6 = iVar6 + -1) {
    *puVar7 = *puVar5;
    puVar5 = puVar5 + 1;
    puVar7 = puVar7 + 1;
  }
  param_1[0x10] = uVar2;
  param_1[0x12] = uVar1;
  return;
}

