
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void * __thiscall OkayDialog_ctor(void *this,int param_1,char *param_2,byte *param_3,int param_4)

{
  char cVar1;
  byte bVar2;
  float fVar3;
  float fVar4;
  char *pcVar5;
  void *pvVar6;
  byte *pbVar7;
  int iVar8;
  byte *pbVar9;
  ulonglong uVar10;
  float local_434;
  float local_430;
  int local_42c;
  int local_428;
  void *local_424;
  float local_414;
  float local_410;
  byte local_40c [1024];
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cbdcc;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  local_424 = this;
  Gadget_ctor(this,param_1);
  local_4 = 0;
  *(undefined ***)this = &PTR_SceneObject_DeletingDtor_004d5de0;
  AthenaString_Init((undefined4 *)((int)this + 0x888));
  *(undefined1 *)((int)this + 0x8a4) = 1;
  local_4 = CONCAT31(local_4._1_3_,1);
  *(char **)((int)this + 0x868) = "Okay Dialog";
  *(int *)((int)this + 0x878) = param_1;
  Font_WordWrap(*(void **)(param_1 + 0x318),param_3,param_4,local_40c);
  pcVar5 = param_2;
  do {
    cVar1 = *pcVar5;
    pcVar5 = pcVar5 + 1;
  } while (cVar1 != '\0');
  pvVar6 = operator_new((uint)(pcVar5 + (1 - (int)(param_2 + 1))));
  *(void **)((int)this + 0x880) = pvVar6;
  pbVar7 = local_40c;
  do {
    bVar2 = *pbVar7;
    pbVar7 = pbVar7 + 1;
  } while (bVar2 != 0);
  pvVar6 = operator_new((uint)(pbVar7 + (1 - (int)(local_40c + 1))));
  pcVar5 = *(char **)((int)this + 0x880);
  *(void **)((int)this + 0x884) = pvVar6;
  do {
    cVar1 = *param_2;
    param_2 = param_2 + 1;
    *pcVar5 = cVar1;
    pcVar5 = pcVar5 + 1;
  } while (cVar1 != '\0');
  pbVar9 = *(byte **)((int)this + 0x884);
  pbVar7 = local_40c;
  do {
    bVar2 = *pbVar7;
    pbVar7 = pbVar7 + 1;
    *pbVar9 = bVar2;
    pbVar9 = pbVar9 + 1;
  } while (bVar2 != 0);
  UI_MeasureTextExtent
            (*(void **)(*(int *)((int)this + 0x878) + 0x318),&local_434,
             *(byte **)((int)this + 0x884));
  fVar3 = _DAT_004cf3f0;
  local_434 = local_434 + _DAT_004cf370 + _DAT_004cf484;
  local_430 = (float)*(int *)(*(int *)(*(int *)((int)this + 0x878) + 0x318) + 0x424) +
              (float)*(int *)(*(int *)(*(int *)((int)this + 0x878) + 0x318) + 0x424) + local_430 +
              _DAT_004cf528 + _DAT_004cf484 + _DAT_004d5dc4;
  *(float *)((int)this + 0x10) = local_430;
  *(float *)((int)this + 4) = _DAT_004cff7c - fVar3 * local_434;
  *(float *)((int)this + 8) = _DAT_004d041c - local_430 * _DAT_004cf3f0;
  *(float *)((int)this + 0xc) = local_434;
  uVar10 = Font_MeasureText("OKAY!");
  fVar3 = *(float *)((int)this + 0xc) * _DAT_004cf3f0;
  iVar8 = (int)uVar10 + 0x28;
  local_428 = *(int *)(*(int *)(*(int *)((int)this + 0x878) + 0x318) + 0x424);
  local_42c = local_428 + 10;
  fVar4 = *(float *)((int)this + 0x10) - _DAT_004cf528;
  local_414 = (float)iVar8;
  local_410 = (float)local_42c;
  *(float *)((int)this + 0x894) = local_414;
  *(float *)((int)this + 0x88c) = fVar3 - (float)(iVar8 / 2);
  *(float *)((int)this + 0x898) = local_410;
  *(float *)((int)this + 0x890) = fVar4 - (float)local_428;
  AthenaList_Append((void *)((int)this + 0x44c),(int)this + 0x888);
  ExceptionList = local_c;
  return this;
}

