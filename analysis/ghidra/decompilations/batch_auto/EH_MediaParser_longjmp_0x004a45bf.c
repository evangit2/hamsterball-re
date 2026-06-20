
void __cdecl EH_MediaParser_longjmp(int *param_1,undefined4 *param_2)

{
  undefined1 local_54 [80];
  
  Fmt_HexEncode(local_54,(int)param_1,param_2);
  longjmp_with_cleanup(param_1,local_54);
  return;
}

