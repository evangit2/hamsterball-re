# "Camera FOV far"

**CT Entry ID:** 193

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+CF3F0:
db 00 00 80 3F 00 00


[DISABLE]
Hamsterball.exe+CF3F0:
db 00 00 00 3F 00 00

```
