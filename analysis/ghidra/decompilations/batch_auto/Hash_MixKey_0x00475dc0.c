
uint __fastcall Hash_MixKey(undefined4 param_1,undefined4 param_2,uint param_3,byte param_4)

{
  return (param_3 << 0xd | CONCAT22((short)((uint)param_2 >> 0x10),(ushort)param_3 >> 3)) ^
         (uint)(byte)(&DAT_004f7534)[param_3 & 0xff ^ (uint)param_4];
}

