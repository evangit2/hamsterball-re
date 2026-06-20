
void __fastcall TourneyMenu_CreateBoard(int param_1)

{
  void *pvVar1;
  int *piVar2;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cad55;
  local_c = ExceptionList;
  switch(*(undefined4 *)(param_1 + 8)) {
  case 1:
    ExceptionList = &local_c;
    pvVar1 = operator_new(0x47e0);
    local_4 = 0;
    if (pvVar1 == (void *)0x0) {
LAB_00426a77:
      piVar2 = (int *)0x0;
    }
    else {
      piVar2 = RumbleBoard_Warmup_Ctor(pvVar1,*(int *)(param_1 + 4));
    }
    break;
  case 2:
    ExceptionList = &local_c;
    pvVar1 = operator_new(0x5850);
    local_4 = 1;
    if (pvVar1 == (void *)0x0) goto LAB_00426a77;
    piVar2 = RumbleBoard_Beginner_Ctor(pvVar1,*(int *)(param_1 + 4));
    break;
  case 3:
    ExceptionList = &local_c;
    pvVar1 = operator_new(0x47e0);
    local_4 = 2;
    if (pvVar1 == (void *)0x0) goto LAB_00426a77;
    piVar2 = RumbleBoard_Intermediate_Ctor(pvVar1,*(int *)(param_1 + 4));
    break;
  case 4:
    ExceptionList = &local_c;
    pvVar1 = operator_new(0x47e4);
    local_4 = 3;
    if (pvVar1 == (void *)0x0) goto LAB_00426a77;
    piVar2 = RumbleBoard_Dizzy_Ctor(pvVar1,*(int *)(param_1 + 4));
    break;
  case 5:
    ExceptionList = &local_c;
    pvVar1 = operator_new(0x501c);
    local_4 = 4;
    if (pvVar1 == (void *)0x0) goto LAB_00426a77;
    piVar2 = RumbleBoard_Tower_Ctor(pvVar1,*(int *)(param_1 + 4));
    break;
  case 6:
    ExceptionList = &local_c;
    pvVar1 = operator_new(0x47e4);
    local_4 = 5;
    if (pvVar1 == (void *)0x0) goto LAB_00426a77;
    piVar2 = RumbleBoard_UpArena_Ctor(pvVar1,*(int *)(param_1 + 4));
    break;
  case 7:
    ExceptionList = &local_c;
    pvVar1 = operator_new(0x47e8);
    local_4 = 6;
    if (pvVar1 == (void *)0x0) goto LAB_00426a77;
    piVar2 = RumbleBoard_NeonArena_ctor(pvVar1,*(int *)(param_1 + 4));
    break;
  case 8:
    ExceptionList = &local_c;
    pvVar1 = operator_new(0x4bfc);
    local_4 = 7;
    if (pvVar1 == (void *)0x0) goto LAB_00426a77;
    piVar2 = RumbleBoard_ExpertArena_ctor(pvVar1,*(int *)(param_1 + 4));
    break;
  case 9:
    ExceptionList = &local_c;
    pvVar1 = operator_new(0x47e0);
    local_4 = 8;
    if (pvVar1 == (void *)0x0) goto LAB_00426a77;
    piVar2 = RumbleBoard_OddArena_ctor(pvVar1,*(int *)(param_1 + 4));
    break;
  case 10:
    ExceptionList = &local_c;
    pvVar1 = operator_new(0x5c6c);
    local_4 = 9;
    if (pvVar1 == (void *)0x0) goto LAB_00426a77;
    piVar2 = RumbleBoard_ToobArena_ctor(pvVar1,*(int *)(param_1 + 4));
    break;
  case 0xb:
    ExceptionList = &local_c;
    pvVar1 = operator_new(0x47e4);
    local_4 = 10;
    if (pvVar1 == (void *)0x0) goto LAB_00426a77;
    piVar2 = RumbleBoard_WobblyArena_ctor(pvVar1,*(int *)(param_1 + 4));
    break;
  case 0xc:
    ExceptionList = &local_c;
    pvVar1 = operator_new(0x47e0);
    local_4 = 0xb;
    if (pvVar1 == (void *)0x0) goto LAB_00426a77;
    piVar2 = BoardLevel_Glass_ctor(pvVar1,*(int *)(param_1 + 4));
    break;
  case 0xd:
    ExceptionList = &local_c;
    pvVar1 = operator_new(0x4cfc);
    local_4 = 0xc;
    if (pvVar1 == (void *)0x0) goto LAB_00426a77;
    piVar2 = RumbleBoard_SkyArena_ctor(pvVar1,*(int *)(param_1 + 4));
    break;
  case 0xe:
    ExceptionList = &local_c;
    pvVar1 = operator_new(0x47e0);
    local_4 = 0xd;
    if (pvVar1 == (void *)0x0) goto LAB_00426a77;
    piVar2 = RumbleBoard_WarmupArena_ctor(pvVar1,*(int *)(param_1 + 4));
    break;
  case 0xf:
    ExceptionList = &local_c;
    pvVar1 = operator_new(0x47e4);
    local_4 = 0xe;
    if (pvVar1 == (void *)0x0) goto LAB_00426a77;
    piVar2 = RumbleBoard_Impossible_ctor(pvVar1,*(int *)(param_1 + 4));
    break;
  default:
    goto switchD_004267a6_default;
  }
  *(int **)(param_1 + 0xc) = piVar2;
  local_4 = 0xffffffff;
  (**(code **)(*piVar2 + 0x48))();
  Scene_AddObject(*(void **)(*(int *)(param_1 + 4) + 0x184),*(int **)(param_1 + 0xc));
switchD_004267a6_default:
  ExceptionList = local_c;
  return;
}

