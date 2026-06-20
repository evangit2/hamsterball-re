
undefined4 * __fastcall MWParser_ReadTag(int param_1)

{
  undefined4 *puVar1;
  char cVar2;
  int iVar3;
  int iVar4;
  char *pcVar5;
  char *pcVar6;
  char *pcVar7;
  size_t sVar8;
  char local_60e;
  char local_60d;
  undefined4 *local_60c;
  int local_608;
  size_t local_604;
  char local_600 [256];
  char local_500 [256];
  char local_400 [1024];
  
  cVar2 = *(char *)(param_1 + 0x14);
  do {
    if (cVar2 != '\0') {
      return (undefined4 *)0x0;
    }
    if (*(char *)(param_1 + 4) == '\0') {
      local_60d = **(char **)(param_1 + 0xc);
      *(char **)(param_1 + 0xc) = *(char **)(param_1 + 0xc) + 1;
      if (local_60d == '\0') goto LAB_0046965f;
    }
    else {
      iVar3 = __read(*(int *)(param_1 + 0x10),&local_60d,1);
      if (iVar3 < 1) {
LAB_0046965f:
        *(undefined1 *)(param_1 + 0x14) = 1;
        return (undefined4 *)0x0;
      }
    }
    cVar2 = *(char *)(param_1 + 0x14);
  } while (local_60d != '<');
  iVar3 = 0;
  if (cVar2 == '\0') goto LAB_0046967b;
  cVar2 = '\0';
  while( true ) {
    local_400[iVar3] = cVar2;
    local_400[iVar3 + 1] = '\0';
    if (cVar2 == '>') break;
    iVar3 = iVar3 + 1;
    if (*(char *)(param_1 + 0x14) != '\0') {
      return (undefined4 *)0x0;
    }
LAB_0046967b:
    if (*(char *)(param_1 + 4) == '\0') {
      local_60e = **(char **)(param_1 + 0xc);
      *(char **)(param_1 + 0xc) = *(char **)(param_1 + 0xc) + 1;
      cVar2 = local_60e;
      if (local_60e == '\0') {
        *(undefined1 *)(param_1 + 0x14) = 1;
      }
    }
    else {
      iVar4 = __read(*(int *)(param_1 + 0x10),&local_60e,1);
      cVar2 = local_60e;
      if (iVar4 < 1) {
        local_60e = '\0';
        *(undefined1 *)(param_1 + 0x14) = 1;
        cVar2 = '\0';
      }
    }
  }
  local_60c = operator_new(0xc);
  if (local_60c == (undefined4 *)0x0) {
    local_60c = (undefined4 *)0x0;
  }
  else {
    *local_60c = &PTR_StreamReader_DeletingDtor_004d91b8;
    local_60c[1] = 0;
    local_60c[2] = 0;
  }
  pcVar5 = local_400;
  local_400[iVar3] = '\0';
  do {
    cVar2 = *pcVar5;
    pcVar5 = pcVar5 + 1;
  } while (cVar2 != '\0');
  pcVar6 = operator_new((uint)(pcVar5 + (1 - (int)(local_400 + 1))));
  local_60c[1] = pcVar6;
  pcVar5 = local_400;
  do {
    cVar2 = *pcVar5;
    pcVar5 = pcVar5 + 1;
    *pcVar6 = cVar2;
    pcVar6 = pcVar6 + 1;
  } while (cVar2 != '\0');
  local_604 = 0x400;
  iVar3 = 0;
  pcVar5 = _malloc(0x400);
  local_60d = '\0';
  AthenaString_SprintfToBuffer(local_500,&DAT_004d91c0);
  local_608 = 0;
  sVar8 = 0x400;
  do {
    if ((int)(sVar8 - 1) <= iVar3) {
      local_604 = sVar8 + 0x400;
      pcVar5 = _realloc(pcVar5,local_604);
    }
    if (*(char *)(param_1 + 0x14) == '\0') {
      if (*(char *)(param_1 + 4) == '\0') {
        local_60e = **(char **)(param_1 + 0xc);
        *(char **)(param_1 + 0xc) = *(char **)(param_1 + 0xc) + 1;
        if (local_60e == '\0') {
          *(undefined1 *)(param_1 + 0x14) = 1;
        }
      }
      else {
        iVar4 = __read(*(int *)(param_1 + 0x10),&local_60e,1);
        if (iVar4 < 1) {
          local_60e = '\0';
          *(undefined1 *)(param_1 + 0x14) = 1;
        }
      }
      cVar2 = local_60e;
      if (local_60e == '<') {
        local_60d = '\x01';
        local_608 = 0;
        goto LAB_00469898;
      }
      if (local_60e == '>') {
        local_60d = '\0';
        local_600[local_608] = '\0';
        iVar4 = __stricmp(local_600,local_500);
        if (iVar4 != 0) goto LAB_00469898;
        do {
          iVar4 = iVar3 + -1;
          iVar3 = iVar3 + -1;
        } while (pcVar5[iVar4] != '<');
        pcVar5[iVar3] = '\0';
LAB_0046984d:
        pcVar6 = pcVar5;
        do {
          cVar2 = *pcVar6;
          pcVar6 = pcVar6 + 1;
        } while (cVar2 != '\0');
        pcVar7 = operator_new((uint)(pcVar6 + (1 - (int)(pcVar5 + 1))));
        puVar1 = local_60c;
        local_60c[2] = pcVar7;
        pcVar6 = pcVar5;
        do {
          cVar2 = *pcVar6;
          pcVar6 = pcVar6 + 1;
          *pcVar7 = cVar2;
          pcVar7 = pcVar7 + 1;
        } while (cVar2 != '\0');
        _free(pcVar5);
        return puVar1;
      }
      if (('\x1f' < local_60e) || (local_60e == '\0')) goto LAB_00469898;
    }
    else {
      cVar2 = '\0';
LAB_00469898:
      pcVar5[iVar3] = cVar2;
      iVar3 = iVar3 + 1;
      if ((local_60d != '\0') && (cVar2 != '<')) {
        local_600[local_608] = cVar2;
        local_608 = local_608 + 1;
      }
    }
    sVar8 = local_604;
    if (*(char *)(param_1 + 0x14) != '\0') goto LAB_0046984d;
  } while( true );
}

