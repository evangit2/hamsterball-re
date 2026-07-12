# "spawn loopers everywhere (impossible race)"

**CT Entry ID:** 266

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+1801A:
db 90 90


[DISABLE]
Hamsterball.exe+1801A:
db 75 68

```
