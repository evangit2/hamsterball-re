# "Allow 1 player in Arenas"

**CT Entry ID:** 333

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+31121:
  db EB
Hamsterball.exe+31129:
  db EB
Hamsterball.exe+31131:
  db EB

[DISABLE]
Hamsterball.exe+31121:
  db 75
Hamsterball.exe+31129:
  db 75
Hamsterball.exe+31131:
  db 75

```
