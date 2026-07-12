# "Gravity Flip Cycle everywhere"

**CT Entry ID:** 412

**Script Type:** Code cave / complex

**Uses alloc:** Yes

## Script

```
[ENABLE]
alloc(newmem, 1000)
alloc(gravTimer, 4)
alloc(gravState, 4)

gravTimer:
  dd 300
gravState:
  dd 0

newmem:
  push eax
  push ecx
  push edx

  // Timer only on player 1
  mov eax, [esi+0x18]
  cmp eax, 0
  jne apply_grav

  // Death fix
  push eax
  mov eax, [esi+0x14]
  test eax, eax
  jz @f
  mov eax, [eax+0x878]
  test eax, eax
  jz @f
  mov byte ptr [eax+0x237], 1
@@:
  pop eax

  dec dword ptr [gravTimer]
  cmp dword ptr [gravTimer], 0
  jg apply_grav
  mov dword ptr [gravTimer], 300

  inc dword ptr [gravState]
  cmp dword ptr [gravState], 4
  jl apply_grav
  mov dword ptr [gravState], 0

apply_grav:
  mov eax, [esi+0x1A4]
  test eax, eax
  jz done

  mov ecx, [gravState]
  cmp ecx, 0
  je grav_down
  cmp ecx, 1
  je grav_left
  cmp ecx, 2
  je grav_up

  mov dword ptr [eax+0xC8C], 0x3F800000
  mov dword ptr [eax+0xC90], 0
  mov dword ptr [eax+0xC94], 0
  jmp done

grav_down:
  mov dword ptr [eax+0xC8C], 0
  mov dword ptr [eax+0xC90], 0xBF800000
  mov dword ptr [eax+0xC94], 0
  jmp done

grav_left:
  mov dword ptr [eax+0xC8C], 0xBF800000
  mov dword ptr [eax+0xC90], 0
  mov dword ptr [eax+0xC94], 0
  jmp done

grav_up:
  mov dword ptr [eax+0xC8C], 0
  mov dword ptr [eax+0xC90], 0x3F800000
  mov dword ptr [eax+0xC94], 0

done:
  pop edx
  pop ecx
  pop eax
  mov [esi+0x168], eax
  jmp 00407C72

00407C6C:
  jmp newmem
  nop

[DISABLE]
00407C6C:
db 89 86 68 01 00 00
dealloc(newmem)
dealloc(gravTimer)
dealloc(gravState)

```
