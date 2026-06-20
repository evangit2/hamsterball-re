
void __thiscall RegKeyList_AppendStr(void *this,char *param_1,undefined4 param_2)

{
  char cVar1;
  undefined4 *puVar2;
  char *pcVar3;
  
  puVar2 = operator_new(8);
  pcVar3 = param_1;
  do {
    cVar1 = *pcVar3;
    pcVar3 = pcVar3 + 1;
  } while (cVar1 != '\0');
  pcVar3 = operator_new((uint)(pcVar3 + (1 - (int)(param_1 + 1))));
  *puVar2 = pcVar3;
  do {
    cVar1 = *param_1;
    param_1 = param_1 + 1;
    *pcVar3 = cVar1;
    pcVar3 = pcVar3 + 1;
  } while (cVar1 != '\0');
  puVar2[1] = param_2;
  AthenaList_Append((void *)((int)this + 0xc),(int)puVar2);
  return;
}

