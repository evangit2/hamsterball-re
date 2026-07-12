# "magnifier glass doesnt follow you (sky race)"

**CT Entry ID:** 203

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+CFDD2:
db 90 90 90 90 90


[DISABLE]
Hamsterball.exe+CFDD2:
db 48 45 41 54 4F

```
