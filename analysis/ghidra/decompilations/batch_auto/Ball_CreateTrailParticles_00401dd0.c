/*
 * Function: Ball_CreateTrailParticles
 * Address: 0x00401dd0
 * Signature: Ball_CreateTrailParticles(...)
 *
 * Patterns: allocates, SEH frame, ball. Calls: Ball_CreateTrailParticles, Wave_Sin, Wave_Cos, operator_new, RumbleScore_ctor, RNG_Rand, AthenaList_Append. Offsets: 19, Lines: 86
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall Ball_CreateTrailParticles(int param_1)

{
  float *pfVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  float fVar5;
  float fVar6;
  float fVar7;
  float fVar8;
  float fVar9;
  float fVar10;
  float fVar11;
  void *this;
  int iVar12;
  void *pvVar13;
  undefined1 auVar14 [10];
  int local_50;
  float local_24;
  float local_20;
  float local_1c;
  float local_18 [3];
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004c922b;
  local_c = ExceptionList;
  local_50 = 0;
  ExceptionList = &local_c;
  do {
    iVar12 = *(int *)(*(int *)(*(int *)(*(int *)(param_1 + 0x14) + 0x878) + 0x174) + 0x744);
    fVar3 = *(float *)(iVar12 + 0x5c);
    fVar4 = *(float *)(iVar12 + 0x60);
    fVar5 = *(float *)(iVar12 + 100);
    fVar6 = *(float *)(iVar12 + 0x68);
    fVar7 = *(float *)(iVar12 + 0x6c);
    fVar8 = *(float *)(iVar12 + 0x70);
    auVar14 = Wave_Sin(&PTR_PTR_004f7188,(float)local_50);
    fVar2 = (float)(float10)auVar14;
    auVar14 = Wave_Cos(&PTR_PTR_004f7188,(float)local_50);
    fVar9 = *(float *)(param_1 + 0x284);
    fVar10 = fVar9 * fVar3 * fVar2;
    fVar11 = fVar9 * fVar4 * fVar2;
    fVar9 = fVar9 * fVar5 * fVar2;
    fVar2 = *(float *)(param_1 + 0x284);
    fVar6 = (float)((float10)fVar6 * (float10)auVar14) * fVar2;
    fVar7 = (float)((float10)fVar7 * (float10)auVar14) * fVar2;
    fVar2 = fVar2 * (float)((float10)fVar8 * (float10)auVar14);
    fVar3 = *(float *)(param_1 + 0x164);
    fVar4 = *(float *)(param_1 + 0x168);
    fVar5 = *(float *)(param_1 + 0x16c);
    this = operator_new(0x28);
    pvVar13 = (void *)0x0;
    local_4 = 0;
    if (this != (void *)0x0) {
      pvVar13 = RumbleScore_ctor(this,*(undefined4 *)(*(int *)(param_1 + 0x14) + 0x878));
    }
    local_4 = 0xffffffff;
    if ((float *)((int)pvVar13 + 8) != local_18) {
      *(float *)((int)pvVar13 + 8) = (fVar10 + fVar3) - fVar6;
      *(float *)((int)pvVar13 + 0xc) = (fVar11 + fVar4) - fVar7;
      *(float *)((int)pvVar13 + 0x10) = (fVar9 + fVar5) - fVar2;
    }
    pfVar1 = (float *)((int)pvVar13 + 0x14);
    local_20 = fVar11 - fVar7;
    local_1c = fVar9 - fVar2;
    if (pfVar1 != &local_24) {
      *pfVar1 = fVar10 - fVar6;
      *(float *)((int)pvVar13 + 0x18) = local_20;
      *(float *)((int)pvVar13 + 0x1c) = local_1c;
    }
    iVar12 = RNG_Rand(&PTR_OBJ_VTABLE,0x14,'\0');
    fVar2 = _DAT_004cf310 / (float)(iVar12 + 0x14);
    *pfVar1 = fVar2 * *pfVar1;
    *(float *)((int)pvVar13 + 0x18) = fVar2 * *(float *)((int)pvVar13 + 0x18);
    *(float *)((int)pvVar13 + 0x1c) = fVar2 * *(float *)((int)pvVar13 + 0x1c);
    AthenaList_Append((void *)(*(int *)(param_1 + 0x14) + 0x3b00),(int)pvVar13);
    local_50 = local_50 + 0x28;
  } while (local_50 < 0x168);
  ExceptionList = local_c;
  return;
}
