
void __thiscall App_Initialize_Full(void *this,undefined4 param_1,undefined4 param_2)

{
  int iVar1;
  int *piVar2;
  undefined4 uVar3;
  undefined4 *puVar4;
  uint uVar5;
  void *pvVar6;
  void *pvVar7;
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 uStack_4;
  
  uStack_4 = 0xffffffff;
  puStack_8 = &LAB_004caeec;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  *(char **)((int)this + 0x208) = "Initialize(1)";
  App_Initialize(this,param_1,param_2);
  *(undefined1 *)(*(int *)((int)this + 0x174) + 0x7d1) = 1;
  pvVar7 = *(void **)((int)this + 4);
  *(char **)((int)this + 0x208) = "Initialize(3)";
  uVar3 = LoadCursorA(pvVar7,"BLANKCURSOR");
  *(undefined4 *)((int)this + 0x240) = uVar3;
  *(char **)((int)this + 0x208) = "Initialize(4)";
  (**(code **)(*(int *)this + 0x8c))(800,600);
  iVar1 = *(int *)((int)this + 0x174);
  *(char **)((int)this + 0x208) = "Initialize(5)";
  if (iVar1 == 0) {
    *(char **)((int)this + 0x208) = "** No Graphics **";
  }
  if (*(int *)(iVar1 + 0x154) == 0) {
    *(char **)((int)this + 0x208) = "** No Graphics Device **";
  }
  if (iVar1 != 0) {
    piVar2 = *(int **)(iVar1 + 0x154);
    if (*(char *)(iVar1 + 0x7d2) == '\0') {
      (**(code **)(*piVar2 + 200))(piVar2,0x16,3);
    }
    else {
      (**(code **)(*piVar2 + 200))(piVar2,0x16,2);
    }
    *(undefined4 *)(iVar1 + 0x708) = 3;
  }
  *(char **)((int)this + 0x208) = "Initialize(6)";
  puVar4 = Graphics_LoadTexture(*(void **)((int)this + 0x174),"shadow.png",'\x01');
  *(undefined4 **)((int)this + 0x278) = puVar4;
  *(int *)((int)this + 0x1cc) = *(int *)((int)this + 0x1cc) + 1;
  *(char **)((int)this + 0x208) = "Initialize(7)";
  puVar4 = FUN_004743f0(*(void **)((int)this + 0x17c),"music\\music.mo3");
  *(undefined4 **)((int)this + 0x534) = puVar4;
  *(undefined4 *)((int)this + 0x53c) = 0;
  *(undefined4 *)((int)this + 0x538) = 0;
  *(char **)((int)this + 0x208) = "Initialize(8)";
  if (puVar4 != (undefined4 *)0x0) {
    LoadJukebox("jukebox.xml");
    *(char **)((int)this + 0x208) = "Initialize(9)";
    puVar4 = FUN_0046a3c0(*(int *)((int)this + 0x534));
    *(undefined4 **)((int)this + 0x53c) = puVar4;
    *(char **)((int)this + 0x208) = "Initialize(10)";
    puVar4 = FUN_0046a3c0(*(int *)((int)this + 0x534));
    *(undefined4 **)((int)this + 0x538) = puVar4;
    *(char **)((int)this + 0x208) = "Initialize(11)";
  }
  *(char **)((int)this + 0x208) = "Initialize(12)";
  FUN_00472ec0(*(int *)((int)this + 0x54));
  *(char **)((int)this + 0x208) = "Initialize(13)";
  uVar3 = FUN_00473170(*(void **)((int)this + 0x54),"PlayCount");
  if ((char)uVar3 == '\0') {
    *(undefined4 *)((int)this + 0x914) = 0x14;
  }
  else {
    uVar5 = FUN_00473080(*(void **)((int)this + 0x54),"PlayCount");
    *(uint *)((int)this + 0x914) = uVar5;
  }
  *(undefined1 *)((int)this + 0x200) = 1;
  *(char **)((int)this + 0x208) = "Initialize(15)";
  puStack_8 = operator_new(0x14);
  if (puStack_8 == (void *)0x0) {
    pvVar6 = (void *)0x0;
  }
  else {
    pvVar6 = (void *)FUN_0046dfa0(puStack_8,*(undefined4 *)((int)this + 0x180));
  }
  *(void **)((int)this + 0x550) = pvVar6;
  *(char **)((int)this + 0x208) = "Initialize(16)";
  FUN_0046dfc0(pvVar6,1);
  *(char **)((int)this + 0x208) = "Initialize(17)";
  puStack_8 = operator_new(0x14);
  if (puStack_8 == (void *)0x0) {
    pvVar6 = (void *)0x0;
  }
  else {
    pvVar6 = (void *)FUN_0046dfa0(puStack_8,*(undefined4 *)((int)this + 0x180));
  }
  *(void **)((int)this + 0x554) = pvVar6;
  *(char **)((int)this + 0x208) = "Initialize(18)";
  FUN_0046dfc0(pvVar6,2);
  *(char **)((int)this + 0x208) = "Initialize(19)";
  puStack_8 = operator_new(0x14);
  if (puStack_8 == (void *)0x0) {
    pvVar6 = (void *)0x0;
  }
  else {
    pvVar6 = (void *)FUN_0046dfa0(puStack_8,*(undefined4 *)((int)this + 0x180));
  }
  *(void **)((int)this + 0x558) = pvVar6;
  *(char **)((int)this + 0x208) = "Initialize(20)";
  FUN_0046dfc0(pvVar6,4);
  *(char **)((int)this + 0x208) = "Initialize(21)";
  puStack_8 = operator_new(0x14);
  if (puStack_8 == (void *)0x0) {
    pvVar6 = (void *)0x0;
  }
  else {
    pvVar6 = (void *)FUN_0046dfa0(puStack_8,*(undefined4 *)((int)this + 0x180));
  }
  *(void **)((int)this + 0x55c) = pvVar6;
  *(char **)((int)this + 0x208) = "Initialize(22)";
  FUN_0046dfc0(pvVar6,5);
  *(char **)((int)this + 0x208) = "Initialize(23)";
  FUN_00472f30(*(int *)((int)this + 0x54));
  *(char **)((int)this + 0x208) = "Initialize(25)";
  (**(code **)(*(int *)this + 0xa0))();
  *(char **)((int)this + 0x208) = "Initialize(26)";
  ExceptionList = pvVar7;
  return;
}

