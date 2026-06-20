
undefined4 __cdecl
Mesh_FindWeldVertex16
          (float *param_1,float param_2,int param_3,int param_4,uint param_5,int param_6,int param_7
          ,int param_8,int *param_9,float *param_10)

{
  float *pfVar1;
  ushort *puVar2;
  float fVar3;
  float fVar4;
  int iVar5;
  float *pfVar6;
  float fVar7;
  
  fVar7 = param_1[1];
  fVar3 = *param_1;
  fVar4 = param_1[2];
  *param_10 = -NAN;
  pfVar1 = (float *)(param_3 + ((uint)((int)fVar7 + (int)fVar3 + (int)fVar4) % param_5) * 4);
  pfVar6 = (float *)*pfVar1;
  do {
    if (pfVar6 == (float *)0x0) {
      pfVar6 = (float *)(param_4 + *param_9 * 0x14);
      *param_9 = *param_9 + 1;
      *pfVar6 = *param_1;
      pfVar6[1] = param_1[1];
      pfVar6[2] = param_1[2];
      pfVar6[3] = param_2;
      pfVar6[4] = *pfVar1;
      *pfVar1 = (float)pfVar6;
      return 0;
    }
    if (((*pfVar6 == *param_1) && (pfVar6[1] == param_1[1])) &&
       (iVar5 = param_6, fVar7 = param_2, pfVar6[2] == param_1[2])) {
      do {
        fVar7 = *(float *)(iVar5 + (int)fVar7 * 4);
        if (fVar7 == -NAN) {
          *param_10 = pfVar6[3];
          return 1;
        }
        puVar2 = (ushort *)(param_8 + ((uint)fVar7 / 3) * 6);
        fVar3 = pfVar6[3];
      } while ((((float)(uint)*puVar2 != fVar3) && ((float)(uint)puVar2[1] != fVar3)) &&
              (iVar5 = param_7, (float)(uint)puVar2[2] != fVar3));
    }
    pfVar6 = (float *)pfVar6[4];
  } while( true );
}

