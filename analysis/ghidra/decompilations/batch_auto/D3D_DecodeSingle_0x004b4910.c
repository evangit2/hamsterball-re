
void __cdecl
D3D_DecodeSingle(int param_1,undefined4 param_2,int *param_3,undefined4 param_4,int param_5,
                int *param_6)

{
  (**(code **)(*(int *)(param_1 + 0x19c) + 0xc))(param_1,param_2,*param_3,param_5 + *param_6 * 4);
  *param_6 = *param_6 + 1;
  *param_3 = *param_3 + 1;
  return;
}

