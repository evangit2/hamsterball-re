
void CRT_ThrowStringTooLong(void)

{
  int unaff_EBP;
  
  __security_init_cookie();
  Exception_AssignCStr((void *)(unaff_EBP + -0x28),"string too long");
  *(undefined4 *)(unaff_EBP + -4) = 0;
  AthenaList_Ctor((void *)(unaff_EBP + -0x50),unaff_EBP + -0x28);
  *(undefined ***)(unaff_EBP + -0x50) = &PTR_Exception_DeletingDtor_004e9998;
                    /* WARNING: Subroutine does not return */
  __CxxThrowException_8(unaff_EBP + -0x50,&DAT_004f44a4);
}

