
/* UIList_AddItem: Alloc UIListItem(0x444B),copies display_text+id_code,creates AthenaString,appends
   to lists at +0x44c/+0x88c,calculates height/width. See decompilations/ui/decomp_menu_system.c */

void __thiscall
UIList_AddItem(void *this,char *param_1,char *param_2,undefined4 param_3,undefined4 param_4,
              undefined4 param_5,undefined4 param_6,undefined4 param_7,int param_8)

{
  char cVar1;
  int iVar2;
  void *pvVar3;
  undefined4 *puVar4;
  char *pcVar5;
  char *pcVar6;
  undefined4 *puVar7;
  int iVar8;
  int iVar9;
  int iVar10;
  ulonglong uVar11;
  void *local_c;
  undefined1 *puStack_8;
  int local_4;
  
  puStack_8 = &LAB_004cc43e;
  local_c = ExceptionList;
  local_4 = 0;
  ExceptionList = &local_c;
  pvVar3 = operator_new(0x444);
  local_4._0_1_ = 1;
  if (pvVar3 == (void *)0x0) {
    puVar4 = (undefined4 *)0x0;
  }
  else {
    puVar4 = (undefined4 *)UIListItem_ctor((int)pvVar3);
  }
  local_4._0_1_ = 0;
  *(undefined1 *)((int)puVar4 + 0x441) = 0;
  *(undefined1 *)(puVar4 + 0x110) = 0;
  pcVar5 = param_1;
  do {
    cVar1 = *pcVar5;
    pcVar5 = pcVar5 + 1;
  } while (cVar1 != '\0');
  pcVar6 = operator_new((uint)(pcVar5 + (1 - (int)(param_1 + 1))));
  *puVar4 = pcVar6;
  pcVar5 = param_1;
  do {
    cVar1 = *pcVar5;
    pcVar5 = pcVar5 + 1;
    *pcVar6 = cVar1;
    pcVar6 = pcVar6 + 1;
  } while (cVar1 != '\0');
  pcVar5 = param_2;
  do {
    cVar1 = *pcVar5;
    pcVar5 = pcVar5 + 1;
  } while (cVar1 != '\0');
  pvVar3 = operator_new((uint)(pcVar5 + (1 - (int)(param_2 + 1))));
  puVar4[1] = pvVar3;
  iVar9 = (int)pvVar3 - (int)param_2;
  do {
    cVar1 = *param_2;
    param_2[iVar9] = cVar1;
    param_2 = param_2 + 1;
  } while (cVar1 != '\0');
  puVar4[3] = param_4;
  puVar4[4] = param_5;
  puVar4[5] = param_6;
  puVar4[6] = param_7;
  puVar7 = operator_new(0x1c);
  local_4._0_1_ = 2;
  if (puVar7 == (undefined4 *)0x0) {
    iVar9 = 0;
  }
  else {
    iVar9 = AthenaString_Init(puVar7);
  }
  iVar2 = param_8;
  local_4 = (uint)local_4._1_3_ << 8;
  puVar4[7] = iVar9;
  puVar4[8] = param_8;
  AthenaList_Append((void *)((int)this + 0x44c),iVar9);
  AthenaList_Append((void *)((int)this + 0x88c),(int)puVar4);
  uVar11 = __ftol2();
  iVar9 = (int)uVar11;
  puVar4[9] = iVar9;
  if (iVar2 != 0) {
    if (iVar9 <= *(int *)(iVar2 + 0xcc)) {
      iVar9 = *(int *)(iVar2 + 0xcc);
    }
    puVar4[9] = iVar9;
  }
  *(int *)((int)this + 0xcb4) = *(int *)((int)this + 0xcb4) + puVar4[9];
  uVar11 = Font_MeasureText(param_1);
  iVar9 = (int)uVar11 + 2;
  if (iVar2 != 0) {
    iVar9 = (int)uVar11 + 7 + *(int *)(iVar2 + 200);
  }
  iVar2 = *(int *)((int)this + 0xcb0);
  iVar10 = iVar2;
  if (iVar2 <= iVar9) {
    iVar10 = iVar9;
  }
  iVar8 = *(int *)((int)this + 0xcac);
  if ((iVar8 <= iVar10) && (iVar8 = iVar9, iVar9 < iVar2)) {
    iVar8 = iVar2;
  }
  *(int *)((int)this + 0xcb0) = iVar8;
  *(undefined1 *)((int)this + 0xcbc) = 1;
  local_4 = 0xffffffff;
  Matrix_Identity(&param_3);
  ExceptionList = local_c;
  return;
}

