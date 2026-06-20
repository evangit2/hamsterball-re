
void * __thiscall PracticeMenu_ctor(void *this,int param_1)

{
  void *pvVar1;
  void *pvVar2;
  char *pcVar3;
  undefined4 in_stack_ffffffd4;
  undefined4 in_stack_ffffffd8;
  undefined4 in_stack_ffffffdc;
  undefined4 in_stack_ffffffe0;
  undefined4 uVar4;
  int iVar5;
  void *local_c;
  undefined1 *puStack_8;
  int local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cb35d;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  SimpleMenu_ctor(this,param_1);
  local_4 = 0;
  *(undefined ***)this = &PTR_OptionsMenu_DeletingDtor3_004d4560;
  *(char **)((int)this + 0x868) = "Practice Menu";
  *(undefined4 *)((int)this + 0xccc) = 100;
  *(undefined4 *)((int)this + 0xcc4) = 100;
  *(undefined4 *)((int)this + 0xcc8) = 0;
  *(undefined1 *)((int)this + 0xcd0) = 1;
  *(char **)((int)this + 0x888) = "CHOOSE A PRACTICE RACE!";
  pvVar1 = operator_new(0xd4);
  local_4._0_1_ = 1;
  if (pvVar1 == (void *)0x0) {
    pvVar1 = (void *)0x0;
  }
  else {
    in_stack_ffffffe0 = 0x42eace;
    pvVar1 = Sprite_ctor(pvVar1,*(void **)(*(int *)((int)this + 0x878) + 0x174),
                         "Textures\\practice-level1.png");
  }
  local_4._0_1_ = 0;
  *(void **)((int)this + 0xcdc) = pvVar1;
  pvVar1 = operator_new(0xd4);
  local_4._0_1_ = 2;
  if (pvVar1 == (void *)0x0) {
    pvVar1 = (void *)0x0;
  }
  else {
    in_stack_ffffffe0 = 0x42eb10;
    pvVar1 = Sprite_ctor(pvVar1,*(void **)(*(int *)((int)this + 0x878) + 0x174),
                         "Textures\\practice-cascade.png");
  }
  local_4._0_1_ = 0;
  *(void **)((int)this + 0xce0) = pvVar1;
  pvVar1 = operator_new(0xd4);
  local_4._0_1_ = 3;
  if (pvVar1 == (void *)0x0) {
    pvVar1 = (void *)0x0;
  }
  else {
    in_stack_ffffffe0 = 0x42eb52;
    pvVar1 = Sprite_ctor(pvVar1,*(void **)(*(int *)((int)this + 0x878) + 0x174),
                         "Textures\\practice-level2.png");
  }
  local_4._0_1_ = 0;
  *(void **)((int)this + 0xce4) = pvVar1;
  pvVar1 = operator_new(0xd4);
  local_4._0_1_ = 4;
  if (pvVar1 == (void *)0x0) {
    pvVar1 = (void *)0x0;
  }
  else {
    in_stack_ffffffe0 = 0x42eb94;
    pvVar1 = Sprite_ctor(pvVar1,*(void **)(*(int *)((int)this + 0x878) + 0x174),
                         "Textures\\practice-level3.png");
  }
  local_4._0_1_ = 0;
  *(void **)((int)this + 0xce8) = pvVar1;
  pvVar1 = operator_new(0xd4);
  local_4._0_1_ = 5;
  if (pvVar1 == (void *)0x0) {
    pvVar1 = (void *)0x0;
  }
  else {
    in_stack_ffffffe0 = 0x42ebd6;
    pvVar1 = Sprite_ctor(pvVar1,*(void **)(*(int *)((int)this + 0x878) + 0x174),
                         "Textures\\practice-level4.png");
  }
  local_4._0_1_ = 0;
  *(void **)((int)this + 0xcec) = pvVar1;
  pvVar1 = operator_new(0xd4);
  local_4._0_1_ = 6;
  if (pvVar1 == (void *)0x0) {
    pvVar1 = (void *)0x0;
  }
  else {
    in_stack_ffffffe0 = 0x42ec18;
    pvVar1 = Sprite_ctor(pvVar1,*(void **)(*(int *)((int)this + 0x878) + 0x174),
                         "Textures\\practice-up.png");
  }
  local_4._0_1_ = 0;
  *(void **)((int)this + 0xcf0) = pvVar1;
  pvVar1 = operator_new(0xd4);
  local_4._0_1_ = 7;
  if (pvVar1 == (void *)0x0) {
    pvVar1 = (void *)0x0;
  }
  else {
    in_stack_ffffffe0 = 0x42ec5a;
    pvVar1 = Sprite_ctor(pvVar1,*(void **)(*(int *)((int)this + 0x878) + 0x174),
                         "Textures\\practice-level5.png");
  }
  local_4._0_1_ = 0;
  *(void **)((int)this + 0xcf4) = pvVar1;
  pvVar1 = operator_new(0xd4);
  local_4._0_1_ = 8;
  if (pvVar1 == (void *)0x0) {
    pvVar1 = (void *)0x0;
  }
  else {
    in_stack_ffffffe0 = 0x42ec9c;
    pvVar1 = Sprite_ctor(pvVar1,*(void **)(*(int *)((int)this + 0x878) + 0x174),
                         "Textures\\practice-level6.png");
  }
  local_4._0_1_ = 0;
  *(void **)((int)this + 0xcf8) = pvVar1;
  pvVar1 = operator_new(0xd4);
  local_4._0_1_ = 9;
  if (pvVar1 == (void *)0x0) {
    pvVar1 = (void *)0x0;
  }
  else {
    in_stack_ffffffe0 = 0x42ecde;
    pvVar1 = Sprite_ctor(pvVar1,*(void **)(*(int *)((int)this + 0x878) + 0x174),
                         "Textures\\practice-level7.png");
  }
  local_4._0_1_ = 0;
  *(void **)((int)this + 0xcfc) = pvVar1;
  pvVar1 = operator_new(0xd4);
  local_4._0_1_ = 10;
  if (pvVar1 == (void *)0x0) {
    pvVar1 = (void *)0x0;
  }
  else {
    in_stack_ffffffe0 = 0x42ed20;
    pvVar1 = Sprite_ctor(pvVar1,*(void **)(*(int *)((int)this + 0x878) + 0x174),
                         "Textures\\practice-level8.png");
  }
  local_4._0_1_ = 0;
  *(void **)((int)this + 0xd00) = pvVar1;
  pvVar1 = operator_new(0xd4);
  local_4._0_1_ = 0xb;
  if (pvVar1 == (void *)0x0) {
    pvVar1 = (void *)0x0;
  }
  else {
    in_stack_ffffffe0 = 0x42ed62;
    pvVar1 = Sprite_ctor(pvVar1,*(void **)(*(int *)((int)this + 0x878) + 0x174),
                         "Textures\\practice-level9.png");
  }
  local_4._0_1_ = 0;
  *(void **)((int)this + 0xd04) = pvVar1;
  pvVar1 = operator_new(0xd4);
  local_4._0_1_ = 0xc;
  if (pvVar1 == (void *)0x0) {
    pvVar1 = (void *)0x0;
  }
  else {
    in_stack_ffffffe0 = 0x42eda4;
    pvVar1 = Sprite_ctor(pvVar1,*(void **)(*(int *)((int)this + 0x878) + 0x174),
                         "Textures\\practice-level10.png");
  }
  local_4._0_1_ = 0;
  *(void **)((int)this + 0xd08) = pvVar1;
  pvVar1 = operator_new(0xd4);
  local_4._0_1_ = 0xd;
  if (pvVar1 == (void *)0x0) {
    pvVar1 = (void *)0x0;
  }
  else {
    in_stack_ffffffe0 = 0x42ede6;
    pvVar1 = Sprite_ctor(pvVar1,*(void **)(*(int *)((int)this + 0x878) + 0x174),
                         "Textures\\practice-neon.png");
  }
  local_4._0_1_ = 0;
  *(void **)((int)this + 0xd0c) = pvVar1;
  pvVar1 = operator_new(0xd4);
  local_4._0_1_ = 0xe;
  if (pvVar1 == (void *)0x0) {
    pvVar1 = (void *)0x0;
  }
  else {
    in_stack_ffffffe0 = 0x42ee28;
    pvVar1 = Sprite_ctor(pvVar1,*(void **)(*(int *)((int)this + 0x878) + 0x174),
                         "Textures\\practice-glass.png");
  }
  local_4._0_1_ = 0;
  *(void **)((int)this + 0xd10) = pvVar1;
  pvVar1 = (void *)0x42ee41;
  pvVar2 = operator_new(0xd4);
  local_4._0_1_ = 0xf;
  if (pvVar2 == (void *)0x0) {
    pvVar2 = (void *)0x0;
  }
  else {
    pvVar1 = *(void **)(*(int *)((int)this + 0x878) + 0x174);
    in_stack_ffffffe0 = 0x42ee6a;
    pvVar2 = Sprite_ctor(pvVar2,pvVar1,"Textures\\practice-impossible.png");
  }
  *(void **)((int)this + 0xd14) = pvVar2;
  iVar5 = *(int *)((int)this + 0xcdc);
  local_4 = (uint)local_4._1_3_ << 8;
  Matrix_Scale4x4(&stack0xffffffd4,0x3f800000,0x3f400000,0x3f800000,0x3f800000);
  UIList_AddItem(this,"WARM-UP RACE","0",in_stack_ffffffd4,in_stack_ffffffd8,in_stack_ffffffdc,
                 in_stack_ffffffe0,pvVar1,iVar5);
  iVar5 = *(int *)((int)this + 0xce0);
  Matrix_Scale4x4(&stack0xffffffd4,0x3f800000,0x3f400000,0x3e800000,0x3f800000);
  UIList_AddItem(this,"BEGINNER RACE","1",in_stack_ffffffd4,in_stack_ffffffd8,in_stack_ffffffdc,
                 in_stack_ffffffe0,pvVar1,iVar5);
  iVar5 = *(int *)((int)this + 0xce4);
  Matrix_Scale4x4(&stack0xffffffd4,0x3f400000,0x3f400000,0x3f800000,0x3f800000);
  UIList_AddItem(this,"INTERMEDIATE RACE","2",in_stack_ffffffd4,in_stack_ffffffd8,in_stack_ffffffdc,
                 in_stack_ffffffe0,pvVar1,iVar5);
  uVar4 = 0x42ef30;
  UIList_AddSpacer(this,10);
  if (*(char *)(*(int *)((int)this + 0x878) + 0x851) == '\0') {
    iVar5 = *(int *)(*(int *)((int)this + 0x878) + 0x3ac);
    Matrix_Scale4x4(&stack0xffffffd4,0x3f266666,0x3f266666,0x3f266666,0x3f800000);
    pcVar3 = "LOCKED DIZZY";
  }
  else {
    iVar5 = *(int *)((int)this + 0xce8);
    Matrix_Scale4x4(&stack0xffffffd4,0x3f400000,0x3f800000,0x3f400000,0x3f800000);
    pcVar3 = "3";
  }
  UIList_AddItem(this,"DIZZY RACE",pcVar3,in_stack_ffffffd4,in_stack_ffffffd8,in_stack_ffffffdc,
                 in_stack_ffffffe0,uVar4,iVar5);
  if (*(char *)(*(int *)((int)this + 0x878) + 0x852) == '\0') {
    iVar5 = *(int *)(*(int *)((int)this + 0x878) + 0x3ac);
    Matrix_Scale4x4(&stack0xffffffd4,0x3f266666,0x3f266666,0x3f266666,0x3f800000);
    pcVar3 = "LOCKED TOWER";
  }
  else {
    iVar5 = *(int *)((int)this + 0xcec);
    Matrix_Scale4x4(&stack0xffffffd4,0x3f800000,0x3f666666,0x3ee66666,0x3f800000);
    pcVar3 = "4";
  }
  UIList_AddItem(this,"TOWER RACE",pcVar3,in_stack_ffffffd4,in_stack_ffffffd8,in_stack_ffffffdc,
                 in_stack_ffffffe0,uVar4,iVar5);
  if (*(char *)(*(int *)((int)this + 0x878) + 0x853) == '\0') {
    iVar5 = *(int *)(*(int *)((int)this + 0x878) + 0x3ac);
    Matrix_Scale4x4(&stack0xffffffd4,0x3f266666,0x3f266666,0x3f266666,0x3f800000);
    pcVar3 = "LOCKED UP";
  }
  else {
    iVar5 = *(int *)((int)this + 0xcf0);
    Matrix_Scale4x4(&stack0xffffffd4,0x3f400000,0x3ee66666,0x3f800000,0x3f800000);
    pcVar3 = "5";
  }
  UIList_AddItem(this,"UP RACE",pcVar3,in_stack_ffffffd4,in_stack_ffffffd8,in_stack_ffffffdc,
                 in_stack_ffffffe0,uVar4,iVar5);
  uVar4 = 0x42f0a7;
  UIList_AddSpacer(this,10);
  if (*(char *)(*(int *)((int)this + 0x878) + 0x863) == '\0') {
    iVar5 = *(int *)(*(int *)((int)this + 0x878) + 0x3ac);
    Matrix_Scale4x4(&stack0xffffffd4,0x3f266666,0x3f266666,0x3f266666,0x3f800000);
    pcVar3 = "LOCKED NEON";
  }
  else {
    iVar5 = *(int *)((int)this + 0xd0c);
    Matrix_Scale4x4(&stack0xffffffd4,0x3f800000,0x3f800000,0,0x3f800000);
    pcVar3 = "6";
  }
  UIList_AddItem(this,"NEON RACE",pcVar3,in_stack_ffffffd4,in_stack_ffffffd8,in_stack_ffffffdc,
                 in_stack_ffffffe0,uVar4,iVar5);
  if (*(char *)(*(int *)((int)this + 0x878) + 0x854) == '\0') {
    iVar5 = *(int *)(*(int *)((int)this + 0x878) + 0x3ac);
    Matrix_Scale4x4(&stack0xffffffd4,0x3f266666,0x3f266666,0x3f266666,0x3f800000);
    pcVar3 = "LOCKED EXPERT";
  }
  else {
    iVar5 = *(int *)((int)this + 0xcf4);
    Matrix_Scale4x4(&stack0xffffffd4,0x3f800000,0x3e800000,0x3e800000,0x3f800000);
    pcVar3 = "7";
  }
  UIList_AddItem(this,"EXPERT RACE",pcVar3,in_stack_ffffffd4,in_stack_ffffffd8,in_stack_ffffffdc,
                 in_stack_ffffffe0,uVar4,iVar5);
  if (*(char *)(*(int *)((int)this + 0x878) + 0x855) == '\0') {
    iVar5 = *(int *)(*(int *)((int)this + 0x878) + 0x3ac);
    Matrix_Scale4x4(&stack0xffffffd4,0x3f266666,0x3f266666,0x3f266666,0x3f800000);
    pcVar3 = "LOCKED ODD";
  }
  else {
    iVar5 = *(int *)((int)this + 0xcf8);
    Matrix_Scale4x4(&stack0xffffffd4,0x3f800000,0x3f400000,0,0x3f800000);
    pcVar3 = "8";
  }
  UIList_AddItem(this,"ODD RACE",pcVar3,in_stack_ffffffd4,in_stack_ffffffd8,in_stack_ffffffdc,
                 in_stack_ffffffe0,uVar4,iVar5);
  uVar4 = 0x42f218;
  UIList_AddSpacer(this,10);
  if (*(char *)(*(int *)((int)this + 0x878) + 0x856) == '\0') {
    iVar5 = *(int *)(*(int *)((int)this + 0x878) + 0x3ac);
    Matrix_Scale4x4(&stack0xffffffd4,0x3f266666,0x3f266666,0x3f266666,0x3f800000);
    pcVar3 = "LOCKED TOOB";
  }
  else {
    iVar5 = *(int *)((int)this + 0xd00);
    Matrix_Scale4x4(&stack0xffffffd4,0x3f400000,0x3f400000,0x3f800000,0x3f800000);
    pcVar3 = "9";
  }
  UIList_AddItem(this,"TOOB RACE",pcVar3,in_stack_ffffffd4,in_stack_ffffffd8,in_stack_ffffffdc,
                 in_stack_ffffffe0,uVar4,iVar5);
  if (*(char *)(*(int *)((int)this + 0x878) + 0x857) == '\0') {
    iVar5 = *(int *)(*(int *)((int)this + 0x878) + 0x3ac);
    Matrix_Scale4x4(&stack0xffffffd4,0x3f266666,0x3f266666,0x3f266666,0x3f800000);
    pcVar3 = "LOCKED WOBBLY";
  }
  else {
    iVar5 = *(int *)((int)this + 0xcfc);
    Matrix_Scale4x4(&stack0xffffffd4,0x3f1c28f6,0x3f70a3d7,0x3e947ae1,0x3f800000);
    pcVar3 = "10";
  }
  UIList_AddItem(this,"WOBBLY RACE",pcVar3,in_stack_ffffffd4,in_stack_ffffffd8,in_stack_ffffffdc,
                 in_stack_ffffffe0,uVar4,iVar5);
  if (*(char *)(*(int *)((int)this + 0x878) + 0x864) == '\0') {
    iVar5 = *(int *)(*(int *)((int)this + 0x878) + 0x3ac);
    Matrix_Scale4x4(&stack0xffffffd4,0x3f266666,0x3f266666,0x3f266666,0x3f800000);
    pcVar3 = "LOCKED GLASS";
  }
  else {
    iVar5 = *(int *)((int)this + 0xd10);
    Matrix_Scale4x4(&stack0xffffffd4,0x3f800000,0x3f400000,0x3f800000,0x3f800000);
    pcVar3 = "11";
  }
  UIList_AddItem(this,"GLASS RACE",pcVar3,in_stack_ffffffd4,in_stack_ffffffd8,in_stack_ffffffdc,
                 in_stack_ffffffe0,uVar4,iVar5);
  uVar4 = 0x42f38f;
  UIList_AddSpacer(this,10);
  if (*(char *)(*(int *)((int)this + 0x878) + 0x858) == '\0') {
    iVar5 = *(int *)(*(int *)((int)this + 0x878) + 0x3ac);
    Matrix_Scale4x4(&stack0xffffffd4,0x3f266666,0x3f266666,0x3f266666,0x3f800000);
    pcVar3 = "LOCKED SKY";
  }
  else {
    iVar5 = *(int *)((int)this + 0xd04);
    Matrix_Scale4x4(&stack0xffffffd4,0x3e800000,0x3f400000,0x3f800000,0x3f800000);
    pcVar3 = "12";
  }
  UIList_AddItem(this,"SKY RACE",pcVar3,in_stack_ffffffd4,in_stack_ffffffd8,in_stack_ffffffdc,
                 in_stack_ffffffe0,uVar4,iVar5);
  if (*(char *)(*(int *)((int)this + 0x878) + 0x859) == '\0') {
    iVar5 = *(int *)(*(int *)((int)this + 0x878) + 0x3ac);
    Matrix_Scale4x4(&stack0xffffffd4,0x3f266666,0x3f266666,0x3f266666,0x3f800000);
    pcVar3 = "LOCKED MASTER";
  }
  else {
    iVar5 = *(int *)((int)this + 0xd08);
    Matrix_Scale4x4(&stack0xffffffd4,0x3f59999a,0x3f3851ec,0x3e8a3d71,0x3f800000);
    pcVar3 = "13";
  }
  UIList_AddItem(this,"MASTER RACE",pcVar3,in_stack_ffffffd4,in_stack_ffffffd8,in_stack_ffffffdc,
                 in_stack_ffffffe0,uVar4,iVar5);
  if (*(char *)(*(int *)((int)this + 0x878) + 0x865) == '\0') {
    iVar5 = *(int *)(*(int *)((int)this + 0x878) + 0x3ac);
    Matrix_Scale4x4(&stack0xffffffd4,0x3f266666,0x3f266666,0x3f266666,0x3f800000);
    pcVar3 = "LOCKED IMPOSSIBLE";
  }
  else {
    iVar5 = *(int *)((int)this + 0xd14);
    Matrix_Scale4x4(&stack0xffffffd4,0x3f800000,0,0,0x3f800000);
    pcVar3 = "14";
  }
  UIList_AddItem(this,"IMPOSSIBLE RACE",pcVar3,in_stack_ffffffd4,in_stack_ffffffd8,in_stack_ffffffdc
                 ,in_stack_ffffffe0,uVar4,iVar5);
  uVar4 = 0x42f500;
  UIList_AddSpacer(this,10);
  iVar5 = 0;
  Matrix_Scale4x4(&stack0xffffffd4,0x3f800000,0x3f400000,0x3f400000,0x3f800000);
  UIList_AddItem(this,"PREVIOUS MENU","BACK",in_stack_ffffffd4,in_stack_ffffffd8,in_stack_ffffffdc,
                 in_stack_ffffffe0,uVar4,iVar5);
  ExceptionList = local_c;
  return this;
}

