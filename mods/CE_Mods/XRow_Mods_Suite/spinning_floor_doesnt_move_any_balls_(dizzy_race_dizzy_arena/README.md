# "spinning floor doesn't move any balls (dizzy race, dizzy arena)"

**CT Entry ID:** 199

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+CF92A:
db 90 90 90 90 90


[DISABLE]
Hamsterball.exe+CF92A:
db 53 57 49 52 4C

```
