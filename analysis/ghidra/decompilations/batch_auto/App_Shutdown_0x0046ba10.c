
void __fastcall App_Shutdown(int *param_1)

{
  *(undefined1 *)((int)param_1 + 0x159) = 1;
                    /* WARNING: Could not recover jumptable at 0x0046ba19. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*param_1 + 8))();
  return;
}

