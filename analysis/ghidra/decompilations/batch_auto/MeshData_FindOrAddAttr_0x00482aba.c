
void __thiscall MeshData_FindOrAddAttr(void *this,int param_1,char *param_2)

{
  int iVar1;
  
  iVar1 = SmallIntArray_Find(this,param_1);
  *param_2 = (char)iVar1;
  if ((char)iVar1 == '\0') {
    SmallIntArray_Push(this,param_1);
  }
  return;
}

