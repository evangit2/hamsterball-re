# "Turn off lights with Blue ball lighting"

**CT Entry ID:** 464

**Script Type:** Code cave / complex

**Uses alloc:** Yes

## Script

```
[ENABLE]

alloc(neon_cave, 256)
alloc(light_data, 104)

label(skip)
label(light_loop)

light_data:
  dd 1
  dd 00000000
  dd 00000000
  dd 3F800000
  dd 3F800000
  dd 00000000
  dd 00000000
  dd 00000000
  dd 00000000
  dd 00000000
  dd 00000000
  dd 00000000
  dd 00000000
  dd 00000000
  dd 00000000
  dd 00000000
  dd 00000000
  dd 00000000
  dd 00000000
  dd 43960000
  dd 00000000
  dd 3C23D70A
  dd 00000000
  dd 00000000
  dd 00000000
  dd 00000000

neon_cave:
  push eax
  push ecx
  push edx
  push edi
  push esi

  mov eax, [esi+10]
  test eax, eax
  jz skip
  mov eax, [eax+174]
  test eax, eax
  jz skip

  mov dword ptr [eax+730], FF000000
  mov byte ptr [eax+734], 0
  mov dword ptr [eax+738], FF000000

  mov eax, [eax+154]
  test eax, eax
  jz skip
  mov edi, eax

  mov edx, [edi]
  push FF000000
  push 8B
  push edi
  call dword ptr [edx+C8]

  xor ecx, ecx
light_loop:
  push 0
  push ecx
  push edi
  mov eax, [edi]
  call dword ptr [eax+B8]
  inc ecx
  cmp ecx, 8
  jl light_loop

  cmp dword ptr [esi+18], 0
  jne skip

  mov eax, [esi+164]
  mov [light_data+34], eax
  mov eax, [esi+168]
  mov [light_data+38], eax
  mov eax, [esi+16C]
  mov [light_data+3C], eax

  push light_data
  push 7
  push edi
  mov eax, [edi]
  call dword ptr [eax+B0]

  push 1
  push 7
  push edi
  mov eax, [edi]
  call dword ptr [eax+B8]

skip:
  pop esi
  pop edi
  pop edx
  pop ecx
  pop eax
  mov eax, [esi+0c5c]
  jmp 00405E28

00405E22:
  jmp neon_cave
  nop

[DISABLE]

00405E22:
  db 8B 86 5C 0C 00 00

dealloc(light_data)
dealloc(neon_cave)

```
