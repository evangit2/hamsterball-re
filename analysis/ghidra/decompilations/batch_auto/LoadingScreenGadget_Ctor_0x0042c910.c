
void * __thiscall LoadingScreenGadget_Ctor(void *this,int param_1)

{
  void *pvVar1;
  void *pvVar2;
  int iVar3;
  undefined4 *puVar4;
  char *pcVar5;
  char cVar6;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cb160;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  Menu_Ctor(this,param_1);
  pvVar1 = (void *)((int)this + 0x2d88);
  local_4 = 0;
  *(undefined ***)this = &PTR_LoadingScreenGadget_DeletingDtor_004d3c78;
  AthenaList_Init(pvVar1,0);
  local_4._0_1_ = 1;
  AthenaString_Init((undefined4 *)((int)this + 0x31a8));
  local_4._0_1_ = 2;
  AthenaString_Init((undefined4 *)((int)this + 0x31c4));
  local_4._0_1_ = 3;
  D3DTexture_Ctor((undefined4 *)((int)this + 0x31e0));
  local_4._0_1_ = 4;
  AthenaList_Init((void *)((int)this + 0x320c),0);
  *(char **)((int)this + 0x868) = "LoadingScreen Gadget";
  *(undefined1 *)((int)this + 0x2d84) = 0;
  *(undefined4 *)((int)this + 0x3624) = 0x41200000;
  *(int *)((int)this + 0x2d68) = param_1;
  *(undefined1 *)((int)this + 0x3208) = 0;
  local_4._0_1_ = 5;
  if ((*(char *)(param_1 + 0x200) == '\0') && (*(int *)(param_1 + 0x914) < 1)) {
    *(undefined1 *)((int)this + 0x3208) = 1;
  }
  pvVar2 = operator_new(0xd4);
  local_4._0_1_ = 6;
  if (pvVar2 == (void *)0x0) {
    pvVar2 = (void *)0x0;
  }
  else {
    pvVar2 = Sprite_ctor(pvVar2,*(void **)(*(int *)((int)this + 0x2d68) + 0x174),"Loader.png");
  }
  local_4._0_1_ = 5;
  *(void **)((int)this + 0x2d6c) = pvVar2;
  pvVar2 = operator_new(0xd4);
  local_4._0_1_ = 7;
  if (pvVar2 == (void *)0x0) {
    pvVar2 = (void *)0x0;
  }
  else {
    pvVar2 = Sprite_ctor(pvVar2,*(void **)(*(int *)((int)this + 0x2d68) + 0x174),"Loader(Grey).png")
    ;
  }
  local_4._0_1_ = 5;
  *(void **)((int)this + 0x2d70) = pvVar2;
  pvVar2 = operator_new(0xd4);
  local_4._0_1_ = 8;
  if (pvVar2 == (void *)0x0) {
    pvVar2 = (void *)0x0;
  }
  else {
    pvVar2 = Sprite_ctor(pvVar2,*(void **)(*(int *)((int)this + 0x2d68) + 0x174),"Loadingswirl.png")
    ;
  }
  local_4._0_1_ = 5;
  *(void **)((int)this + 0x2d74) = pvVar2;
  pvVar2 = operator_new(0xd4);
  local_4._0_1_ = 9;
  if (pvVar2 == (void *)0x0) {
    pvVar2 = (void *)0x0;
  }
  else {
    pvVar2 = Sprite_ctor(pvVar2,*(void **)(*(int *)((int)this + 0x2d68) + 0x174),"loaderbkg.png");
  }
  *(void **)((int)this + 0x2d7c) = pvVar2;
  *(undefined4 *)(*(int *)((int)this + 0x2d68) + 0x40c) = *(undefined4 *)((int)this + 0x2d6c);
  local_4._0_1_ = 5;
  *(undefined4 *)(*(int *)((int)this + 0x2d68) + 0x410) = *(undefined4 *)((int)this + 0x2d7c);
  iVar3 = AthenaList_GetSize((int)this + 0x320c);
  if (iVar3 == 0) {
    puVar4 = operator_new(0xc);
    *puVar4 = 0x2a0;
    puVar4[1] = 0x1d8;
    pvVar2 = operator_new(0xd4);
    local_4._0_1_ = 10;
    if (pvVar2 == (void *)0x0) {
      pvVar2 = (void *)0x0;
    }
    else {
      pvVar2 = Sprite_ctor(pvVar2,*(void **)(*(int *)((int)this + 0x2d68) + 0x174),
                           "textures\\raptisoftlogo.png");
    }
    local_4._0_1_ = 5;
    puVar4[2] = pvVar2;
    AthenaList_Append((void *)((int)this + 0x320c),(int)puVar4);
  }
  *(undefined4 *)((int)this + 0x2d78) = 0;
  pvVar2 = operator_new(0xd4);
  local_4._0_1_ = 0xb;
  if (pvVar2 == (void *)0x0) {
    pvVar2 = (void *)0x0;
  }
  else {
    pvVar2 = Sprite_ctor(pvVar2,*(void **)(*(int *)((int)this + 0x2d68) + 0x174),"demo.png");
  }
  *(void **)((int)this + 0x2d78) = pvVar2;
  *(undefined4 *)((int)this + 4) = 0;
  *(undefined4 *)((int)this + 8) = 0;
  *(undefined4 *)((int)this + 0xc) = 0x44480000;
  *(undefined4 *)((int)this + 0x10) = 0x44160000;
  local_4 = CONCAT31(local_4._1_3_,5);
  Audio_PlayMusicAtSpeed(*(void **)(*(int *)((int)this + 0x2d68) + 0x534),"Loading",2.0);
  iVar3 = RNG_Rand(&PTR_OBJ_VTABLE,0x168,'\0');
  *(float *)((int)this + 0x2d80) = (float)iVar3;
  AthenaList_Free((int)pvVar1);
  if (*(char *)((int)this + 0x3208) == '\0') {
    AthenaList_InsertAt(pvVar1,"REGISTER HAMSTERBALL FOR FASTER LOADING!",0);
    cVar6 = '\0';
    iVar3 = AthenaList_GetSize((int)pvVar1);
    iVar3 = RNG_Rand(&PTR_OBJ_VTABLE,iVar3,cVar6);
    AthenaList_InsertAt(pvVar1,"SAVE AND CONTINUE YOUR TOURNAMENTS LATER!",iVar3);
    cVar6 = '\0';
    iVar3 = AthenaList_GetSize((int)pvVar1);
    iVar3 = RNG_Rand(&PTR_OBJ_VTABLE,iVar3,cVar6);
    AthenaList_InsertAt(pvVar1,"SAVE YOUR HIGH SCORES!",iVar3);
    cVar6 = '\0';
    iVar3 = AthenaList_GetSize((int)pvVar1);
    iVar3 = RNG_Rand(&PTR_OBJ_VTABLE,iVar3,cVar6);
    AthenaList_InsertAt(pvVar1,"NO MORE PLAY LIMITS!",iVar3);
    pcVar5 = "CLICK HERE TO REGISTER HAMSTERBALL!";
  }
  else {
    pcVar5 = "YOU MUST BUY HAMSTERBALL TO CONTINUE PLAYING!";
  }
  AthenaList_InsertAt(pvVar1,pcVar5,0);
  *(undefined4 *)((int)this + 0x31a0) = 0xffffffff;
  *(undefined4 *)((int)this + 0x31a4) = 0;
  pvVar1 = (void *)((int)this + 0x44c);
  *(undefined4 *)((int)this + 0x31ac) = 0;
  *(undefined4 *)((int)this + 0x31b0) = 0;
  *(undefined4 *)((int)this + 0x31b4) = 0;
  *(undefined4 *)((int)this + 0x31b8) = 0;
  AthenaList_Append(pvVar1,(int)this + 0x31a8);
  *(undefined4 *)((int)this + 0x31c8) = 0x43910000;
  *(undefined4 *)((int)this + 0x31cc) = 0x440c0000;
  *(undefined4 *)((int)this + 0x31d0) = 0x435c0000;
  *(undefined4 *)((int)this + 0x31d4) = 0x41f00000;
  *(undefined1 *)((int)this + 0x31da) = 0;
  AthenaList_Append(pvVar1,(int)this + 0x31c4);
  *(undefined4 *)((int)this + 0x31fc) = 0x2e0;
  *(undefined4 *)((int)this + 0x3200) = 0x218;
  *(undefined4 *)((int)this + 0x3204) = 0x40;
  AthenaList_Append(pvVar1,(int)this + 0x31e0);
  ExceptionList = local_c;
  return this;
}

