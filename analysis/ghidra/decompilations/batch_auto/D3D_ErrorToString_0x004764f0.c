
undefined4 D3D_ErrorToString(int param_1,char *param_2,int param_3)

{
  int iVar1;
  char *pcVar2;
  
  if (param_2 == (char *)0x0) {
    return 0;
  }
  if (param_3 == 0) {
    return 0;
  }
  if (param_1 < -0x7789f799) {
    if (param_1 != -0x7789f79a) {
      if (param_1 < -0x7789f7e3) {
        if (param_1 != -0x7789f7e4) {
          if (param_1 < -0x7789f7e7) {
            if (((param_1 != -0x7789f7e8) && (param_1 != -0x7fffbfff)) &&
               ((param_1 != -0x7fffbffb && ((param_1 != -0x7ff8fff2 && (param_1 != -0x7789fe84))))))
            goto LAB_0047673b;
          }
          else if ((param_1 != -0x7789f7e7) &&
                  ((param_1 != -0x7789f7e6 && (param_1 != -0x7789f7e5)))) goto LAB_0047673b;
        }
      }
      else if ((((param_1 != -0x7789f7e3) && (param_1 != -0x7789f7e2)) &&
               ((param_1 != -0x7789f7e1 &&
                (((param_1 != -0x7789f7df && (param_1 != -0x7789f7de)) && (param_1 != -0x7789f7da)))
                ))) && (param_1 != -0x7789f7d9)) goto LAB_0047673b;
    }
  }
  else if (param_1 < -0x7789f4ab) {
    if ((((param_1 != -0x7789f4ac) && (param_1 != -0x7789f799)) &&
        ((param_1 != -0x7789f798 &&
         (((param_1 != -0x7789f797 && (param_1 != -0x7789f796)) && (param_1 != -0x7789f795)))))) &&
       ((param_1 != -0x7789f794 && (param_1 != -0x7789f793)))) {
LAB_0047673b:
      pcVar2 = "Unrecognized error: 0x%0.8x";
      goto LAB_00476741;
    }
  }
  else if ((((((param_1 != -0x7789f4ab) && (param_1 != -0x7789f4aa)) && (param_1 != -0x7789f4a9)) &&
            ((param_1 != -0x7789f4a8 && (param_1 != -0x7789f4a7)))) && (param_1 != -0x7789f4a6)) &&
          (param_1 != 0)) goto LAB_0047673b;
  pcVar2 = &DAT_004d3ad8;
LAB_00476741:
  iVar1 = CRT_vsprintf(param_2,param_3,(byte *)pcVar2);
  if ((iVar1 < 0) && (param_3 != 0)) {
    param_2[param_3 + -1] = '\0';
  }
  return 0;
}

