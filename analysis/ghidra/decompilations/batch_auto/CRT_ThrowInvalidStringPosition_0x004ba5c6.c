
void CRT_ThrowInvalidStringPosition(void)

{
  int unaff_EBP;
  
  __security_init_cookie();
  Exception_AssignCStr((void *)(unaff_EBP + -0x28),"invalid string position");
  *(undefined4 *)(unaff_EBP + -4) = 0;
  AthenaList_Ctor((void *)(unaff_EBP + -0x50),unaff_EBP + -0x28);
  *(undefined ***)(unaff_EBP + -0x50) = &PTR_Exception_ScalarDeletingDtor_004e99a4;
                    /* WARNING: Subroutine does not return */
  __CxxThrowException_8(unaff_EBP + -0x50,&DAT_004f5af0);
}

