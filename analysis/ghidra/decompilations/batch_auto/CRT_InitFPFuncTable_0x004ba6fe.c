
void CRT_InitFPFuncTable(void)

{
  PTR_CRT_amsg_exit_004fc810 = __cfltcvt;
  PTR_CRT_amsg_exit_004fc814 = &LAB_004bddf9;
  PTR_CRT_amsg_exit_004fc818 = __fassign;
  PTR_CRT_amsg_exit_004fc81c = __forcdecpt;
  PTR_CRT_amsg_exit_004fc820 = &LAB_004bde44;
  PTR_CRT_amsg_exit_004fc824 = __cfltcvt;
  return;
}

