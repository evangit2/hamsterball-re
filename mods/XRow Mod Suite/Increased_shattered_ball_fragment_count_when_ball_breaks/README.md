# "Increased shattered ball fragment count when ball breaks"

**CT Entry ID:** 49

**Script Type:** Code cave / complex

**Uses alloc:** Yes

**Uses registersymbol:** Yes

## Script

```
[ENABLE]
alloc(callCount, 4)
registersymbol(callCount)

alloc(hookHamsterEnd, 1000)
hookHamsterEnd:
  inc dword ptr [callCount]

  push ecx
  mov byte ptr [edi+0x324], 0
  mov ecx, edi
  call 00409480
  mov byte ptr [edi+0x324], 0
  mov ecx, edi
  call 00409480
  mov byte ptr [edi+0x324], 0
  mov ecx, edi
  call 00409480
  mov byte ptr [edi+0x324], 0
  mov ecx, edi
  call 00409480
  mov byte ptr [edi+0x324], 0
  mov ecx, edi
  call 00409480
  pop ecx

  mov ecx, [esp+0x24]
  pop edi
  pop esi
  pop ebp
  pop ebx
  add esp, 0x20
  ret

00409468:
  jmp hookHamsterEnd

[DISABLE]
00409468:
db 8B 4C 24 24 5F 5E 5D 5B 83 C4 20 C3
dealloc(callCount)
dealloc(hookHamsterEnd)
unregistersymbol(callCount)

```
