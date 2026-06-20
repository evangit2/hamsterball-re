
uint VertexDecl_ParseFVF(uint param_1,undefined4 *param_2)

{
  uint uVar1;
  uint uVar2;
  int iVar3;
  undefined1 local_44;
  byte local_43;
  int local_3c;
  int local_34;
  int local_30;
  uint local_c;
  uint local_8;
  
  uVar1 = param_1;
  VertexDecl_Init(&local_44,param_1);
  uVar2 = param_1 & 0xe001;
  param_1 = 0;
  if ((uVar2 == 0) && ((uVar1 & 0xf00) < 0x801)) {
    iVar3 = 1;
    *param_2 = 0x20000000;
    if ((uVar1 & 0xe) != 0) {
      iVar3 = 2;
      param_2[1] = (((uVar1 & 0xe) != 4) - 1 & 0x10000) + 0x40020000;
    }
    if (local_c != 0) {
      if ((local_43 & 0x10) == 0) {
        if (local_c == 5) {
          return 0x8876086c;
        }
        param_2[iVar3] = (local_c - 1) * 0x10000 | 0x40000001;
      }
      else {
        if (1 < local_c) {
          param_2[iVar3] = (local_c - 2) * 0x10000 | 0x40000001;
          iVar3 = iVar3 + 1;
        }
        param_2[iVar3] = 0x40050002;
      }
      iVar3 = iVar3 + 1;
    }
    if (local_3c != 0) {
      param_2[iVar3] = 0x40020003;
      iVar3 = iVar3 + 1;
    }
    if ((uVar1 & 0x20) != 0) {
      param_2[iVar3] = 0x40000004;
      iVar3 = iVar3 + 1;
    }
    if (local_34 != 0) {
      param_2[iVar3] = 0x40040005;
      iVar3 = iVar3 + 1;
    }
    if (local_30 != 0) {
      param_2[iVar3] = 0x40040006;
      iVar3 = iVar3 + 1;
    }
    if ((local_8 != 0) && (uVar2 = 0, local_8 != 0)) {
      do {
        uVar1 = D3D_GetAdapterMode(&local_44,uVar2);
        param_2[iVar3] = ((uVar1 >> 2) - 1 | 0x4000) << 0x10 | uVar2 + 7;
        iVar3 = iVar3 + 1;
        uVar2 = uVar2 + 1;
      } while (uVar2 < local_8);
    }
    param_2[iVar3] = 0xffffffff;
  }
  else {
    param_1 = 0x8876086c;
  }
  return param_1;
}

