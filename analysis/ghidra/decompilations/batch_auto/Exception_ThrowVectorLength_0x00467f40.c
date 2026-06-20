
void Exception_ThrowVectorLength(void)

{
  char *pcVar1;
  undefined1 local_50 [4];
  undefined1 local_4c;
  undefined4 local_3c;
  undefined4 local_38;
  undefined **local_34 [10];
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cd338;
  pvStack_c = ExceptionList;
  pcVar1 = "?vector<T> too long";
  local_38 = 0xf;
  local_3c = 0;
  local_4c = 0;
  do {
    pcVar1 = pcVar1 + 1;
  } while (*pcVar1 != '\0');
  ExceptionList = &pvStack_c;
  StdString_Assign(local_50,(undefined4 *)"vector<T> too long",(uint)(pcVar1 + -0x4d9148));
  local_4 = 0;
  AthenaList_Ctor(local_34,local_50);
  local_34[0] = &PTR_Exception_DeletingDtor_004e9998;
                    /* WARNING: Subroutine does not return */
  __CxxThrowException_8(local_34,&DAT_004f44a4);
}

