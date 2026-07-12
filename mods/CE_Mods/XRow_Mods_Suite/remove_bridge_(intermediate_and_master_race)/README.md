# "remove bridge (intermediate and master race)"

**CT Entry ID:** 299

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+CF678:
db 90 90 90 90 90 90



[DISABLE]
Hamsterball.exe+CF678:
db 42 52 49 44 47 45

```
