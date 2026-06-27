// Decompiled via GhidraMCP
// Function: Board_ctor
// Address: 0x00419030
// Category: scene

/* Board_ctor: Base board constructor. Size~0x4368. Calls Gadget_ctor, inits
   RumbleTimers@+0x884/+0x898, 13 AthenaLists, Ball@+0x361C via Ball_ctor2, physics params@+0x29B0,
   player ball colors@+0x3AAC (4 RGB entries). See decompilations/scene/decomp_object_factory.c */

void * __thiscall Board_ctor(void *this,int param_1)

{
  int iVar1;
  char cVar2;
  void *pvVar3;
  Ball *pBVar4;
  int iVar5;
  void *pvVar6;
  float fVar7;
  float fVar8;
  float fVar9;
  float fVar10;
  undefined1 *local_c84;
  undefined4 local_c80;
  void *local_c70;
  undefined4 local_c6c [2];
  undefined1 local_c64 [3160];
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004c9ea8;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  local_c70 = this;
  Gadget_ctor(this,param_1);
  local_4 = 0;
  *(undefined ***)this = &PTR_Scene_DeletingDtor_004d0260;
  RumbleBoard_InitTimer((undefined4 *)((int)this + 0x884));
  local_4._0_1_ = 1;
  RumbleBoard_InitTimer((undefined4 *)((int)this + 0x898));
  local_4._0_1_ = 2;
  AthenaList_Init((void *)((int)this + 0x8b8),0);
  local_4._0_1_ = 3;
  AthenaList_Init((void *)((int)this + 0xcd4),0);
  local_4._0_1_ = 4;
  AthenaList_Init((void *)((int)this + 0x10ec),0);
  local_4._0_1_ = 5;
  Vec3_Init((undefined4 *)((int)this + 0x1504));
  local_4._0_1_ = 6;
  AthenaList_Init((void *)((int)this + 0x1518),0);
  local_4._0_1_ = 7;
  AthenaList_Init((void *)((int)this + 0x1930),0);
  local_4._0_1_ = 8;
  AthenaList_Init((void *)((int)this + 0x1d48),0);
  local_4._0_1_ = 9;
  AthenaList_Init((void *)((int)this + 0x2160),0);
  local_4._0_1_ = 10;
  AthenaList_Init((void *)((int)this + 0x2578),0);
  local_4._0_1_ = 0xb;
  AthenaList_Init((void *)((int)this + 0x29d4),0);
  local_4._0_1_ = 0xc;
  AthenaList_Init((void *)((int)this + 0x2dec),0);
  local_4._0_1_ = 0xd;
  AthenaList_Init((void *)((int)this + 0x3204),0);
  local_4._0_1_ = 0xe;
  AthenaList_Init((void *)((int)this + 0x362c),0);
  local_4._0_1_ = 0xf;
  _eh_vector_constructor_iterator_((void *)((int)this + 0x3aac),0x14,4,Vec3_Init,Matrix_Identity);
  local_4._0_1_ = 0x10;
  SceneObject_EmptyListCtor((undefined4 *)((int)this + 0x3afc));
  pvVar6 = (void *)((int)this + 0x3f20);
  local_4._0_1_ = 0x11;
  _eh_vector_constructor_iterator_(pvVar6,0x10,2,FUN_00409d40,NoOp);
  *(int *)((int)this + 0x878) = param_1;
  *(undefined4 *)((int)this + 0x299c) = 0xbf800000;
  *(undefined1 *)((int)this + 0x880) = 0;
  *(undefined4 *)((int)this + 0x4358) = 0;
  local_4 = CONCAT31(local_4._1_3_,0x12);
  if (5 < *(int *)(*(int *)(param_1 + 0x220) + 8)) {
    cVar2 = Game_SetInProgress(param_1);
    if (cVar2 == '\0') {
      *(undefined4 *)((int)this + 0x4358) = 1;
      *(undefined4 *)((int)this + 0x435c) = 0x9c4;
    }
  }
  *(undefined1 *)((int)this + 0x4368) = 0;
  if ((undefined1 **)((int)this + 0x434c) != &local_c84) {
    *(undefined1 **)((int)this + 0x434c) = (undefined1 *)0x0;
    *(undefined4 *)((int)this + 0x4350) = 0xc3160000;
    *(undefined4 *)((int)this + 0x4354) = 0;
  }
  *(undefined1 *)((int)this + 0x29b0) = 1;
  *(undefined4 *)((int)this + 0x8b4) = 0;
  *(undefined4 *)((int)this + 0x29b4) = 0;
  *(undefined1 *)((int)this + 0xcd0) = 0;
  *(undefined4 *)((int)this + 0x8ac) = 0;
  *(undefined4 *)((int)this + 0x87c) = 0;
  *(undefined1 *)((int)this + 0x3a44) = 0;
  *(undefined4 *)((int)this + 0x29bc) = 0x43070000;
  *(undefined4 *)((int)this + 0x29c0) = 0x44898000;
  *(undefined4 *)((int)this + 0x3620) = 0;
  *(undefined4 *)((int)this + 0x3624) = 0x3f800000;
  *(undefined4 *)((int)this + 0x88c) = 0x19;
  *(undefined4 *)((int)this + 0x8a0) = 0x32;
  *(char **)((int)this + 0x868) = "Board";
  *(undefined4 *)((int)this + 4) = 0;
  *(undefined4 *)((int)this + 8) = 0;
  *(undefined4 *)((int)this + 0xc) = 0x44480000;
  *(undefined4 *)((int)this + 0x10) = 0x44160000;
  *(undefined4 *)((int)this + 0x3628) = 0;
  AthenaList_Free((int)this + 0x362c);
  iVar5 = 0;
  do {
    *(undefined1 *)(*(int *)((int)this + 0x878) + 0x5d6 + iVar5) = 0;
    *(undefined1 *)(*(int *)((int)this + 0x878) + 0x5f0 + iVar5) = 0;
    iVar1 = *(int *)(*(int *)((int)this + 0x878) + 0x220);
    if ((*(char *)(iVar1 + 0x11) != '\0') || (*(int *)(iVar1 + 8) == 1)) {
      *(undefined4 *)(*(int *)((int)this + 0x878) + 0x5f4 + iVar5) = 0;
    }
    *(undefined4 *)(*(int *)((int)this + 0x878) + 0x5f8 + iVar5) = 0;
    if (*(char *)(*(int *)((int)this + 0x878) + 0x5d7 + iVar5) == '\0') {
      AthenaList_Append((void *)((int)this + 0x362c),*(int *)((int)this + 0x878) + iVar5 + 0x5cc);
      *(undefined4 *)(*(int *)((int)this + 0x878) + 0x5fc + iVar5) = 0;
      *(int *)((int)this + 0x3628) = *(int *)((int)this + 0x3628) + 1;
    }
    iVar5 = iVar5 + 0xa0;
  } while (iVar5 < 0x280);
  local_c84 = operator_new(0x418);
  local_4._0_1_ = 0x13;
  if (local_c84 == (undefined1 *)0x0) {
    pvVar3 = (void *)0x0;
  }
  else {
    pvVar3 = AthenaList_Init(local_c84,2);
  }
  local_4._0_1_ = 0x12;
  *(void **)((int)this + 0x3a48) = pvVar3;
  Ball_ctor2(local_c6c,(int)this);
  local_4._0_1_ = 0x14;
  IndexList_SetRange(*(void **)((int)this + 0x3a48),(int)local_c6c,(int)local_c64);
  *(undefined4 *)((int)this + 0x29d0) = 0;
  *(undefined4 *)((int)this + 0x3a50) = 0;
  *(undefined4 *)((int)this + 0x3a54) = 0;
  *(undefined1 *)((int)this + 0x3a58) = 1;
  *(undefined1 *)((int)this + 0x3a4c) = 0;
  if (*(char *)(*(int *)(*(int *)((int)this + 0x878) + 0x220) + 0x10) != '\0') {
    *(undefined1 *)((int)this + 0x3a4c) = 1;
  }
  *(undefined4 *)((int)this + 0x3a8c) = 0x42480000;
  *(undefined4 *)((int)this + 0x3a90) = 0x42480000;
  *(undefined4 *)((int)this + 0x3a94) = 0x443b8000;
  *(undefined4 *)((int)this + 0x3a9c) = 0x42480000;
  local_c80 = 0x44098000;
  *(undefined4 *)((int)this + 15000) = 0x42480000;
  local_c84 = (undefined1 *)0x443b8000;
  *(undefined4 *)((int)this + 0x3aa8) = 0x44098000;
  *(undefined4 *)((int)this + 0x29b8) = 800;
  *(undefined4 *)((int)this + 0x3aa0) = 0x44098000;
  *(undefined4 *)((int)this + 0x3aa4) = 0x443b8000;
  iVar5 = Vec3_Init(&local_c84,0x3f800000,0x3f800000,0x3f800000);
  *(undefined4 *)((int)this + 0x3ab0) = *(undefined4 *)(iVar5 + 4);
  *(undefined4 *)((int)this + 0x3ab4) = *(undefined4 *)(iVar5 + 8);
  *(undefined4 *)((int)this + 0x3ab8) = *(undefined4 *)(iVar5 + 0xc);
  *(undefined4 *)((int)this + 0x3abc) = *(undefined4 *)(iVar5 + 0x10);
  Matrix_Identity(&local_c84);
  iVar5 = Vec3_Init(&local_c84,0,0x3f000000,0x3f800000);
  *(undefined4 *)((int)this + 0x3ac4) = *(undefined4 *)(iVar5 + 4);
  *(undefined4 *)((int)this + 0x3ac8) = *(undefined4 *)(iVar5 + 8);
  *(undefined4 *)((int)this + 0x3acc) = *(undefined4 *)(iVar5 + 0xc);
  *(undefined4 *)((int)this + 0x3ad0) = *(undefined4 *)(iVar5 + 0x10);
  Matrix_Identity(&local_c84);
  iVar5 = Vec3_Init(&local_c84,0x3f800000,0x3e800000,0x3e800000);
  *(undefined4 *)((int)this + 0x3ad8) = *(undefined4 *)(iVar5 + 4);
  *(undefined4 *)((int)this + 0x3adc) = *(undefined4 *)(iVar5 + 8);
  *(undefined4 *)((int)this + 0x3ae0) = *(undefined4 *)(iVar5 + 0xc);
  *(undefined4 *)((int)this + 0x3ae4) = *(undefined4 *)(iVar5 + 0x10);
  Matrix_Identity(&local_c84);
  fVar8 = 1.0;
  fVar7 = 6.022939e-39;
  iVar5 = Vec3_Init(&local_c84,0x3f800000,0x3f800000,0);
  *(undefined4 *)((int)this + 0x3aec) = *(undefined4 *)(iVar5 + 4);
  *(undefined4 *)((int)this + 0x3af0) = *(undefined4 *)(iVar5 + 8);
  *(undefined4 *)((int)this + 0x3af4) = *(undefined4 *)(iVar5 + 0xc);
  *(undefined4 *)((int)this + 0x3af8) = *(undefined4 *)(iVar5 + 0x10);
  Matrix_Identity(&local_c84);
  iVar5 = *(int *)(*(int *)((int)this + 0x878) + 0x178);
  *(undefined4 *)(iVar5 + 0x914) = 0x43fa0000;
  *(undefined4 *)(iVar5 + 0x918) = 0x44bb8000;
  *(undefined4 *)((int)this + 0x3f18) = 0;
  NoOp_return();
  iVar5 = *(int *)((int)this + 0x878);
  *(undefined1 *)((int)this + 0x3f40) = 0;
  *(undefined4 *)((int)this + 0x2990) = 100;
  *(undefined4 *)((int)this + 0x4340) = 0x3f800000;
  *(undefined1 *)((int)this + 0x4348) = 0;
  *(undefined4 *)((int)this + 0x361c) = 0;
  if (((*(char *)(*(int *)(iVar5 + 0x220) + 0x11) != '\0') && (*(char *)(iVar5 + 0x234) == '\0')) &&
     (*(int *)(iVar5 + 0x910) != 0)) {
    *(undefined4 *)(*(int *)(iVar5 + 0x910) + 0x41c) = 0;
    fVar9 = 6.023215e-39;
    local_c84 = operator_new(0xc60);
    local_4._0_1_ = 0x15;
    if (local_c84 == (undefined1 *)0x0) {
      pBVar4 = (Ball *)0x0;
    }
    else {
      fVar9 = 6.023253e-39;
      pBVar4 = Ball_ctor2(local_c84,(int)this);
    }
    *(Ball **)((int)this + 0x361c) = pBVar4;
    local_4._0_1_ = 0x14;
    fVar10 = 6.023288e-39;
    (**(code **)(pBVar4->dwVtable + 4))();
    local_c84 = &stack0xfffff358;
    Matrix_Scale4x4(&stack0xfffff358,0x3f800000,0x3f800000,0,0x3eb33333);
    Ball_SetTrajectory(*(void **)((int)this + 0x361c),pvVar6,fVar7,fVar8,fVar9,fVar10);
    *(undefined4 *)(*(int *)((int)this + 0x361c) + 0x18) = 0xffffffff;
    *(undefined4 *)(*(int *)((int)this + 0x361c) + 0x278) = 0x3f000000;
    *(undefined4 *)(*(int *)((int)this + 0x361c) + 0x27c) = 0x3dcccccd;
    *(undefined4 *)(*(int *)((int)this + 0x361c) + 0x284) = 0x41d00000;
    *(undefined4 *)(*(int *)((int)this + 0x361c) + 0x1a0) = 0x3f4ccccd;
    *(undefined4 *)(*(int *)((int)this + 0x361c) + 0x188) = 0x4479c000;
    *(undefined1 *)(*(int *)((int)this + 0x361c) + 0x281) = 0;
  }
  local_4 = CONCAT31(local_4._1_3_,0x12);
  GameObject_dtor(local_c6c);
  ExceptionList = pvStack_c;
  return this;
}