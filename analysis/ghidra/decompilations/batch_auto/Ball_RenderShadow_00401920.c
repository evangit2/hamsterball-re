/*
 * Function: Ball_RenderShadow
 * Address: 0x00401920
 * Signature: Ball_RenderShadow(...)
 *
 * Patterns: vtable dispatch, SEH frame, rendering, ball. Calls: Ball_RenderShadow, Timer_Init, Gfx_SetPosition, Timer_Cleanup. Offsets: 11, Lines: 56
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall Ball_RenderShadow(int param_1)

{
  int *piVar1;
  int iVar2;
  int **ppiVar3;
  undefined4 *puVar4;
  int *piStack_a8;
  float fStack_a4;
  int iStack_a0;
  undefined **ppuStack_68;
  undefined4 auStack_64 [16];
  void *pvStack_24;
  undefined1 uStack_1c;
  uint uStack_10;
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 uStack_4;
  
  uStack_4 = 0xffffffff;
  puStack_8 = &LAB_004c91d3;
  pvStack_c = ExceptionList;
  piStack_a8 = *(int **)(*(int *)(*(int *)(param_1 + 0x10) + 0x174) + 0x154);
  iStack_a0 = 0;
  fStack_a4 = 2.35418e-43;
  ExceptionList = &pvStack_c;
  (**(code **)(*piStack_a8 + 200))();
  if (*(int *)(param_1 + 0x754) < 3) {
    Timer_Init(&iStack_a0);
    fStack_a4 = *(float *)(param_1 + 0x284) * (float)_DAT_004cf388;
    uStack_10 = 0;
    (**(code **)(iStack_a0 + 0x18))(fStack_a4,fStack_a4,fStack_a4);
    Gfx_SetPosition(*(undefined4 *)(param_1 + 0x164),*(undefined4 *)(param_1 + 0x168),
                    *(undefined4 *)(param_1 + 0x16c));
    ppuStack_68 = &PTR_Timer_dtor_004cf338;
    ppiVar3 = &piStack_a8;
    puVar4 = auStack_64;
    for (iVar2 = 0x10; iVar2 != 0; iVar2 = iVar2 + -1) {
      *puVar4 = *ppiVar3;
      ppiVar3 = ppiVar3 + 1;
      puVar4 = puVar4 + 1;
    }
    uStack_1c = 1;
    (**(code **)(**(int **)(*(int *)(param_1 + 0x10) + 0x244 + *(int *)(param_1 + 0x754) * 4) + 0x1c
                ))(&ppuStack_68,0);
    uStack_10 = uStack_10 & 0xffffff00;
    Timer_Cleanup(auStack_64 + 2);
    uStack_10 = 0xffffffff;
    Timer_Cleanup(&iStack_a0);
  }
  piVar1 = *(int **)(*(int *)(*(int *)(param_1 + 0x10) + 0x174) + 0x154);
  (**(code **)(*piVar1 + 200))(piVar1,0xa8,0xf);
  ExceptionList = pvStack_24;
  return;
}
