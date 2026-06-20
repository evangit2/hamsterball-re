
void __cdecl Audio_Upscale2xStereo(int param_1,undefined4 param_2,undefined4 *param_3,int *param_4)

{
  undefined1 uVar1;
  int iVar2;
  undefined1 *puVar3;
  undefined1 *puVar4;
  undefined1 *puVar5;
  int iVar6;
  
  iVar2 = *param_4;
  iVar6 = 0;
  if (0 < *(int *)(param_1 + 0x110)) {
    param_4 = param_3;
    do {
      puVar5 = (undefined1 *)*param_4;
      puVar4 = *(undefined1 **)(iVar2 + iVar6 * 4);
      puVar3 = puVar4 + *(int *)(param_1 + 0x5c);
      for (; puVar4 < puVar3; puVar4 = puVar4 + 2) {
        uVar1 = *puVar5;
        puVar5 = puVar5 + 1;
        *puVar4 = uVar1;
        puVar4[1] = uVar1;
      }
      Array_CopyElements(iVar2,iVar6,iVar2,iVar6 + 1,1,*(uint *)(param_1 + 0x5c));
      param_4 = param_4 + 1;
      iVar6 = iVar6 + 2;
    } while (iVar6 < *(int *)(param_1 + 0x110));
  }
  return;
}

