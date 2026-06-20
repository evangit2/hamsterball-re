
void __thiscall FileHandle_Open(void *this,char *param_1)

{
  int iVar1;
  
  if (*(int *)((int)this + 0x10) != 0) {
    __close(*(int *)((int)this + 0x10));
  }
  _free(*(void **)((int)this + 8));
  *(undefined4 *)((int)this + 8) = 0;
  iVar1 = FID_conflict___open(param_1,0x8000);
  *(int *)((int)this + 0x10) = iVar1;
  *(undefined1 *)((int)this + 4) = 1;
  *(undefined1 *)((int)this + 0x14) = 0;
  if (iVar1 == -1) {
    *(undefined1 *)((int)this + 0x14) = 1;
  }
  return;
}

