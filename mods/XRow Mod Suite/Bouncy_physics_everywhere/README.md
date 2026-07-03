# "Bouncy physics everywhere"

**CT Entry ID:** 396

**Script Type:** Code cave / complex

**Uses alloc:** Yes

**Uses registersymbol:** Yes

## Script

```
[ENABLE]
alloc(newmem, 1000)
alloc(bounceForce, 4)
alloc(bounceTimer, 4)
registersymbol(bounceForce)
registersymbol(bounceTimer)

bounceForce:
  dd (float)8.0
bounceTimer:
  dd 0

newmem:
  push eax
  push ecx

  // Bounce timer - jump every X frames
  cmp dword ptr [bounceTimer], 0
  jle do_bounce
  dec dword ptr [bounceTimer]
  jmp skip_bounce

do_bounce:
  mov eax, [esi+0x1A4]
  test eax, eax
  jz skip_bounce

  // Apply upward force (Y velocity = bounceForce)
  push edx
  mov edx, [bounceForce]
  mov [eax+0xCA8], edx
  pop edx

  // Reset timer (30 = bounce every half second at 60fps)
  mov dword ptr [bounceTimer], 50

skip_bounce:
  pop ecx
  pop eax

original:
  mov eax, [esi+0x0c5c]
  jmp 00405E28

00405E22:
  jmp newmem
  nop

[DISABLE]
00405E22:
db 8B 86 5C 0C 00 00
unregistersymbol(bounceForce)
unregistersymbol(bounceTimer)
dealloc(newmem)
dealloc(bounceForce)
dealloc(bounceTimer)

```
