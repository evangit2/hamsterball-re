# "teleporting effects while falling (dizzy race)"

**CT Entry ID:** 267

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+D539:
db 90 90


[DISABLE]
Hamsterball.exe+D539:
db 75 1E

```
