# "Glubies tiny size (respawning speed is very slow)"

**CT Entry ID:** 148

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+CF524:
db 0A D7 50 3A 00


[DISABLE]
Hamsterball.exe+CF524:
db 0A D7 23 3C 00

```
