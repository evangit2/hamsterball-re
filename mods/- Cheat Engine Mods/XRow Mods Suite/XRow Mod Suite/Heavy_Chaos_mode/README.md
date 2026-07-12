# "Heavy Chaos mode"

**CT Entry ID:** 306

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+CF310:
db 9A 99 99 3E 50 2A 40


[DISABLE]
Hamsterball.exe+CF310:
db 00 00 80 3F 50 2A

```
