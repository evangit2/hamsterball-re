# "Entity Speed and Size "

**CT Entry ID:** 400

**Script Type:** Code cave / complex

**Uses alloc:** Yes

## Script

```
[ENABLE]
alloc(newmem, 1000)
alloc(sizeP1, 4)
alloc(sizeBad, 4)
alloc(sizeP2, 4)
alloc(masterSpeedP1, 4)
alloc(masterSpeedBad, 4)
alloc(maxSpeedP1, 4)
alloc(maxSpeedBad, 4)
alloc(dragValue, 4)

// ===== Player 1 Speed and Size =====
sizeP1:
  dd (float)39 // Size, default 26
masterSpeedP1:
  dd (float)8.0
maxSpeedP1:
  dd (float)13.0

// ===== Badballs Speed and Size =====
sizeBad:
  dd (float)35 // Size, default 37
masterSpeedBad:
  dd (float)5.0
maxSpeedBad:
  dd (float)7.0

// ===== Players 2-4 Size =====
sizeP2:
  dd (float)28 // Size, default 26

// ======== Don't Edit =========
dragValue:
  dd (float)1.0

newmem:
  push eax
  push edx

  mov eax, [esi+0x18]
  cmp eax, 0
  jne @f
  // Player 1
  mov eax, [sizeP1]
  mov [esi+0x284], eax
  mov eax, [masterSpeedP1]
  mov [esi+0x188], eax
  mov eax, [esi+0x1A4]
  cmp eax, 0
  je done
  mov edx, [maxSpeedP1]
  mov [eax+0xC70], edx
  mov edx, [dragValue]
  mov [eax+0xC68], edx
  jmp done
@@:
  cmp eax, -1
  jne @f
  // Badball
  mov eax, [sizeBad]
  mov [esi+0x284], eax
  mov eax, [masterSpeedBad]
  mov [esi+0x188], eax
  mov eax, [esi+0x1A4]
  cmp eax, 0
  je done
  mov edx, [maxSpeedBad]
  mov [eax+0xC70], edx
  mov edx, [dragValue]
  mov [eax+0xC68], edx
  jmp done
@@:
  // Players 2-4
  mov eax, [sizeP2]
  mov [esi+0x284], eax

done:
  pop edx
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
dealloc(sizeP1)
dealloc(sizeBad)
dealloc(sizeP2)
dealloc(masterSpeedP1)
dealloc(masterSpeedBad)
dealloc(maxSpeedP1)
dealloc(maxSpeedBad)
dealloc(dragValue)

```
