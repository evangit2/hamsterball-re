
int Audio_BarkLookup(void)

{
  int in_EAX;
  int iVar1;
  
  iVar1 = 0;
  do {
    if (((int)(&DAT_004e63d8)[iVar1] <= in_EAX) && (in_EAX < (int)(&DAT_004e63dc)[iVar1])) break;
    if (((int)(&DAT_004e63dc)[iVar1] <= in_EAX) && (in_EAX < (int)(&DAT_004e63e0)[iVar1])) {
      iVar1 = iVar1 + 1;
      break;
    }
    if (((int)(&DAT_004e63e0)[iVar1] <= in_EAX) && (in_EAX < (int)(&DAT_004e63e4)[iVar1])) {
      iVar1 = iVar1 + 2;
      break;
    }
    if (((int)(&DAT_004e63e4)[iVar1] <= in_EAX) && (in_EAX < (int)(&DAT_004e63e8)[iVar1])) {
      iVar1 = iVar1 + 3;
      break;
    }
    if (((int)(&DAT_004e63e8)[iVar1] <= in_EAX) && (in_EAX < (int)(&DAT_004e63ec)[iVar1])) {
      iVar1 = iVar1 + 4;
      break;
    }
    if (((int)(&DAT_004e63ec)[iVar1] <= in_EAX) && (in_EAX < (int)(&DAT_004e63f0)[iVar1])) {
      iVar1 = iVar1 + 5;
      break;
    }
    if (((int)(&DAT_004e63f0)[iVar1] <= in_EAX) && (in_EAX < (int)(&DAT_004e63f4)[iVar1])) {
      iVar1 = iVar1 + 6;
      break;
    }
    if (((int)(&DAT_004e63f4)[iVar1] <= in_EAX) && (in_EAX < (int)(&DAT_004e63f8)[iVar1])) {
      iVar1 = iVar1 + 7;
      break;
    }
    if (((int)(&DAT_004e63f8)[iVar1] <= in_EAX) && (in_EAX < (int)(&DAT_004e63fc)[iVar1])) {
      iVar1 = iVar1 + 8;
      break;
    }
    iVar1 = iVar1 + 9;
  } while (iVar1 < 0x1b);
  if (iVar1 != 0x1b) {
    return ((in_EAX - (&DAT_004e63d8)[iVar1]) * 0x8000) /
           (int)((&DAT_004e63dc)[iVar1] - (&DAT_004e63d8)[iVar1]) + iVar1 * 0x8000;
  }
  return 0xd8000;
}

