# "remove trodes (neon race)"

**CT Entry ID:** 182

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+D00C8:
db 90 90 90 90 90


[DISABLE]
Hamsterball.exe+D00C8:
db 54 52 4F 44 45

```
