# "spawn drawbridge everywhere (tower race)"

**CT Entry ID:** 268

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+D968:
db 90 90
Hamsterball.exe+DDBB:
db 90 90


[DISABLE]
Hamsterball.exe+D968:
db 75 75
Hamsterball.exe+DDBB:
db 75 19

```
