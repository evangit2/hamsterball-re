
void __thiscall MeshData_FindOrAddShortAttr(void *this,short param_1,char *param_2)

{
  int iVar1;
  
  iVar1 = MeshAttribute_FindIndex(this,param_1);
  *param_2 = (char)iVar1;
  if ((char)iVar1 == '\0') {
    ShortArray_Push(this,param_1);
  }
  return;
}

