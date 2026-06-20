
undefined4 StrNEq(char *param_1,char *param_2,short param_3)

{
  char cVar1;
  char cVar2;
  
  if ((param_2 != (char *)0x0) && (param_1 != (char *)0x0)) {
    do {
      if (param_3 == 0) {
        return 1;
      }
      cVar1 = *param_2;
      if (cVar1 == '\0') {
        return 0;
      }
      cVar2 = *param_1;
      if (cVar2 == '\0') {
        return 0;
      }
      param_1 = param_1 + 1;
      param_2 = param_2 + 1;
      param_3 = param_3 + -1;
    } while (cVar1 == cVar2);
  }
  return 0;
}

