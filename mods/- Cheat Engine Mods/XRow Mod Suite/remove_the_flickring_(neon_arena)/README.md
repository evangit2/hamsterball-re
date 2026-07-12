# "remove the flickring (neon arena)"

**CT Entry ID:** 190

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+D0144:
db 90 90 90 90 90


[DISABLE]
Hamsterball.exe+D0144:
db 46 4C 49 43 4B

```
