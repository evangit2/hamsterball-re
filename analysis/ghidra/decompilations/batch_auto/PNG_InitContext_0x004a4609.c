
int * __cdecl
PNG_InitContext(char *param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  int *piVar1;
  int iVar2;
  void *pvVar3;
  void *unaff_ESI;
  char *pcVar4;
  
  piVar1 = Zlib_AllocState(1);
  if (piVar1 == (int *)0x0) {
    return (int *)0x0;
  }
  iVar2 = __setjmp3(piVar1,0,unaff_ESI,piVar1);
  if (iVar2 != 0) {
    Zlib_FreeIf((int)piVar1,(void *)piVar1[0x27]);
    CRT_FreeIfNotNull(piVar1);
    return (int *)0x0;
  }
  Stream_SetPositions((int)piVar1,param_2,param_3,param_4);
  if ((param_1 == (char *)0x0) || (*param_1 != '1')) {
    longjmp_with_cleanup(piVar1,"Incompatible libpng version in application and library");
  }
  piVar1[0x28] = 0x2000;
  pvVar3 = Malloc_OrLongjmp(piVar1,0x2000);
  piVar1[0x27] = (int)pvVar3;
  piVar1[0x21] = (int)PNG_AllocZeroed;
  piVar1[0x22] = (int)Zlib_FreeIf;
  piVar1[0x23] = (int)piVar1;
  iVar2 = Zlib_InflateInit15((int)(piVar1 + 0x19),"1.1.3",0x38);
  if (iVar2 == -6) {
    pcVar4 = "zlib version error";
  }
  else if ((iVar2 == -4) || (iVar2 == -2)) {
    pcVar4 = "zlib memory error";
  }
  else {
    if (iVar2 == 0) goto LAB_004a46e8;
    pcVar4 = "Unknown zlib error";
  }
  longjmp_with_cleanup(piVar1,pcVar4);
LAB_004a46e8:
  piVar1[0x1c] = piVar1[0x27];
  piVar1[0x1d] = piVar1[0x28];
  PNG_SetWriteFn((int)piVar1,0,0);
  return piVar1;
}

