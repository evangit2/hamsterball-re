
int __cdecl
MeshArchive_ctor(undefined4 *param_1,uint param_2,int param_3,undefined *param_4,int param_5,
                int param_6)

{
  int in_EAX;
  undefined4 *puVar1;
  void *pvVar2;
  int iVar3;
  uint uVar4;
  int *unaff_EBX;
  int *piVar5;
  int local_4;
  
  if (in_EAX == 0) {
    local_4 = -1;
  }
  else {
    local_4 = (*(code *)param_4)();
  }
  piVar5 = unaff_EBX;
  for (iVar3 = 0x4a; iVar3 != 0; iVar3 = iVar3 + -1) {
    *piVar5 = 0;
    piVar5 = piVar5 + 1;
  }
  unaff_EBX[0x46] = param_3;
  unaff_EBX[0x47] = (int)param_4;
  unaff_EBX[0x48] = param_5;
  *unaff_EBX = in_EAX;
  unaff_EBX[0x49] = param_6;
  puVar1 = StreamWriter_Create();
  unaff_EBX[6] = (int)puVar1;
  if (param_1 != (undefined4 *)0x0) {
    puVar1 = (undefined4 *)StreamWriter_ReserveSpace((int)puVar1,param_2);
    for (uVar4 = param_2 >> 2; uVar4 != 0; uVar4 = uVar4 - 1) {
      *puVar1 = *param_1;
      param_1 = param_1 + 1;
      puVar1 = puVar1 + 1;
    }
    for (uVar4 = param_2 & 3; uVar4 != 0; uVar4 = uVar4 - 1) {
      *(undefined1 *)puVar1 = *(undefined1 *)param_1;
      param_1 = (undefined4 *)((int)param_1 + 1);
      puVar1 = (undefined4 *)((int)puVar1 + 1);
    }
    StreamWriter_CommitSize(unaff_EBX[6],param_2);
  }
  if (local_4 != -1) {
    unaff_EBX[1] = 1;
  }
  unaff_EBX[7] = 1;
  pvVar2 = _calloc(1,0x20);
  unaff_EBX[0xc] = (int)pvVar2;
  pvVar2 = _calloc(unaff_EBX[7],0x10);
  unaff_EBX[0xd] = (int)pvVar2;
  iVar3 = WebClient_InitResponse(0xffffffff);
  unaff_EBX[0x18] = iVar3;
  iVar3 = MeshArchive_LoadSubmesh
                    (unaff_EBX,(undefined4 *)unaff_EBX[0xc],(int *)unaff_EBX[0xd],(int *)0x0);
  if (iVar3 < 0) {
    *unaff_EBX = 0;
    MeshArchive_dtor(unaff_EBX);
    return iVar3;
  }
  if (unaff_EBX[0x10] < 1) {
    unaff_EBX[0x10] = 1;
  }
  return iVar3;
}

