
void __cdecl Exception_ThrowAndLongjmp(int *param_1)

{
  (**(code **)(*param_1 + 8))(param_1);
                    /* WARNING: Subroutine does not return */
  _longjmp((int *)(*param_1 + 0x84),1);
}

