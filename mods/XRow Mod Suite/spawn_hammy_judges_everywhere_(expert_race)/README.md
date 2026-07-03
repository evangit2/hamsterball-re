# "spawn hammy judges everywhere (expert race)"

**CT Entry ID:** 258

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+E5A0:
db 90 90


[DISABLE]
Hamsterball.exe+E5A0:
db 75 5F

```
