# "Glubies huge size (respawning speed is fast)"

**CT Entry ID:** 146

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+CF524:
db 0A D7 99 3C 00


[DISABLE]
Hamsterball.exe+CF524:
db 0A D7 23 3C 00

```
