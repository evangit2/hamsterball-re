
ulonglong Font_MeasureText(char *param_1)

{
  char *pcVar1;
  char cVar2;
  ulonglong uVar3;
  
  cVar2 = *param_1;
  while (cVar2 != '\0') {
    pcVar1 = param_1 + 1;
    param_1 = param_1 + 1;
    cVar2 = *pcVar1;
  }
  uVar3 = __ftol2();
  return uVar3;
}

