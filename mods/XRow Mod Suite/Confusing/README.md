# "Confusing"

**CT Entry ID:** 191

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+CF304:
db 00 80 BB 44 9A 99


[DISABLE]
Hamsterball.exe+CF304:
db 00 00 7F 43 9A

```
