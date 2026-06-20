
int __cdecl
Inflate_BuildDynamicTrees
          (uint param_1,uint param_2,uint *param_3,int *param_4,int *param_5,int *param_6,
          int *param_7,int param_8,int param_9)

{
  uint *puVar1;
  int iVar2;
  uint local_8;
  
  local_8 = 0;
  puVar1 = (uint *)(**(code **)(param_9 + 0x20))(*(undefined4 *)(param_9 + 0x28),0x120,4);
  if (puVar1 == (uint *)0x0) {
    return -4;
  }
  iVar2 = Inflate_HuffmanBuild
                    (param_3,param_1,0x101,0x4e9568,0x4e95e8,param_6,param_8,&local_8,puVar1);
  if (iVar2 == 0) {
    if (*param_4 == 0) goto LAB_004b8517;
    iVar2 = Inflate_HuffmanBuild
                      (param_3 + param_1,param_2,0,0x4e9668,0x4e96e0,param_7,param_8,&local_8,puVar1
                      );
    if (iVar2 == 0) {
      if ((*param_5 != 0) || (param_1 < 0x102)) {
        iVar2 = 0;
        goto LAB_004b8523;
      }
LAB_004b84fb:
      *(char **)(param_9 + 0x18) = "empty distance tree with lengths";
    }
    else {
      if (iVar2 == -3) {
        *(char **)(param_9 + 0x18) = "oversubscribed distance tree";
        goto LAB_004b8523;
      }
      if (iVar2 != -5) {
        if (iVar2 == -4) goto LAB_004b8523;
        goto LAB_004b84fb;
      }
      *(char **)(param_9 + 0x18) = "incomplete distance tree";
    }
  }
  else {
    if (iVar2 == -3) {
      *(char **)(param_9 + 0x18) = "oversubscribed literal/length tree";
      goto LAB_004b8523;
    }
    if (iVar2 == -4) goto LAB_004b8523;
LAB_004b8517:
    *(char **)(param_9 + 0x18) = "incomplete literal/length tree";
  }
  iVar2 = -3;
LAB_004b8523:
  (**(code **)(param_9 + 0x24))(*(undefined4 *)(param_9 + 0x28),puVar1);
  return iVar2;
}

