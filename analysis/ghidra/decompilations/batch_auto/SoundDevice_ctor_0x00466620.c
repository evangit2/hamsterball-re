
void * __thiscall SoundDevice_ctor(void *this,undefined4 param_1)

{
  undefined4 *puVar1;
  int iVar2;
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cd2c9;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  *(undefined ***)this = &PTR_SoundDevice_DeletingDtor_004d911c;
  AthenaList_Init((void *)((int)this + 4),0);
  local_4 = 0;
  AthenaList_Init((void *)((int)this + 0x41c),0);
  local_4 = CONCAT31(local_4._1_3_,1);
  *(undefined4 *)((int)this + 0x834) = param_1;
  *(undefined1 *)((int)this + 0x83c) = 1;
  *(undefined4 *)((int)this + 0x838) = 0x3f800000;
  *(undefined4 *)((int)this + 0x848) = 4;
  *(undefined4 *)((int)this + 0x840) = 0;
  *(undefined4 *)((int)this + 0x844) = 0xc5fa0000;
  iVar2 = 0x10;
  puVar1 = (undefined4 *)((int)this + 0x854);
  do {
    *puVar1 = 0;
    puVar1[1] = 0;
    iVar2 = iVar2 + -1;
    puVar1[2] = 0;
    puVar1 = puVar1 + 3;
  } while (iVar2 != 0);
  puVar1 = (undefined4 *)((int)this + 0x84c);
  *(undefined4 *)((int)this + 0x914) = 0;
  *(undefined4 *)((int)this + 0x918) = 0x45bb8000;
  *puVar1 = 0;
  iVar2 = DllEntryPoint(0,puVar1,0);
  if (iVar2 == 0) {
    (**(code **)(*(int *)*puVar1 + 0x18))
              ((int *)*puVar1,*(undefined4 *)(*(int *)((int)this + 0x834) + 8),2);
  }
  else {
    *puVar1 = 0;
  }
  AthenaList_Free((int)this + 4);
  AthenaList_Free((int)this + 0x41c);
  Level_ReadSoundVolume((int)this);
  ExceptionList = (void *)0x0;
  return this;
}

