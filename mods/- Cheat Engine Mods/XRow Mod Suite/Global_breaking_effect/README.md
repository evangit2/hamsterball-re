# "Global breaking effect"

**CT Entry ID:** 278

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+C886:
db 90 90


[DISABLE]
Hamsterball.exe+C886:
db 75 08

```
