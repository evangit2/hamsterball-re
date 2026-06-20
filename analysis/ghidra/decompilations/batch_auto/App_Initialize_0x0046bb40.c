
void __thiscall App_Initialize(void *this,undefined4 param_1,undefined4 param_2)

{
  char *pcVar1;
  undefined4 unaff_retaddr;
  
  *(char **)((int)this + 0x208) = "App::Initialize(1)";
  (**(code **)(*(int *)this + 0x94))(param_2);
  *(undefined4 *)((int)this + 4) = unaff_retaddr;
  *(char **)((int)this + 0x208) = "App::Initialize(2)";
  RegKey_SetSoftwarePath(*(int *)((int)this + 0x54));
  *(char **)((int)this + 0x208) = "App::Initialize(3)";
  (**(code **)(*(int *)this + 0xc))();
  *(char **)((int)this + 0x208) = "App::Initialize(4)";
  (**(code **)(*(int *)this + 0x1c))();
  *(char **)((int)this + 0x208) = "App::Initialize(5)";
  (**(code **)(*(int *)this + 0x18))();
  *(char **)((int)this + 0x208) = "App::Initialize(6)";
  (**(code **)(*(int *)this + 0x30))();
  *(char **)((int)this + 0x208) = "App::Initialize(7)";
  (**(code **)(*(int *)this + 0x3c))();
  *(char **)((int)this + 0x208) = "App::Initialize(8)";
  (**(code **)(*(int *)this + 0x34))();
  *(char **)((int)this + 0x208) = "App::Initialize(9)";
  (**(code **)(*(int *)this + 0x38))();
  *(char **)((int)this + 0x208) = "App::Initialize(10)";
  (**(code **)(*(int *)this + 0x40))();
  *(char **)((int)this + 0x208) = "App::Initialize(11)";
  Graphics_Initialize(*(void **)((int)this + 0x174),*(undefined4 *)((int)this + 8),
                      *(undefined4 *)((int)this + 0x15c),*(undefined4 *)((int)this + 0x160),
                      *(undefined1 *)((int)this + 0x158));
  if (*(char *)(*(int *)((int)this + 0x174) + 0x60) != '\0') {
    MessageBoxA((HWND)0x0,
                "Hamsterball was not able to initialize DirectX!  Hamsterball requires DirectX8.0 or better to run.  To download the latest DirectX, visit http://www.microsoft.com/directx"
                ,"DirectX Initialization Error",0);
    CRT_FlsAlloc(0);
  }
  *(char **)((int)this + 0x208) = "App::Initialize(12)";
  if (*(char **)((int)this + 0x1b4) != (char *)0x0) {
    pcVar1 = strchr(*(char **)((int)this + 0x1b4),0x2d);
    if (pcVar1 != (char *)0x0) {
      Graphics_SetTexturePath(*(void **)((int)this + 0x174),pcVar1 + 1);
    }
  }
  *(char **)((int)this + 0x208) = "App::Initialize(Ok)";
  return;
}

