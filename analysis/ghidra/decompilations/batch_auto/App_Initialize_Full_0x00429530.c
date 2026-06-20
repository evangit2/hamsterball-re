
void __thiscall App_Initialize_Full(void *this,undefined4 param_1,undefined4 param_2)

{
  int iVar1;
  int *piVar2;
  char cVar3;
  HCURSOR pHVar4;
  void *pvVar5;
  undefined4 *puVar6;
  uint uVar7;
  void *unaff_EDI;
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
  *(char **)((int)this + 0x208) = "Initialize(3)";
  pHVar4 = LoadCursorA(*(HINSTANCE *)((int)this + 4),"BLANKCURSOR");
  *(HCURSOR *)((int)this + 0x240) = pHVar4;
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
  pvVar5 = Graphics_FindOrCreateTexture(*(void **)((int)this + 0x174),"shadow.png",'\x01');
  *(void **)((int)this + 0x278) = pvVar5;
  *(int *)((int)this + 0x1cc) = *(int *)((int)this + 0x1cc) + 1;
  *(char **)((int)this + 0x208) = "Initialize(7)";
  puVar6 = MusicChannel_LoadAndAppend(*(void **)((int)this + 0x17c),"music\\music.mo3");
  *(undefined4 **)((int)this + 0x534) = puVar6;
  *(undefined4 *)((int)this + 0x53c) = 0;
  *(undefined4 *)((int)this + 0x538) = 0;
  *(char **)((int)this + 0x208) = "Initialize(8)";
  if (puVar6 != (undefined4 *)0x0) {
    LoadJukebox("jukebox.xml");
    *(char **)((int)this + 0x208) = "Initialize(9)";
    puVar6 = RegKeyList_CopyFromSibling(*(int *)((int)this + 0x534));
    *(undefined4 **)((int)this + 0x53c) = puVar6;
    *(char **)((int)this + 0x208) = "Initialize(10)";
    puVar6 = RegKeyList_CopyFromSibling(*(int *)((int)this + 0x534));
    *(undefined4 **)((int)this + 0x538) = puVar6;
    *(char **)((int)this + 0x208) = "Initialize(11)";
  }
  *(char **)((int)this + 0x208) = "Initialize(12)";
  RegKey_Open(*(int *)((int)this + 0x54));
  *(char **)((int)this + 0x208) = "Initialize(13)";
  cVar3 = RegKey_ReadString(*(void **)((int)this + 0x54),"PlayCount");
  if (cVar3 == '\0') {
    *(undefined4 *)((int)this + 0x914) = 0x14;
  }
  else {
    uVar7 = RegKey_ReadDword(*(void **)((int)this + 0x54),"PlayCount");
    *(uint *)((int)this + 0x914) = uVar7;
  }
  *(undefined1 *)((int)this + 0x200) = 1;
  *(char **)((int)this + 0x208) = "Initialize(15)";
  pvVar5 = operator_new(0x14);
  pvStack_c = (void *)0x0;
  if (pvVar5 == (void *)0x0) {
    pvVar5 = (void *)0x0;
  }
  else {
    pvVar5 = (void *)NetworkConnection_Ctor(pvVar5,*(undefined4 *)((int)this + 0x180));
  }
  pvStack_c = (void *)0xffffffff;
  *(void **)((int)this + 0x550) = pvVar5;
  *(char **)((int)this + 0x208) = "Initialize(16)";
  InputDevice_SetType(pvVar5,1);
  *(char **)((int)this + 0x208) = "Initialize(17)";
  pvVar5 = operator_new(0x14);
  pvStack_c = (void *)0x1;
  if (pvVar5 == (void *)0x0) {
    pvVar5 = (void *)0x0;
  }
  else {
    pvVar5 = (void *)NetworkConnection_Ctor(pvVar5,*(undefined4 *)((int)this + 0x180));
  }
  pvStack_c = (void *)0xffffffff;
  *(void **)((int)this + 0x554) = pvVar5;
  *(char **)((int)this + 0x208) = "Initialize(18)";
  InputDevice_SetType(pvVar5,2);
  *(char **)((int)this + 0x208) = "Initialize(19)";
  pvVar5 = operator_new(0x14);
  pvStack_c = (void *)0x2;
  if (pvVar5 == (void *)0x0) {
    pvVar5 = (void *)0x0;
  }
  else {
    pvVar5 = (void *)NetworkConnection_Ctor(pvVar5,*(undefined4 *)((int)this + 0x180));
  }
  pvStack_c = (void *)0xffffffff;
  *(void **)((int)this + 0x558) = pvVar5;
  *(char **)((int)this + 0x208) = "Initialize(20)";
  InputDevice_SetType(pvVar5,4);
  *(char **)((int)this + 0x208) = "Initialize(21)";
  pvVar5 = operator_new(0x14);
  pvStack_c = (void *)0x3;
  if (pvVar5 == (void *)0x0) {
    pvVar5 = (void *)0x0;
  }
  else {
    pvVar5 = (void *)NetworkConnection_Ctor(pvVar5,*(undefined4 *)((int)this + 0x180));
  }
  pvStack_c = (void *)0xffffffff;
  *(void **)((int)this + 0x55c) = pvVar5;
  *(char **)((int)this + 0x208) = "Initialize(22)";
  InputDevice_SetType(pvVar5,5);
  *(char **)((int)this + 0x208) = "Initialize(23)";
  RegKey_Close(*(int *)((int)this + 0x54));
  *(char **)((int)this + 0x208) = "Initialize(25)";
  (**(code **)(*(int *)this + 0xa0))();
  *(char **)((int)this + 0x208) = "Initialize(26)";
  ExceptionList = unaff_EDI;
  return;
}

