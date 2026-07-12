# "Disable sound effects"

**CT Entry ID:** 195

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+CF3C8:
db 00 00 00 00 00 00 59 40 CD


[DISABLE]
Hamsterball.exe+CF3C8:
db 00 00 00 00 00 00 F0 3F CD

```
