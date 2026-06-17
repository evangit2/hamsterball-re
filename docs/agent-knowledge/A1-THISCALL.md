# A1 - __thiscall Convention for Hamsterball

All member functions (constructors, destructors, vtable methods) in `Hamsterball.exe` use `__thiscall`.

## Detection

Prologue pattern: `PUSH ECX; PUSH ESI; MOV ESI, ECX` saves `this` from ECX.

## Correct Usage

```cpp
typedef void* (__thiscall *Ball_ctor_fn)(void* thisPtr, int scene);
auto BallCtor = (Ball_ctor_fn)0x40AFE0;
void* ball = BallCtor(allocatedMem, (int)scene);

// vtable[1] init defaults
auto InitPhysics = (void (__thiscall *)(void*))0x405100;
InitPhysics(ball);
```

## Constructor Chain

- `Ball_ctor` (0x40AFE0) is the public constructor for badballs.
- It calls `Ball_ctor2` (0x4039E0) internally, then overwrites the vtable to the live Ball vtable (0x4CF3A0).
- Do NOT call `Ball_ctor2` manually when using `Ball_ctor`.

## Heap Allocation

Avoid `operator_new` from injected code. Use:

```cpp
HANDLE hCrtHeap = *(HANDLE*)0x005369C0;
void* mem = HeapAlloc(hCrtHeap, 0, 0xC98);
```

Compute runtime address with `GetModuleHandle(NULL) + RVA` for ASLR safety.
