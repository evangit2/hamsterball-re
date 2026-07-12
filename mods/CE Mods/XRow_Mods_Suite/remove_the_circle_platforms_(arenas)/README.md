# "remove the circle platforms (arenas)"

**CT Entry ID:** 205

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+CFF5C:
db 90 90 90 90 90


[DISABLE]
Hamsterball.exe+CFF5C:
db 53 54 41 4E 44

```
