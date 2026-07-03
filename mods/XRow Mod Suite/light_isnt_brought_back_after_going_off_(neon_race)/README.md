# "light isn't brought back after going off (neon race)"

**CT Entry ID:** 180

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+D0102:
db 90 90 90 90 90


[DISABLE]
Hamsterball.exe+D0102:
db 4C 49 47 48 54

```
