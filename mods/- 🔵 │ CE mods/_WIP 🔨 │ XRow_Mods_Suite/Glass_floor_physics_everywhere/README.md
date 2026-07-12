# "Glass floor physics everywhere"

**CT Entry ID:** 393

**Script Type:** Code cave / complex

**Uses alloc:** Yes

**Uses registersymbol:** Yes

## Script

```
[ENABLE]
alloc(newmem, 1000)
alloc(glassMode, 4)
registersymbol(glassMode)

glassMode:
  dd 1

newmem:
  // Check if glass mode is enabled
  cmp dword ptr [glassMode], 0
  je original

  // Set glass physics for this object
  mov dword ptr [esi+0x0c5c], 0xf

original:
  mov eax, [esi+0x0c5c]
  jmp 00405E28

00405E22:
  jmp newmem
  nop

[DISABLE]
00405E22:
db 8B 86 5C 0C 00 00
unregistersymbol(glassMode)
dealloc(newmem)
dealloc(glassMode)

```
