# "Glubies small size (respawning speed is slow)"

**CT Entry ID:** 147

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+CF524:
db 0A D7 50 3B 00


[DISABLE]
Hamsterball.exe+CF524:
db 0A D7 23 3C 00

```
