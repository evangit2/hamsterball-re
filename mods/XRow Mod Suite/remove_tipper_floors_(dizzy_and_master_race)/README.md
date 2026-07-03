# "remove tipper floors (dizzy and master race)"

**CT Entry ID:** 300

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+CF69C:
db 90 90 90 90 90


[DISABLE]
Hamsterball.exe+CF69C:
db 54 49 50 50 45

```
