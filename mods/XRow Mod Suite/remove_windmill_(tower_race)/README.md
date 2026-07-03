# "remove windmill (tower race)"

**CT Entry ID:** 171

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+CF97C:
db 90 90 90 90 90


[DISABLE]
Hamsterball.exe+CF97C:
db 57 49 4E 44 4D

```
