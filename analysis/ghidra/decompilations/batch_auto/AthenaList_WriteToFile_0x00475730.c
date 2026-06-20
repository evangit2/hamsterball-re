
void __thiscall AthenaList_WriteToFile(void *this,char *param_1)

{
  int iVar1;
  int iVar2;
  int iVar3;
  void *_Buf;
  
  iVar2 = CRT_OpenFileExclusive(param_1,0x180);
  __close(iVar2);
  iVar2 = FID_conflict___open(param_1,0x8002);
  iVar3 = AthenaList_NextIndex((int)this + 8);
  *(undefined4 *)((int)this + iVar3 * 4 + 0x10) = 0;
  if (*(int *)((int)this + 0xc) < 1) {
    _Buf = (void *)0x0;
  }
  else {
    _Buf = (void *)**(undefined4 **)((int)this + 0x414);
    *(undefined4 *)((int)this + iVar3 * 4 + 0x10) = 1;
  }
  while (_Buf != (void *)0x0) {
    __write(iVar2,(void *)((int)_Buf + 0x100),4);
    __write(iVar2,_Buf,0x100);
    iVar1 = *(int *)((int)this + iVar3 * 4 + 0x10);
    if (*(int *)((int)this + 0xc) <= iVar1) break;
    _Buf = *(void **)(*(int *)((int)this + 0x414) + iVar1 * 4);
    *(int *)((int)this + iVar3 * 4 + 0x10) = iVar1 + 1;
  }
  __close(iVar2);
  return;
}

