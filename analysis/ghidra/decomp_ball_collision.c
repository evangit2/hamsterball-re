
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall Ball_CollisionCheck(int param_1)

{
  int *piVar1;
  void *this;
  short sVar2;
  int iVar3;
  undefined4 *puVar4;
  undefined4 *puVar5;
  undefined **ppuStack_94;
  undefined4 auStack_90 [16];
  undefined4 local_50 [17];
  void *local_c;
  undefined1 *puStack_8;
  int local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004c9273;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  Timer_Init(local_50);
  local_4 = 0;
  Graphics_BeginFrame(*(void **)(*(int *)(param_1 + 0x10) + 0x174),(int)local_50);
  iVar3 = *(int *)(param_1 + 0x10);
  this = *(void **)(iVar3 + 0x174);
  sVar2 = Ball_CheckCollisionPlanes
                    (*(void **)((int)this + 0x748),*(float *)(param_1 + 0x164),
                     *(float *)(param_1 + 0x168),*(float *)(param_1 + 0x16c),200.0);
  if ((char)sVar2 == '\0') {
    Graphics_BeginFrame(this,0);
    piVar1 = (int *)(param_1 + 0xc74);
    *piVar1 = *piVar1 + -1;
    if (*piVar1 < 0) {
      *(undefined4 *)(param_1 + 0xc74) = 0;
    }
  }
  else {
    *(uint *)(param_1 + 0xc74) = *(byte *)(iVar3 + 0x234) + 1;
    if (*(int *)(param_1 + 0xc38) != *(int *)(*(int *)(iVar3 + 0x174) + 0x7c4)) {
      (**(code **)(*(int *)(param_1 + 0x1c) + 0x34))
                (*(undefined4 *)(param_1 + 0x164),*(undefined4 *)(param_1 + 0x168),
                 *(undefined4 *)(param_1 + 0x16c),0);
      (**(code **)(*(int *)(param_1 + 0x1c) + 0x38))(*(float *)(param_1 + 0x284) * _DAT_004cf39c);
      (**(code **)(*(int *)(param_1 + 0x1c) + 0x30))();
    }
    iVar3 = *(int *)(*(int *)(param_1 + 0x10) + 0x174);
    if (*(char *)(iVar3 + 0x70c) != '\0') {
      (**(code **)(**(int **)(iVar3 + 0x154) + 200))(*(int **)(iVar3 + 0x154),0xe,0);
      *(undefined1 *)(iVar3 + 0x70c) = 0;
      *(int *)(iVar3 + 0x7c8) = *(int *)(iVar3 + 0x7c8) + 1;
    }
    if (*(char *)(param_1 + 0xc80) != '\0') {
      FUN_00401160(*(void **)(*(int *)(param_1 + 0x10) + 0x174),*(undefined4 *)(param_1 + 0xc88),
                   *(undefined4 *)(param_1 + 0xc8c),*(undefined4 *)(param_1 + 0xc90),0x3f800000);
      FUN_00401100(*(void **)(*(int *)(param_1 + 0x10) + 0x174));
    }
    ppuStack_94 = &PTR_FUN_004cf338;
    puVar4 = (undefined4 *)(param_1 + 0x20);
    puVar5 = auStack_90;
    for (iVar3 = 0x10; iVar3 != 0; iVar3 = iVar3 + -1) {
      *puVar5 = *puVar4;
      puVar4 = puVar4 + 1;
      puVar5 = puVar5 + 1;
    }
    local_4._0_1_ = 1;
    (**(code **)(**(int **)(*(int *)(param_1 + 0x10) + 0x268) + 0x1c))(&ppuStack_94,0);
    local_4 = (uint)local_4._1_3_ << 8;
    Timer_Cleanup(&ppuStack_94);
    if (*(char *)(param_1 + 0x324) == '\0') {
      FUN_0045d660(*(void **)(*(int *)(param_1 + 0x10) + 0x33c),*(float *)(param_1 + 0x164),
                   *(float *)(param_1 + 0x168),*(float *)(param_1 + 0x16c),
                   *(float *)(param_1 + 0x284) * _DAT_004cf398 * (float)_DAT_004cf390,
                   (undefined4 *)(param_1 + 0x1b8));
    }
    if (*(char *)(param_1 + 0xc80) != '\0') {
      FUN_00401100(*(void **)(*(int *)(param_1 + 0x10) + 0x174));
      *(undefined1 *)(*(int *)(*(int *)(param_1 + 0x10) + 0x174) + 0x7a8) = 0;
    }
    iVar3 = *(int *)(*(int *)(param_1 + 0x10) + 0x174);
    if (*(char *)(iVar3 + 0x70c) != '\x01') {
      (**(code **)(**(int **)(iVar3 + 0x154) + 200))(*(int **)(iVar3 + 0x154),0xe,1);
      *(undefined1 *)(iVar3 + 0x70c) = 1;
      *(int *)(iVar3 + 0x7c8) = *(int *)(iVar3 + 0x7c8) + 1;
    }
    if (*(int *)(param_1 + 0x2fc) != 0x3f800000) {
      *(undefined1 *)(*(int *)(*(int *)(param_1 + 0x10) + 0x174) + 0x7a8) = 0;
    }
    *(undefined4 *)(param_1 + 0xc38) =
         *(undefined4 *)(*(int *)(*(int *)(param_1 + 0x10) + 0x174) + 0x7c4);
  }
  local_4 = 0xffffffff;
  Timer_Cleanup(local_50);
  ExceptionList = local_c;
  return;
}

