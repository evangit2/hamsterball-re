
void __fastcall LoaderGadget_OK(int *param_1)

{
  *(char **)(param_1[5] + 0x208) = "LoaderGadget::OK";
  if ((char)param_1[0xb56] != '\0') {
    (**(code **)(*(int *)param_1[5] + 0x44))();
  }
                    /* WARNING: Could not recover jumptable at 0x00474927. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*param_1 + 0x40))();
  return;
}

