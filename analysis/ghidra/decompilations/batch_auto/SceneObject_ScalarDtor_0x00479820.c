
void * __thiscall SceneObject_ScalarDtor(void *this,byte param_1)

{
  SceneObject_DeletingDtor(this);
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

