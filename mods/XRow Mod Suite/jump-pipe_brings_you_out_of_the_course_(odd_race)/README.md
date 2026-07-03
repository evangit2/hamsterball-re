# "jump-pipe brings you out of the course (odd race)"

**CT Entry ID:** 224

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+CFBE8:
db 90 90 90 90 90
Hamsterball.exe+CFBCC:
db 90 90 90 90 90


[DISABLE]
Hamsterball.exe+CFBE8:
db 4A 55 4D 50 50
Hamsterball.exe+CFBCC:
db db 4A 55 4D 50 50

```
