# "remove maces "

**CT Entry ID:** 158

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+CF994:
db 90 90 90 90 90 90


[DISABLE]
Hamsterball.exe+CF994:
db 4D 41 43 45 00 00

```
