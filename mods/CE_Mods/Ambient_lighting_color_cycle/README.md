# "Ambient lighting color cycle"

**CT Entry ID:** 460

**Script Type:** Code cave / complex

**Uses alloc:** Yes

**Uses registersymbol:** Yes

## Script

```
[ENABLE]

alloc(bgcolor_cave, 256)
alloc(bgcolor_colors, 64)
alloc(bgcolor_frame, 4)
alloc(bgcolor_index, 4)
alloc(bgcolor_timer, 4)

registersymbol(bgcolor_frame)
registersymbol(bgcolor_index)
registersymbol(bgcolor_timer)

bgcolor_colors:
  dd FF0000FF  // Blue
  dd FF00FF00  // Green
  dd FFFFFF00  // Yellow
  dd FFFFFFFF  // White
  dd FF000000  // Black
  dd FFFF0000  // Red
  dd FF00FFFF  // Cyan
  dd FFFF00FF  // Magenta
  dd FF808080  // Gray
  dd FFFF8000  // Orange
  dd FF8000FF  // Purple
  dd FF0080FF  // Light Blue
  dd FF80FF00  // Lime
  dd FFFF0080  // Pink
  dd FF008080  // Teal
  dd 00000000   // Terminator

bgcolor_frame:
  dd 0

bgcolor_index:
  dd 0

bgcolor_timer:
  dd 80

label(bgcolor_keep)
label(bgcolor_store)
label(skip)

bgcolor_cave:
  push eax
  push ecx
  push edx

  mov eax, [esi+10]
  test eax, eax
  jz skip
  mov eax, [eax+174]
  test eax, eax
  jz skip

  mov eax, [bgcolor_frame]
  inc eax
  mov [bgcolor_frame], eax

  cmp eax, [bgcolor_timer]
  jl bgcolor_keep

  mov dword ptr [bgcolor_frame], 0
  mov eax, [bgcolor_index]
  inc eax

  mov edx, bgcolor_colors
  mov edx, [edx+eax*4]
  test edx, edx
  jnz bgcolor_store
  xor eax, eax

bgcolor_store:
  mov [bgcolor_index], eax

bgcolor_keep:
  mov eax, [esi+10]
  mov eax, [eax+174]
  mov ecx, [bgcolor_index]
  mov edx, bgcolor_colors
  mov edx, [edx+ecx*4]
  mov [eax+730], edx

  mov eax, [eax+154]
  test eax, eax
  jz skip
  mov ecx, [eax]
  push edx
  push 8B
  push eax
  call dword ptr [ecx+C8]

skip:
  pop edx
  pop ecx
  pop eax
  mov eax, [esi+0c5c]
  jmp 00405E28

00405E22:
  jmp bgcolor_cave
  nop

[DISABLE]

00405E22:
  db 8B 86 5C 0C 00 00

dealloc(bgcolor_cave)
dealloc(bgcolor_colors)
dealloc(bgcolor_frame)
dealloc(bgcolor_index)
dealloc(bgcolor_timer)
unregistersymbol(bgcolor_frame)
unregistersymbol(bgcolor_index)
unregistersymbol(bgcolor_timer)

```
