# "remove the waterwheel (dizzy race)"

**CT Entry ID:** 213

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+CF690:
db 90 90 90 90 90


[DISABLE]
Hamsterball.exe+CF690:
db 57 41 54 45 52

```
