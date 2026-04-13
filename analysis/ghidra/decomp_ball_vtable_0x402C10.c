
void __fastcall FUN_00402c10(int param_1)

{
  undefined4 uVar1;
  undefined4 uVar2;
  int iVar3;
  int *piVar4;
  short sVar5;
  void *this;
  ulonglong uVar6;
  undefined4 uVar7;
  undefined4 uVar8;
  char cVar9;
  undefined4 auStack_50 [17];
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 uStack_4;
  
  uStack_4 = 0xffffffff;
  puStack_8 = &LAB_004c91e8;
  pvStack_c = ExceptionList;
  if (*(char *)(param_1 + 0x324) == '\0') {
    ExceptionList = &pvStack_c;
    Timer_Init(auStack_50);
    uStack_4 = 0;
    Graphics_BeginFrame(*(void **)(*(int *)(param_1 + 0x10) + 0x174),(int)auStack_50);
    this = *(void **)(*(int *)(param_1 + 0x10) + 0x174);
    sVar5 = Ball_CheckCollisionPlanes
                      (*(void **)((int)this + 0x748),*(float *)(param_1 + 0x164),
                       *(float *)(param_1 + 0x168),*(float *)(param_1 + 0x16c),100.0);
    if ((char)sVar5 != '\0') {
      FUN_00401220(this);
      iVar3 = *(int *)(*(int *)(param_1 + 0x10) + 0x174);
      if ((*(char *)(iVar3 + 0x182) != '\0') && (*(char *)(param_1 + 0x280) != '\0')) {
        piVar4 = *(int **)(iVar3 + 0x154);
        (**(code **)(*piVar4 + 200))(piVar4,0x37,1);
      }
      if (*(int *)(param_1 + 0x2fc) != 0x3f800000) {
        FUN_004011c0(*(void **)(*(int *)(param_1 + 0x10) + 0x174),*(int *)(param_1 + 0x2fc));
      }
      uVar1 = *(undefined4 *)(param_1 + 0x16c);
      uVar2 = *(undefined4 *)(param_1 + 0x168);
      iVar3 = *(int *)(param_1 + 0x164);
      uVar7 = *(undefined4 *)(*(int *)(param_1 + 0x10) + 0x278);
      cVar9 = '\x01';
      uVar8 = 0x3f800000;
      uVar6 = FUN_004ba754();
      FUN_0045ec30(*(void **)(*(int *)(param_1 + 0x14) + 0x8ac),iVar3,uVar2,uVar1,(int)uVar6,uVar7,
                   uVar8,cVar9);
      if (*(int *)(param_1 + 0x2fc) != 0x3f800000) {
        *(undefined1 *)(*(int *)(*(int *)(param_1 + 0x10) + 0x174) + 0x7a8) = 0;
      }
      iVar3 = *(int *)(*(int *)(param_1 + 0x10) + 0x174);
      if ((*(char *)(iVar3 + 0x182) != '\0') && (*(char *)(param_1 + 0x280) != '\0')) {
        piVar4 = *(int **)(iVar3 + 0x154);
        (**(code **)(*piVar4 + 200))(piVar4,0x37,3);
      }
      FUN_00401270(*(void **)(*(int *)(param_1 + 0x10) + 0x174));
      this = *(void **)(*(int *)(param_1 + 0x10) + 0x174);
    }
    Graphics_BeginFrame(this,0);
    uStack_4 = 0xffffffff;
    Timer_Cleanup(auStack_50);
  }
  ExceptionList = pvStack_c;
  return;
}
