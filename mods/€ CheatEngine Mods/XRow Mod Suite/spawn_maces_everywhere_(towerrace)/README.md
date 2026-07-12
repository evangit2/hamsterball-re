# "spawn maces everywhere (towerrace) "

**CT Entry ID:** 40

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+D88D:
db 90 90 90 90 90 90
 
 
[DISABLE]
Hamsterball.exe+D88D:
db 0F 85 BF 00 00 00

```
