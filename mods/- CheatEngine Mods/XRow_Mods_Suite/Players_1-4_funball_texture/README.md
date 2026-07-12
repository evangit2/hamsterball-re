# "Players 1-4 funball texture"

**CT Entry ID:** 160

**Script Type:** Code cave / complex

**Uses alloc:** Yes

**Uses registersymbol:** Yes

## Script

```
[ENABLE]

alloc(shapeshift_funball_hook, 128)
alloc(saved_sphere_mesh_funball, 4)

registersymbol(saved_sphere_mesh_funball)

label(shapeshift_skip)
label(shapeshift_swap)

saved_sphere_mesh_funball:
  dd 0

shapeshift_funball_hook:
  mov eax, [esi+10]
  test eax, eax
  jz shapeshift_skip

  mov ecx, [saved_sphere_mesh_funball]
  test ecx, ecx
  jnz shapeshift_swap

  mov ecx, [eax+244]
  test ecx, ecx
  jz shapeshift_skip
  mov [saved_sphere_mesh_funball], ecx

shapeshift_swap:
  mov ecx, [eax+26C]
  test ecx, ecx
  jz shapeshift_skip
  mov [eax+244], ecx

shapeshift_skip:
  mov eax, [esi+0c5c]
  jmp 00405E28

00405E22:
  jmp shapeshift_funball_hook
  nop

[DISABLE]

00405E22:
  db 8B 86 5C 0C 00 00

unregistersymbol(saved_sphere_mesh_funball)

dealloc(saved_sphere_mesh_funball)
dealloc(shapeshift_funball_hook)

```
