# "Remove the ball from hamster"

**CT Entry ID:** 5

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+CF404:
db 33 33 FC 40 D9

[DISABLE]
Hamsterball.exe+CF404:
db CC CC F0 3F D9

```
