
void __thiscall
CRT_FormatSpecifier(void *this,double *param_1,int *param_2,char param_3,undefined1 *param_4)

{
  char *pcVar1;
  char cVar2;
  undefined1 *puVar3;
  undefined4 *puVar4;
  double *pdVar5;
  char *pcVar6;
  int iVar7;
  
  *(undefined4 *)((int)param_1 + 0xc) = 0;
  *(undefined4 *)(param_1 + 2) = 0;
  *(undefined4 *)((int)param_1 + 0x14) = 0;
  *(undefined4 *)(param_1 + 3) = 0;
  *(undefined4 *)((int)param_1 + 0x1c) = 0;
  *(undefined4 *)(param_1 + 4) = 0;
  switch(param_3) {
  case '%':
    *param_4 = 0x25;
    *(int *)((int)param_1 + 0xc) = *(int *)((int)param_1 + 0xc) + 1;
    return;
  case '&':
  case '\'':
  case '(':
  case ')':
  case '*':
  case '+':
  case ',':
  case '-':
  case '.':
  case '/':
  case '0':
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
  case '9':
  case ':':
  case ';':
  case '<':
  case '=':
  case '>':
  case '?':
  case '@':
  case 'A':
  case 'B':
  case 'C':
  case 'D':
  case 'F':
  case 'H':
  case 'I':
  case 'J':
  case 'K':
  case 'L':
  case 'M':
  case 'N':
  case 'O':
  case 'P':
  case 'Q':
  case 'R':
  case 'S':
  case 'T':
  case 'U':
  case 'V':
  case 'W':
  case 'Y':
  case 'Z':
  case '[':
  case '\\':
  case ']':
  case '^':
  case '_':
  case '`':
  case 'a':
  case 'b':
  case 'h':
  case 'j':
  case 'k':
  case 'l':
  case 'm':
  case 'q':
  case 'r':
  case 't':
  case 'v':
  case 'w':
    break;
  case 'E':
  case 'G':
  case 'e':
  case 'f':
  case 'g':
    pdVar5 = (double *)*param_2;
    *param_2 = (int)(pdVar5 + 1);
    *param_1 = *pdVar5;
    if (((ulonglong)*param_1 & 0x8000000000000000) == 0) {
      if (((ulonglong)param_1[6] & 2) != 0) {
        param_4[*(int *)((int)param_1 + 0xc)] = 0x2b;
        goto LAB_00487a8f;
      }
      if (((ulonglong)param_1[6] & 1) != 0) {
        param_4[*(int *)((int)param_1 + 0xc)] = 0x20;
        goto LAB_00487a8f;
      }
    }
    else {
      param_4[*(int *)((int)param_1 + 0xc)] = 0x2d;
LAB_00487a8f:
      *(int *)((int)param_1 + 0xc) = *(int *)((int)param_1 + 0xc) + 1;
    }
    *(undefined1 **)(param_1 + 1) = param_4 + *(int *)((int)param_1 + 0xc);
    CRT_FormatFloat(param_1,param_3);
    break;
  case 'X':
  case 'o':
  case 'u':
  case 'x':
    puVar4 = (undefined4 *)*param_2;
    *param_2 = (int)(puVar4 + 1);
    *(undefined4 *)param_1 = *puVar4;
    if (*(char *)((int)param_1 + 0x34) == 'h') {
      *(uint *)param_1 = (uint)*(ushort *)param_1;
    }
    if ((((ulonglong)param_1[6] & 8) == 0) || (*(int *)param_1 == 0)) goto LAB_004879e6;
    param_4[*(int *)((int)param_1 + 0xc)] = 0x30;
    iVar7 = *(int *)((int)param_1 + 0xc) + 1;
    *(int *)((int)param_1 + 0xc) = iVar7;
    if ((param_3 != 'x') && (param_3 != 'X')) goto LAB_004879e6;
    param_4[iVar7] = param_3;
    goto LAB_004879e3;
  case 'c':
    puVar3 = (undefined1 *)*param_2;
    *param_2 = (int)(puVar3 + 4);
    param_4[*(int *)((int)param_1 + 0xc)] = *puVar3;
    *(int *)((int)param_1 + 0xc) = *(int *)((int)param_1 + 0xc) + 1;
    return;
  case 'd':
  case 'i':
    puVar4 = (undefined4 *)*param_2;
    *param_2 = (int)(puVar4 + 1);
    *(undefined4 *)param_1 = *puVar4;
    if (*(char *)((int)param_1 + 0x34) == 'h') {
      *(int *)param_1 = (int)*(short *)param_1;
    }
    if (*(int *)param_1 < 0) {
      param_4[*(int *)((int)param_1 + 0xc)] = 0x2d;
    }
    else if (((ulonglong)param_1[6] & 2) == 0) {
      if (((ulonglong)param_1[6] & 1) == 0) goto LAB_004879e6;
      param_4[*(int *)((int)param_1 + 0xc)] = 0x20;
    }
    else {
      param_4[*(int *)((int)param_1 + 0xc)] = 0x2b;
    }
LAB_004879e3:
    *(int *)((int)param_1 + 0xc) = *(int *)((int)param_1 + 0xc) + 1;
LAB_004879e6:
    *(undefined1 **)(param_1 + 1) = param_4 + *(int *)((int)param_1 + 0xc);
    CRT_FormatInteger((uint *)param_1,param_3);
    return;
  case 'n':
    if (*(char *)((int)param_1 + 0x34) != 'h') {
      *param_2 = *param_2 + 4;
      iVar7 = StdString_RecalcLen(*(int *)((int)this + 4));
      **(int **)(*param_2 + -4) = iVar7;
      return;
    }
    *param_2 = *param_2 + 4;
    iVar7 = StdString_RecalcLen(*(int *)((int)this + 4));
    **(undefined2 **)(*param_2 + -4) = (short)iVar7;
    return;
  case 'p':
    puVar4 = (undefined4 *)*param_2;
    *param_2 = (int)(puVar4 + 1);
    *(undefined4 *)param_1 = *puVar4;
    *(undefined1 **)(param_1 + 1) = param_4 + *(int *)((int)param_1 + 0xc);
    CRT_FormatInteger((uint *)param_1,'X');
    return;
  case 's':
    puVar4 = (undefined4 *)*param_2;
    *param_2 = (int)(puVar4 + 1);
    pcVar6 = (char *)*puVar4;
    *(char **)(param_1 + 1) = pcVar6;
    pcVar1 = pcVar6 + 1;
    do {
      cVar2 = *pcVar6;
      pcVar6 = pcVar6 + 1;
    } while (cVar2 != '\0');
    iVar7 = *(int *)((int)param_1 + 0x24);
    *(int *)((int)param_1 + 0x14) = (int)pcVar6 - (int)pcVar1;
    if ((-1 < iVar7) && (iVar7 < (int)pcVar6 - (int)pcVar1)) {
      *(int *)((int)param_1 + 0x14) = iVar7;
      return;
    }
    break;
  default:
    goto switchD_004878a7_default;
  }
switchD_004878a7_default:
  return;
}

