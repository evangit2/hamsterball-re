
void * __thiscall SplashScreen_Ctor(void *this,int param_1)

{
  int iVar1;
  void *pvVar2;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cdd89;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  Gadget_ctor(this,param_1);
  local_4._0_1_ = 0;
  local_4._1_3_ = 0;
  *(undefined ***)this = &PTR_SceneObject_ScalarDtor_004da6c8;
  *(int *)((int)this + 0x878) = param_1;
  *(undefined4 *)((int)this + 0x880) = 0;
  *(undefined4 *)((int)this + 0x884) = 0xffffffff;
  iVar1 = _check_file_access("textures\\brand.png",0);
  if (iVar1 != -1) {
    pvVar2 = operator_new(0xd4);
    local_4._0_1_ = 1;
    if (pvVar2 == (void *)0x0) {
      pvVar2 = (void *)0x0;
    }
    else {
      pvVar2 = Sprite_ctor(pvVar2,*(void **)(param_1 + 0x174),"textures\\brand.png");
    }
    local_4._0_1_ = 0;
    *(void **)((int)this + 0x880) = pvVar2;
    *(undefined4 *)((int)this + 0x884) = 0;
    *(undefined4 *)((int)this + 0x888) = 0;
    *(undefined4 *)((int)this + 0x88c) = 0x32;
  }
  pvVar2 = operator_new(0xd4);
  local_4._0_1_ = 2;
  if (pvVar2 == (void *)0x0) {
    pvVar2 = (void *)0x0;
  }
  else {
    pvVar2 = Sprite_ctor(pvVar2,*(void **)(param_1 + 0x174),"textures\\raptisoftlogo.png");
  }
  local_4._0_1_ = 0;
  *(void **)((int)this + 0x87c) = pvVar2;
  *(undefined4 *)((int)this + 0x894) = 0x42c80000;
  *(undefined4 *)((int)this + 0x898) = 0;
  *(undefined4 *)((int)this + 0x89c) = 0;
  pvVar2 = operator_new(0x182c);
  local_4 = CONCAT31(local_4._1_3_,3);
  if (pvVar2 == (void *)0x0) {
    *(undefined4 *)((int)this + 0x8a0) = 0;
  }
  else {
    pvVar2 = LoadFont(pvVar2,*(undefined4 *)(param_1 + 0x174),"fonts\\showcardgothic16");
    *(void **)((int)this + 0x8a0) = pvVar2;
  }
  ExceptionList = local_c;
  return this;
}

