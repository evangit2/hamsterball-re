
void __thiscall WebClient_FetchURL(void *this)

{
  void *this_00;
  int iVar1;
  size_t sVar2;
  HMODULE hInstance;
  HWND pHVar3;
  UINT in_stack_00000020;
  undefined4 in_stack_00000024;
  undefined4 in_stack_ffffff8c;
  char *in_stack_ffffff90;
  LPVOID lpParam;
  undefined **local_44;
  undefined4 local_40;
  undefined4 local_3c;
  undefined4 local_38;
  undefined1 local_34;
  undefined4 local_30;
  undefined1 local_2c;
  undefined4 local_28 [7];
  void *pvStack_c;
  undefined1 *puStack_8;
  int local_4;
  
  puStack_8 = &LAB_004ce0b8;
  pvStack_c = ExceptionList;
  local_4 = 0;
  ExceptionList = &pvStack_c;
  SoundDevice_ReleaseWindow((int)this);
  iVar1 = StdString_FindSubstr(&stack0x00000004,"http://");
  if (iVar1 == -1) {
    *(undefined4 *)((int)this + 4) = 3;
    AthenaString_AssignCStr((void *)((int)this + 8),"Invalid URL (no http://)");
    (**(code **)(*(int *)this + 4))();
  }
  else {
    AthenaString_Substr(&stack0x00000004,local_28,iVar1 + 7,9999);
    local_4 = CONCAT31(local_4._1_3_,1);
    sVar2 = StdString_FindSubstr(local_28,"/");
    if (sVar2 != 0xffffffff) {
      AthenaString_Truncate(local_28,sVar2);
    }
    local_44 = &PTR_AthenaCString_DeletingDtor_004d290c;
    local_40 = 0;
    local_38 = 0;
    local_2c = 1;
    local_3c = 0;
    local_30 = 0;
    local_34 = 0;
    AthenaString_AssignCStr(&local_44,(char *)0x0);
    local_4 = CONCAT31(local_4._1_3_,2);
    AthenaString_CopyCtor(&stack0xffffff8c,(int)&stack0x00000004);
    AthenaString_AssignCStrFree(&local_44,in_stack_ffffff8c,in_stack_ffffff90);
    iVar1 = StdString_FindSubstr(&local_44," ");
    while (iVar1 != -1) {
      AthenaString_ReplaceSubstr(&local_44," ","%20");
      iVar1 = StdString_FindSubstr(&local_44," ");
    }
    this_00 = (void *)((int)this + 0x24);
    StdString_FreeBuffer((int)this_00);
    AthenaString_Assign(this_00,"GET ");
    AthenaString_CopyCtor(&stack0xffffff8c,(int)&local_44);
    AthenaString_AssignFree(this_00,in_stack_ffffff8c,in_stack_ffffff90);
    AthenaString_Assign(this_00," HTTP/1.0");
    AthenaString_AssignCRLF(this_00);
    AthenaString_Assign(this_00,"User-Agent:");
    AthenaString_CopyCtor(&stack0xffffff8c,(int)this + 0x40);
    AthenaString_AssignFree(this_00,in_stack_ffffff8c,in_stack_ffffff90);
    AthenaString_AssignCRLF(this_00);
    AthenaString_AssignCRLF(this_00);
    lpParam = (LPVOID)0x0;
    *(undefined4 *)((int)this + 4) = 2;
    hInstance = GetModuleHandleA((LPCSTR)0x0);
    pHVar3 = CreateWindowExA(0,"WebWindow","WebWindow",0x80000000,0,0,10,10,(HWND)0x0,(HMENU)0x0,
                             hInstance,lpParam);
    *(HWND *)((int)this + 0x610) = pHVar3;
    (**(code **)(*(int *)this + 4))();
    if (*(HWND *)((int)this + 0x610) == (HWND)0x0) {
      *(undefined4 *)((int)this + 4) = 3;
      AthenaString_AssignCStr((void *)((int)this + 8),"Could not create WebClient window");
      (**(code **)(*(int *)this + 4))();
    }
    else {
      SetWindowLongA(*(HWND *)((int)this + 0x610),-0x15,(LONG)this);
      iVar1 = Ordinal_103();
      *(int *)((int)this + 0x45c) = iVar1;
      if (iVar1 == 0) {
        *(undefined4 *)((int)this + 4) = 3;
        AthenaString_AssignCStr((void *)((int)this + 8),"Could start WSAAsyncGetHostByName()");
        (**(code **)(*(int *)this + 4))();
      }
      else {
        SetTimer(*(HWND *)((int)this + 0x610),1,in_stack_00000020,(TIMERPROC)0x0);
        *(undefined4 *)((int)this + 0x47c) = in_stack_00000024;
      }
    }
    local_4._0_1_ = 1;
    AthenaString_dtor(&local_44);
    local_4 = (uint)local_4._1_3_ << 8;
    AthenaString_dtor(local_28);
  }
  local_4 = 0xffffffff;
  AthenaString_dtor((undefined4 *)&stack0x00000004);
  ExceptionList = pvStack_c;
  return;
}

