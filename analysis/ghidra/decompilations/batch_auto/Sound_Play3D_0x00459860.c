
void __thiscall Sound_Play3D(void *this,float param_1,float param_2,float param_3)

{
  Sound_CalculateDistanceAttenuation(*(void **)((int)this + 4),param_1,param_2,param_3);
  Sound_PlayChannel((int)this);
  return;
}

