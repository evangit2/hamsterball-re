# "Dizzy physics everywhere"

**CT Entry ID:** 397

**Script Type:** Code cave / complex

**Uses alloc:** Yes

**Uses registersymbol:** Yes

## Script

```
[ENABLE]
alloc(newmem, 1000)
alloc(globalDizzyTimer, 4)
alloc(globalDizzyInterval, 4)
alloc(processedThisFrame, 4)
registersymbol(globalDizzyTimer)
registersymbol(globalDizzyInterval)

globalDizzyTimer:
  dd 0
globalDizzyInterval:
  dd 300
processedThisFrame:
  dd 0

newmem:
  push eax
  push ecx
  push edx

  cmp dword ptr [processedThisFrame], 1
  je apply_dizzy

  cmp dword ptr [globalDizzyTimer], 0
  jle trigger_now
  dec dword ptr [globalDizzyTimer]
  jmp done

trigger_now:
  mov dword ptr [processedThisFrame], 1
  mov eax, [globalDizzyInterval]
  mov [globalDizzyTimer], eax

apply_dizzy:
  cmp dword ptr [esi+0x2F0], 0
  jg done
  mov ecx, esi
  call 00403750

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
unregistersymbol(globalDizzyTimer)
unregistersymbol(globalDizzyInterval)
dealloc(newmem)
dealloc(globalDizzyTimer)
dealloc(globalDizzyInterval)
dealloc(processedThisFrame)

```
