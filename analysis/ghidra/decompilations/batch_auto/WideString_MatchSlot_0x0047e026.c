
char WideString_MatchSlot(ushort *param_1,uint param_2)

{
  char cVar1;
  
  if (*param_1 == param_2) {
    cVar1 = '\0';
  }
  else if (param_1[1] == param_2) {
    cVar1 = '\x01';
  }
  else {
    cVar1 = (param_1[2] != param_2) + '\x02';
  }
  return cVar1;
}

