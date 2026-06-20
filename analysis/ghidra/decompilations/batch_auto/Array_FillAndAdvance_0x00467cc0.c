
undefined4 * Array_FillAndAdvance(undefined4 *param_1,int param_2,undefined4 *param_3)

{
  Array_FillDWords(param_1,param_2,param_3);
  return param_1 + param_2;
}

