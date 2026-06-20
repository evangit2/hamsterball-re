
undefined4 Huffman_StreamEnd(void)

{
  undefined4 *puVar1;
  undefined4 *in_EAX;
  
  if ((int)in_EAX[2] < 1) {
    puVar1 = (undefined4 *)in_EAX[3];
    in_EAX[1] = puVar1[2] + puVar1[1] + *(int *)*puVar1;
    in_EAX[2] = 0xffffffff;
    *in_EAX = 0;
    return 0xffffffff;
  }
  return 0;
}

