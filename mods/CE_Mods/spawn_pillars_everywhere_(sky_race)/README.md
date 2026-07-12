# "spawn pillars everywhere (sky race)"

**CT Entry ID:** 295

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+1093B:
db 90 90

[DISABLE]
Hamsterball.exe+1093B:
db 75 0C

```
