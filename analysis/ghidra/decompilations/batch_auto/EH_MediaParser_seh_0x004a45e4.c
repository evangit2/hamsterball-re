
void __cdecl EH_MediaParser_seh(int param_1,undefined4 *param_2)

{
  undefined1 local_54 [80];
  
  Fmt_HexEncode(local_54,param_1,param_2);
  seh_filter_invoke(param_1,local_54);
  return;
}

