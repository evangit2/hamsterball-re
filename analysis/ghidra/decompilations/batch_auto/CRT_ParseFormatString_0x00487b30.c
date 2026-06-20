
void __thiscall CRT_ParseFormatString(void *this,char *param_1,void *param_2,int *param_3)

{
  int *piVar1;
  undefined8 *puVar2;
  uint uVar3;
  int iVar4;
  char *pcVar5;
  int iVar6;
  uint uVar7;
  int iVar8;
  double local_260;
  char *local_258;
  size_t local_254;
  int local_250;
  size_t local_24c;
  int local_248;
  size_t local_244;
  int local_240;
  int local_23c;
  int local_238;
  uint local_230;
  char local_22c;
  undefined8 local_228;
  char local_220 [32];
  byte local_200 [256];
  char local_100 [256];
  
  *(void **)((int)this + 4) = param_2;
  uVar7 = 0;
  local_230 = 0;
  pcVar5 = strchr(" +-#0",(int)*param_1);
  uVar3 = local_230;
  while (pcVar5 != (char *)0x0) {
    uVar7 = uVar7 | *(uint *)(&DAT_004db544 + (int)(pcVar5 + -0x4db55c) * 4);
    pcVar5 = param_1 + 1;
    param_1 = param_1 + 1;
    pcVar5 = strchr(" +-#0",(int)*pcVar5);
    uVar3 = uVar7;
  }
  local_230 = uVar3;
  if (*param_1 == '*') {
    piVar1 = (int *)*param_3;
    *param_3 = (int)(piVar1 + 1);
    local_238 = *piVar1;
    if (local_238 < 0) {
      local_238 = -local_238;
      local_230 = uVar7 | 4;
    }
    param_1 = param_1 + 1;
    iVar4 = local_238;
  }
  else {
    iVar8 = 0;
    local_238 = 0;
    iVar6 = _isdigit((int)*param_1);
    iVar4 = local_238;
    while (iVar6 != 0) {
      if (iVar8 < 999) {
        iVar8 = *param_1 + -0x30 + iVar8 * 10;
      }
      pcVar5 = param_1 + 1;
      param_1 = param_1 + 1;
      iVar6 = _isdigit((int)*pcVar5);
      iVar4 = iVar8;
    }
  }
  local_238 = iVar4;
  if (*param_1 == '.') {
    if (param_1[1] == '*') {
      piVar1 = (int *)*param_3;
      *param_3 = (int)(piVar1 + 1);
      param_1 = param_1 + 2;
      iVar4 = *piVar1;
    }
    else {
      iVar8 = 0;
      local_23c = 0;
      iVar6 = _isdigit((int)param_1[1]);
      iVar4 = local_23c;
      while (param_1 = param_1 + 1, iVar6 != 0) {
        if (iVar8 < 999) {
          iVar8 = *param_1 + -0x30 + iVar8 * 10;
        }
        iVar6 = _isdigit((int)param_1[1]);
        iVar4 = iVar8;
      }
    }
  }
  else {
    local_23c = -1;
    iVar4 = local_23c;
  }
  local_23c = iVar4;
  pcVar5 = strchr("hlL",(int)*param_1);
  if (pcVar5 == (char *)0x0) {
    local_22c = '\0';
  }
  else {
    local_22c = *param_1;
    param_1 = param_1 + 1;
  }
  switch(*param_1) {
  case 'E':
  case 'G':
  case 'e':
  case 'f':
  case 'g':
    puVar2 = (undefined8 *)*param_3;
    *param_3 = (int)(puVar2 + 1);
    local_228 = *puVar2;
    AthenaString_SprintfToBuffer((char *)local_200,&DAT_004db538);
    AthenaString_SprintfToBuffer(local_100,local_200);
    AthenaString_Assign(param_2,local_100);
    return;
  default:
    CRT_FormatSpecifier(this,&local_260,param_3,*param_1,local_220);
    local_238 = local_238 - (local_240 + local_244 + local_248 + local_24c + local_250 + local_254);
    uVar7 = local_230 & 4;
    if ((uVar7 == 0) && (0 < local_238)) {
      StdString_AppendCharN(param_2,0x20,local_238);
    }
  }
  if (0 < (int)local_254) {
    StdString_AppendN(param_2,local_220,local_254);
  }
  if (0 < local_250) {
    StdString_AppendCharN(param_2,0x30,local_250);
  }
  if (0 < (int)local_24c) {
    StdString_AppendN(param_2,local_258,local_24c);
  }
  if (0 < local_248) {
    StdString_AppendCharN(param_2,0x30,local_248);
  }
  if (0 < (int)local_244) {
    StdString_AppendN(param_2,local_258 + local_24c,local_244);
  }
  if (0 < local_240) {
    StdString_AppendCharN(param_2,0x30,local_240);
  }
  if ((uVar7 != 0) && (0 < local_238)) {
    StdString_AppendCharN(param_2,0x20,local_238);
  }
  return;
}

