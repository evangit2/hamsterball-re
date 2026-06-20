
/* WARNING: Variable defined which should be unmapped: param_1 */
/* WARNING: Variable defined which should be unmapped: param_3 */
/* WARNING: Variable defined which should be unmapped: param_2 */

void * App_BuildDiagnosticTail(undefined4 param_1,undefined4 param_2,undefined4 param_3)

{
  undefined1 uVar1;
  char *unaff_EBX;
  undefined4 unaff_EBP;
  int *unaff_ESI;
  undefined1 uStack00000018;
  undefined4 uStack00000028;
  undefined1 *puStack0000002c;
  undefined1 *puStack00000030;
  void *in_stack_000000a4;
  undefined1 uStack000000ac;
  undefined1 uStack000000b0;
  void *in_stack_000000b4;
  undefined4 uVar2;
  char *pcVar3;
  
  puStack00000030 = &stack0xffffffe4;
  uStack000000b0 = 0x12;
  uVar1 = SUB41(unaff_EBX,0);
  uVar2 = unaff_EBP;
  pcVar3 = unaff_EBX;
  AthenaString_AssignCStr(&stack0xffffffe4,"DSOUND");
  uStack000000b0 = 0xc;
  AthenaString_WriteTag(&stack0x00000038,uVar2,pcVar3);
  uStack00000018 = 1;
  AthenaString_AssignCStr(&stack0x00000000,(char *)unaff_ESI[0x83]);
  puStack00000030 = &stack0xffffffe4;
  uStack000000b0 = 0x13;
  uVar2 = unaff_EBP;
  pcVar3 = unaff_EBX;
  AthenaString_AssignCStr(&stack0xffffffe4,"CURRENTOBJECT");
  uStack000000b0 = 0xc;
  AthenaString_WriteTag(&stack0x00000038,uVar2,pcVar3);
  uStack00000018 = 1;
  AthenaString_AssignCStr(&stack0x00000000,(char *)unaff_ESI[0x84]);
  puStack00000030 = &stack0xffffffe4;
  uStack000000b0 = 0x14;
  uVar2 = unaff_EBP;
  pcVar3 = unaff_EBX;
  AthenaString_AssignCStr(&stack0xffffffe4,"CURRENTOPERATION");
  uStack000000b0 = 0xc;
  AthenaString_WriteTag(&stack0x00000038,uVar2,pcVar3);
  (**(code **)(*unaff_ESI + 0x9c))();
  puStack0000002c = &stack0xffffffe0;
  uStack000000ac = 0x15;
  AthenaString_AssignCStr(&stack0xffffffe0,"EXTENDED_INFO");
  uStack000000ac = 0xc;
  AthenaString_WriteTag(&stack0x00000034,unaff_EBP,unaff_EBX);
  AthenaString_CopyCtor(in_stack_000000b4,(int)&stack0x00000034);
  uStack00000028 = 1;
  uStack000000ac = 10;
  AthenaString_dtor((undefined4 *)&stack0x00000088);
  uStack000000ac = 4;
  AthenaString_dtor((undefined4 *)&stack0x00000050);
  uStack000000ac = 1;
  AthenaString_dtor((undefined4 *)&stack0x0000006c);
  uStack000000ac = uVar1;
  AthenaString_dtor((undefined4 *)&stack0x00000034);
  ExceptionList = in_stack_000000a4;
  return in_stack_000000b4;
}

