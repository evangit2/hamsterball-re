
bool __thiscall StreamReader_OpenFile(void *this,char *param_1)

{
  int iVar1;
  
  iVar1 = FID_conflict___open(param_1,0x8000);
  *(int *)((int)this + 4) = iVar1;
  return iVar1 != -1;
}

