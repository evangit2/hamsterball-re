
void __fastcall Scene_RenderBallShadow(int param_1)

{
  undefined4 uVar1;
  int *piVar2;
  int iVar3;
  int iVar4;
  int unaff_ESI;
  int iVar5;
  
  Ball_InitRenderState(*(void **)(param_1 + 4));
  uVar1 = *(undefined4 *)(*(int *)(param_1 + 0x480) + 0x444);
  iVar5 = *(int *)(param_1 + 4);
  piVar2 = *(int **)(iVar5 + 0x154);
  (**(code **)(*piVar2 + 0x14c))(piVar2,0,uVar1,0x20);
  *(undefined4 *)(iVar5 + 8) = uVar1;
  Graphics_BeginFrame(*(void **)(param_1 + 4),*(int *)(param_1 + 0x434));
  iVar4 = AthenaList_NextIndex(unaff_ESI);
  iVar5 = 0;
  *(undefined4 *)(unaff_ESI + 8 + iVar4 * 4) = 0;
  if (0 < *(int *)(unaff_ESI + 4)) {
    iVar5 = **(int **)(unaff_ESI + 0x40c);
    *(undefined4 *)(unaff_ESI + 8 + iVar4 * 4) = 1;
  }
  while( true ) {
    if (iVar5 == 0) {
      return;
    }
    if (*(int *)(iVar5 + 0x83c) == *(int *)((int)*(void **)(param_1 + 4) + 0x7c4)) {
      Graphics_ApplyMaterialAndDraw
                (*(void **)(param_1 + 4),
                 (undefined4 *)(*(int *)(iVar5 + 4) * 0x50 + *(int *)(*(int *)(param_1 + 8) + 0x28))
                );
      piVar2 = *(int **)(*(int *)(*(int *)(param_1 + 0x47c) + 0x480) + 0x44c);
      (**(code **)(*piVar2 + 0xc))(piVar2,*(undefined4 *)(iVar5 + 0x858));
    }
    iVar3 = *(int *)(unaff_ESI + 8 + iVar4 * 4);
    if (*(int *)(unaff_ESI + 4) <= iVar3) break;
    iVar5 = *(int *)(*(int *)(unaff_ESI + 0x40c) + iVar3 * 4);
    *(int *)(unaff_ESI + 8 + iVar4 * 4) = iVar3 + 1;
  }
  return;
}

