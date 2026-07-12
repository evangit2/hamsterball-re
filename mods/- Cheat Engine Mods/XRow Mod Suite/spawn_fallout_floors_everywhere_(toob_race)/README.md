# "spawn fallout floors everywhere (toob race)"

**CT Entry ID:** 271

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+FD5A:
db 90 90


[DISABLE]
Hamsterball.exe+FD5A:
db 75 76

```
