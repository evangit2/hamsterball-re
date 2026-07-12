# "spawn trapdoors everywhere (tower race)"

**CT Entry ID:** 294

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+DA94:
db 90 90 90 90 90 90


[DISABLE]
Hamsterball.exe+DA94:
db 0F 85 C2 00 00 00

```
