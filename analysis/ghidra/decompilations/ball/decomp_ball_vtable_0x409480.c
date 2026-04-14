
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_00409480(int param_1)

{
  float *pfVar1;
  float fVar2;
  int iVar3;
  float fVar4;
  float fVar5;
  float fVar6;
  float fVar7;
  float fVar8;
  float fVar9;
  float fVar10;
  float fVar11;
  float fVar12;
  void *pvVar13;
  int *this;
  undefined4 *puVar14;
  int iVar15;
  undefined1 auVar16 [10];
  int iStack_50;
  float fStack_24;
  float fStack_20;
  float fStack_1c;
  float afStack_18 [3];
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 uStack_4;
  
  uStack_4 = 0xffffffff;
  puStack_8 = &LAB_004c94c6;
  pvStack_c = ExceptionList;
  if (*(char *)(param_1 + 0x324) == '\0') {
    ExceptionList = &pvStack_c;
    if (*(void **)(param_1 + 0xc28) != (void *)0x0) {
      ExceptionList = &pvStack_c;
      _free(*(void **)(param_1 + 0xc28));
      *(undefined4 *)(param_1 + 0xc28) = 0;
    }
    *(undefined1 *)(param_1 + 0x2e8) = 1;
    if (*(char *)(param_1 + 0xc4c) == '\0') {
      iVar15 = *(int *)(*(int *)(param_1 + 0x10) + 0x44c);
    }
    else {
      iVar15 = *(int *)(*(int *)(param_1 + 0x10) + 0x450);
    }
    FUN_004597b0(iVar15);
    iStack_50 = 0;
    do {
      pvVar13 = operator_new(0xc64);
      uStack_4 = 0;
      if (pvVar13 == (void *)0x0) {
        this = (int *)0x0;
      }
      else {
        this = FUN_00408d10(pvVar13,*(int *)(param_1 + 0x14));
      }
      uStack_4 = 0xffffffff;
      *(undefined1 *)(this + 0xc9) = 1;
      (**(code **)(*this + 4))();
      iVar15 = *(int *)(param_1 + 0x1a4);
      iVar3 = this[0x69];
      if ((undefined4 *)(iVar3 + 0xca4) != (undefined4 *)(iVar15 + 0xca4)) {
        *(undefined4 *)(iVar3 + 0xca4) = *(undefined4 *)(iVar15 + 0xca4);
        *(undefined4 *)(iVar3 + 0xca8) = *(undefined4 *)(iVar15 + 0xca8);
        *(undefined4 *)(iVar3 + 0xcac) = *(undefined4 *)(iVar15 + 0xcac);
      }
      this[0x59] = *(int *)(param_1 + 0x164);
      this[0x5a] = *(int *)(param_1 + 0x168);
      this[0x5b] = *(int *)(param_1 + 0x16c);
      FUN_00403850(this,&PTR_FUN_004cf300,*(float *)(param_1 + 0x2ac),*(float *)(param_1 + 0x2b0),
                   *(float *)(param_1 + 0x2b4),*(float *)(param_1 + 0x2b8));
      this[6] = -1;
      this[0x9e] = 0x3c23d70a;
      this[0x9f] = 0x3f000000;
      this[0xa1] = *(int *)(param_1 + 0x284);
      if (*(int *)(param_1 + 0x748) == 0) {
        iVar15 = this[0x69];
        this[0x1d2] = 0;
        if ((float *)(iVar15 + 0xc8c) != &fStack_24) {
          *(float *)(iVar15 + 0xc8c) = 0.0;
          *(undefined4 *)(iVar15 + 0xc90) = 0xbf800000;
          *(undefined4 *)(iVar15 + 0xc94) = 0;
        }
        (**(code **)(this[7] + 4))();
        this[0x3e] = this[0x1d2];
      }
      if (*(int *)(param_1 + 0x748) == 1) {
        this[0x1d2] = 1;
        iVar15 = this[0x69];
        if ((float *)(iVar15 + 0xc8c) != &fStack_24) {
          *(float *)(iVar15 + 0xc8c) = -1.0;
          *(undefined4 *)(iVar15 + 0xc90) = 0;
          *(undefined4 *)(iVar15 + 0xc94) = 0;
        }
        (**(code **)(this[7] + 4))();
        this[0x3e] = this[0x1d2];
      }
      if (*(int *)(param_1 + 0x748) == 2) {
        iVar15 = this[0x69];
        this[0x1d2] = 2;
        if ((float *)(iVar15 + 0xc8c) != &fStack_24) {
          *(float *)(iVar15 + 0xc8c) = 0.0;
          *(undefined4 *)(iVar15 + 0xc90) = 0;
          *(undefined4 *)(iVar15 + 0xc94) = 0x3f800000;
        }
        (**(code **)(this[7] + 4))();
        this[0x3e] = this[0x1d2];
      }
      if (*(int *)(param_1 + 0x748) == 0) {
        (**(code **)(*this + 0x14))();
      }
      if (*(int *)(param_1 + 0x748) == 1) {
        (**(code **)(*this + 0x14))();
      }
      if (*(int *)(param_1 + 0x748) == 2) {
        (**(code **)(*this + 0x14))();
      }
      iVar15 = iStack_50 + 1;
      this[0x1d5] = iVar15;
      thunk_FUN_00453780((void *)(*(int *)(param_1 + 0x14) + 0x3204),(int)this);
      *(undefined1 *)((int)this + 0x31d) = 1;
      this[0xc6] = 0x41200000;
      if (*(int *)(param_1 + 0x748) == 0) {
        iVar3 = *this;
        FUN_0045dd60(&PTR_PTR_004f7360,5,'\x01');
        FUN_0045dd60(&PTR_PTR_004f7360,10,'\0');
        FUN_0045dd60(&PTR_PTR_004f7360,5,'\x01');
        (**(code **)(iVar3 + 0x14))();
      }
      if (*(int *)(param_1 + 0x748) == 1) {
        iVar3 = *this;
        FUN_0045dd60(&PTR_PTR_004f7360,5,'\x01');
        FUN_0045dd60(&PTR_PTR_004f7360,5,'\x01');
        FUN_0045dd60(&PTR_PTR_004f7360,10,'\0');
        (**(code **)(iVar3 + 0x14))();
      }
      if (*(int *)(param_1 + 0x748) == 2) {
        iVar3 = *this;
        FUN_0045dd60(&PTR_PTR_004f7360,10,'\0');
        FUN_0045dd60(&PTR_PTR_004f7360,5,'\x01');
        FUN_0045dd60(&PTR_PTR_004f7360,5,'\x01');
        (**(code **)(iVar3 + 0x14))();
      }
      if (iStack_50 == 0) {
        this[0xca] = 2;
      }
      else if (iStack_50 == 1) {
        this[0xca] = 4;
      }
      iStack_50 = iVar15;
    } while (iVar15 < 2);
    iStack_50 = 0;
    do {
      puVar14 = (undefined4 *)0x0;
      iVar15 = *(int *)(*(int *)(*(int *)(*(int *)(param_1 + 0x14) + 0x878) + 0x174) + 0x744);
      fVar4 = *(float *)(iVar15 + 0x5c);
      fVar5 = *(float *)(iVar15 + 0x60);
      fVar6 = *(float *)(iVar15 + 100);
      fVar7 = *(float *)(iVar15 + 0x68);
      fVar8 = *(float *)(iVar15 + 0x6c);
      fVar9 = *(float *)(iVar15 + 0x70);
      auVar16 = FUN_00457da0(0x4f7188,(float)iStack_50);
      fVar2 = (float)(float10)auVar16;
      auVar16 = FUN_00457dc0(0x4f7188,(float)iStack_50);
      fVar10 = *(float *)(param_1 + 0x284);
      fVar11 = fVar10 * fVar4 * fVar2;
      fVar12 = fVar10 * fVar5 * fVar2;
      fVar10 = fVar10 * fVar6 * fVar2;
      fVar2 = *(float *)(param_1 + 0x284);
      fVar7 = (float)((float10)fVar7 * (float10)auVar16) * fVar2;
      fVar8 = (float)((float10)fVar8 * (float10)auVar16) * fVar2;
      fVar2 = fVar2 * (float)((float10)fVar9 * (float10)auVar16);
      fVar4 = *(float *)(param_1 + 0x164);
      fVar5 = *(float *)(param_1 + 0x168);
      fVar6 = *(float *)(param_1 + 0x16c);
      pvVar13 = operator_new(0x28);
      uStack_4 = 1;
      if (pvVar13 != (void *)0x0) {
        puVar14 = FUN_0044ad50(pvVar13,*(undefined4 *)(*(int *)(param_1 + 0x14) + 0x878));
      }
      uStack_4 = 0xffffffff;
      if ((float *)(puVar14 + 2) != afStack_18) {
        puVar14[2] = (fVar11 + fVar4) - fVar7;
        puVar14[3] = (fVar12 + fVar5) - fVar8;
        puVar14[4] = (fVar10 + fVar6) - fVar2;
      }
      pfVar1 = (float *)(puVar14 + 5);
      fStack_20 = fVar12 - fVar8;
      fStack_1c = fVar10 - fVar2;
      if (pfVar1 != &fStack_24) {
        *pfVar1 = fVar11 - fVar7;
        puVar14[6] = fStack_20;
        puVar14[7] = fStack_1c;
      }
      iVar15 = FUN_0045dd60(&PTR_PTR_004f7360,0x14,'\0');
      fVar2 = _DAT_004cf310 / (float)(iVar15 + 0x14);
      *pfVar1 = fVar2 * *pfVar1;
      puVar14[6] = fVar2 * (float)puVar14[6];
      puVar14[7] = fVar2 * (float)puVar14[7];
      thunk_FUN_00453780((void *)(*(int *)(param_1 + 0x14) + 0x3b00),(int)puVar14);
      iStack_50 = iStack_50 + 0x28;
    } while (iStack_50 < 0x168);
  }
  ExceptionList = pvStack_c;
  return;
}
