
byte * __cdecl Zlib_Inflate(byte *param_1,int param_2)

{
  byte bVar1;
  undefined4 uVar2;
  byte *pbVar3;
  undefined4 *puVar4;
  byte *pbVar5;
  
  if (((param_1 == (byte *)0x0) ||
      (puVar4 = *(undefined4 **)(param_1 + 0x1c), puVar4 == (undefined4 *)0x0)) ||
     (*(int *)param_1 == 0)) {
LAB_004ad3fd:
    return (byte *)0xfffffffe;
  }
  pbVar3 = (byte *)0xfffffffb;
  pbVar5 = (byte *)0x0;
  if (param_2 == 4) {
    pbVar5 = pbVar3;
  }
LAB_004ad3f0:
  switch(*puVar4) {
  case 0:
    if (*(int *)(param_1 + 4) == 0) {
      return pbVar3;
    }
    *(int *)(param_1 + 8) = *(int *)(param_1 + 8) + 1;
    *(int *)(param_1 + 4) = *(int *)(param_1 + 4) + -1;
    *(uint *)(*(int *)(param_1 + 0x1c) + 4) = (uint)**(byte **)param_1;
    puVar4 = *(undefined4 **)(param_1 + 0x1c);
    uVar2 = puVar4[1];
    *(int *)param_1 = *(int *)param_1 + 1;
    if (((byte)uVar2 & 0xf) == 8) {
      if (((uint)puVar4[1] >> 4) + 8 <= (uint)puVar4[4]) {
        *puVar4 = 1;
        pbVar3 = pbVar5;
        goto switchD_004ad1fd_caseD_1;
      }
      *puVar4 = 0xd;
      *(char **)(param_1 + 0x18) = "invalid window size";
    }
    else {
      *puVar4 = 0xd;
      *(char **)(param_1 + 0x18) = "unknown compression method";
    }
    goto LAB_004ad3e3;
  case 1:
switchD_004ad1fd_caseD_1:
    if (*(int *)(param_1 + 4) == 0) {
      return pbVar3;
    }
    puVar4 = *(undefined4 **)(param_1 + 0x1c);
    *(int *)(param_1 + 8) = *(int *)(param_1 + 8) + 1;
    *(int *)(param_1 + 4) = *(int *)(param_1 + 4) + -1;
    bVar1 = **(byte **)param_1;
    *(byte **)param_1 = *(byte **)param_1 + 1;
    if ((puVar4[1] * 0x100 + (uint)bVar1) % 0x1f != 0) {
      *puVar4 = 0xd;
      *(char **)(param_1 + 0x18) = "incorrect header check";
      goto LAB_004ad3e3;
    }
    if ((bVar1 & 0x20) != 0) {
      **(undefined4 **)(param_1 + 0x1c) = 2;
      pbVar3 = pbVar5;
      goto switchD_004ad1fd_caseD_2;
    }
    *puVar4 = 7;
    pbVar3 = pbVar5;
    break;
  case 2:
switchD_004ad1fd_caseD_2:
    if (*(int *)(param_1 + 4) == 0) {
      return pbVar3;
    }
    *(int *)(param_1 + 8) = *(int *)(param_1 + 8) + 1;
    *(int *)(param_1 + 4) = *(int *)(param_1 + 4) + -1;
    *(uint *)(*(int *)(param_1 + 0x1c) + 8) = (uint)**(byte **)param_1 << 0x18;
    *(int *)param_1 = *(int *)param_1 + 1;
    **(undefined4 **)(param_1 + 0x1c) = 3;
    pbVar3 = pbVar5;
  case 3:
    goto switchD_004ad1fd_caseD_3;
  case 4:
    goto switchD_004ad1fd_caseD_4;
  case 5:
    goto switchD_004ad1fd_caseD_5;
  case 6:
    **(undefined4 **)(param_1 + 0x1c) = 0xd;
    *(char **)(param_1 + 0x18) = "need dictionary";
    *(undefined4 *)(*(int *)(param_1 + 0x1c) + 4) = 0;
    return (byte *)0xfffffffe;
  case 7:
    pbVar3 = (byte *)Inflate_Process(*(uint **)(*(int *)(param_1 + 0x1c) + 0x14),param_1,pbVar3);
    if (pbVar3 == (byte *)0xfffffffd) {
      **(undefined4 **)(param_1 + 0x1c) = 0xd;
      *(undefined4 *)(*(int *)(param_1 + 0x1c) + 4) = 0;
      pbVar3 = (byte *)0xfffffffd;
    }
    else {
      if (pbVar3 == (byte *)0x0) {
        pbVar3 = pbVar5;
      }
      if (pbVar3 != (byte *)0x1) {
        return pbVar3;
      }
      Sound_DecodeFrame(*(int **)(*(int *)(param_1 + 0x1c) + 0x14),(int)param_1,
                        (int *)(*(int *)(param_1 + 0x1c) + 4));
      puVar4 = *(undefined4 **)(param_1 + 0x1c);
      if (puVar4[3] == 0) {
        *puVar4 = 8;
        pbVar3 = pbVar5;
        goto switchD_004ad1fd_caseD_8;
      }
      *puVar4 = 0xc;
      pbVar3 = pbVar5;
    }
    break;
  case 8:
switchD_004ad1fd_caseD_8:
    if (*(int *)(param_1 + 4) == 0) {
      return pbVar3;
    }
    *(int *)(param_1 + 8) = *(int *)(param_1 + 8) + 1;
    *(int *)(param_1 + 4) = *(int *)(param_1 + 4) + -1;
    *(uint *)(*(int *)(param_1 + 0x1c) + 8) = (uint)**(byte **)param_1 << 0x18;
    *(int *)param_1 = *(int *)param_1 + 1;
    **(undefined4 **)(param_1 + 0x1c) = 9;
    pbVar3 = pbVar5;
  case 9:
    if (*(int *)(param_1 + 4) == 0) {
      return pbVar3;
    }
    *(int *)(param_1 + 8) = *(int *)(param_1 + 8) + 1;
    *(int *)(param_1 + 4) = *(int *)(param_1 + 4) + -1;
    *(int *)(*(int *)(param_1 + 0x1c) + 8) =
         *(int *)(*(int *)(param_1 + 0x1c) + 8) + (uint)**(byte **)param_1 * 0x10000;
    *(int *)param_1 = *(int *)param_1 + 1;
    **(undefined4 **)(param_1 + 0x1c) = 10;
    pbVar3 = pbVar5;
  case 10:
    goto switchD_004ad1fd_caseD_a;
  case 0xb:
    goto switchD_004ad1fd_caseD_b;
  case 0xc:
    goto LAB_004ad3fd;
  case 0xd:
    return (byte *)0xfffffffd;
  default:
    goto LAB_004ad3fd;
  }
LAB_004ad3ed:
  puVar4 = *(undefined4 **)(param_1 + 0x1c);
  goto LAB_004ad3f0;
switchD_004ad1fd_caseD_a:
  if (*(int *)(param_1 + 4) == 0) {
    return pbVar3;
  }
  *(int *)(param_1 + 8) = *(int *)(param_1 + 8) + 1;
  *(int *)(param_1 + 4) = *(int *)(param_1 + 4) + -1;
  *(int *)(*(int *)(param_1 + 0x1c) + 8) =
       *(int *)(*(int *)(param_1 + 0x1c) + 8) + (uint)**(byte **)param_1 * 0x100;
  *(int *)param_1 = *(int *)param_1 + 1;
  **(undefined4 **)(param_1 + 0x1c) = 0xb;
  pbVar3 = pbVar5;
switchD_004ad1fd_caseD_b:
  if (*(int *)(param_1 + 4) == 0) {
    return pbVar3;
  }
  *(int *)(param_1 + 8) = *(int *)(param_1 + 8) + 1;
  *(int *)(param_1 + 4) = *(int *)(param_1 + 4) + -1;
  *(int *)(*(int *)(param_1 + 0x1c) + 8) =
       *(int *)(*(int *)(param_1 + 0x1c) + 8) + (uint)**(byte **)param_1;
  puVar4 = *(undefined4 **)(param_1 + 0x1c);
  *(int *)param_1 = *(int *)param_1 + 1;
  if (puVar4[1] == puVar4[2]) {
    **(undefined4 **)(param_1 + 0x1c) = 0xc;
LAB_004ad3fd:
    return (byte *)0x1;
  }
  *puVar4 = 0xd;
  *(char **)(param_1 + 0x18) = "incorrect data check";
LAB_004ad3e3:
  *(undefined4 *)(*(int *)(param_1 + 0x1c) + 4) = 5;
  pbVar3 = pbVar5;
  goto LAB_004ad3ed;
switchD_004ad1fd_caseD_3:
  if (*(int *)(param_1 + 4) == 0) {
    return pbVar3;
  }
  *(int *)(param_1 + 8) = *(int *)(param_1 + 8) + 1;
  *(int *)(param_1 + 4) = *(int *)(param_1 + 4) + -1;
  *(int *)(*(int *)(param_1 + 0x1c) + 8) =
       *(int *)(*(int *)(param_1 + 0x1c) + 8) + (uint)**(byte **)param_1 * 0x10000;
  *(int *)param_1 = *(int *)param_1 + 1;
  **(undefined4 **)(param_1 + 0x1c) = 4;
  pbVar3 = pbVar5;
switchD_004ad1fd_caseD_4:
  if (*(int *)(param_1 + 4) == 0) {
    return pbVar3;
  }
  *(int *)(param_1 + 8) = *(int *)(param_1 + 8) + 1;
  *(int *)(param_1 + 4) = *(int *)(param_1 + 4) + -1;
  *(int *)(*(int *)(param_1 + 0x1c) + 8) =
       *(int *)(*(int *)(param_1 + 0x1c) + 8) + (uint)**(byte **)param_1 * 0x100;
  *(int *)param_1 = *(int *)param_1 + 1;
  **(undefined4 **)(param_1 + 0x1c) = 5;
  pbVar3 = pbVar5;
switchD_004ad1fd_caseD_5:
  if (*(int *)(param_1 + 4) != 0) {
    *(int *)(param_1 + 8) = *(int *)(param_1 + 8) + 1;
    *(int *)(param_1 + 4) = *(int *)(param_1 + 4) + -1;
    *(int *)(*(int *)(param_1 + 0x1c) + 8) =
         *(int *)(*(int *)(param_1 + 0x1c) + 8) + (uint)**(byte **)param_1;
    *(int *)param_1 = *(int *)param_1 + 1;
    *(undefined4 *)(param_1 + 0x30) = (*(undefined4 **)(param_1 + 0x1c))[2];
    **(undefined4 **)(param_1 + 0x1c) = 6;
    return (byte *)0x2;
  }
  return pbVar3;
}

