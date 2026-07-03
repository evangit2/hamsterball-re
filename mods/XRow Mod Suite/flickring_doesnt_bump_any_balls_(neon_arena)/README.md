# "flickring doesnt bump any balls (neon arena)"

**CT Entry ID:** 215

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+D0150:
db 90 90 90


[DISABLE]
Hamsterball.exe+D0150:
db 4E 3A 42

```
