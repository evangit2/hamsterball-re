
void * __thiscall TourneyBoard_ObjectDtor(void *this,byte param_1)

{
  SceneObject_FreeStrings(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

