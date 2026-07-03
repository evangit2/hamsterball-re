# "Camera FOV very far"

**CT Entry ID:** 192

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+CF3F0:
db 00 00 C0 3F 00 00


[DISABLE]
Hamsterball.exe+CF3F0:
db 00 00 00 3F 00 00

```
