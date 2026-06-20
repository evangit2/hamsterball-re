
void __cdecl
Mesh_FindBestDuplicateVertex
          (uint param_1,uint param_2,int param_3,uint *param_4,uint param_5,uint param_6,int param_7
          ,int param_8)

{
  uint *puVar1;
  uint *puVar2;
  uint *puVar3;
  uint *puVar4;
  float10 fVar5;
  float10 fVar6;
  uint *local_8;
  
  puVar1 = param_4 + param_1 % param_5;
  param_5 = param_6;
  puVar2 = (uint *)*puVar1;
  local_8 = (uint *)0x0;
  do {
    puVar4 = puVar2;
    if (puVar4 == (uint *)0x0) {
LAB_0047fe4f:
      if (puVar4 != (uint *)0x0) {
        puVar2 = (uint *)puVar4[4];
        param_4 = puVar4;
        while (puVar3 = puVar2, puVar3 != (uint *)0x0) {
          if ((puVar3[1] == param_2) && (*puVar3 == param_1)) {
            fVar5 = Mesh_ComputeTriCrossProduct
                              (*puVar4,puVar4[1],puVar4[2],param_2,param_1,param_3,param_7,param_8);
            fVar6 = Mesh_ComputeTriCrossProduct
                              (*puVar3,puVar3[1],puVar3[2],param_2,param_1,param_3,param_7,param_8);
            if ((float10)(float)fVar5 < fVar6) {
              local_8 = param_4;
              param_5 = puVar3[3];
              puVar4 = puVar3;
            }
          }
          param_4 = puVar3;
          puVar2 = (uint *)puVar3[4];
        }
      }
      if (param_5 != param_6) {
        if (local_8 == (uint *)0x0) {
          *puVar1 = puVar4[4];
        }
        else {
          local_8[4] = puVar4[4];
        }
      }
      return;
    }
    if ((puVar4[1] == param_2) && (*puVar4 == param_1)) {
      param_5 = puVar4[3];
      goto LAB_0047fe4f;
    }
    puVar2 = (uint *)puVar4[4];
    local_8 = puVar4;
  } while( true );
}

