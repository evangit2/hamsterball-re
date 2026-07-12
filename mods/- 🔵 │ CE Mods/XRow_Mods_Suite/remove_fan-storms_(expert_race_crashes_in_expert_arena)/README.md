# "remove fan-storms (expert race, crashes in expert arena)"

**CT Entry ID:** 174

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+CFA48:
db 90 90 90


[DISABLE]
Hamsterball.exe+CFA48:
db 46 41 4E

```
