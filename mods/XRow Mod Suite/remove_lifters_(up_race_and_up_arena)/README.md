# "remove lifters (up race and up arena)"

**CT Entry ID:** 163

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+CFB1C:
db 90 90 90 90 90


[DISABLE]
Hamsterball.exe+CFB1C:
db 4C 49 46 54 45

```
