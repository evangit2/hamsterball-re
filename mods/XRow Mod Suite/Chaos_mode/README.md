# "Chaos mode"

**CT Entry ID:** 305

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+CF310:
db 33 33 33 3F 50


[DISABLE]
Hamsterball.exe+CF310:
db 00 00 80 3F 50 2A

```
