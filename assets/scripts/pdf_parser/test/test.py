import math

dc = 17.7
sig = 140

rc = dc / 2
tg = math.tan(math.radians(180 - sig) / 2)
h = rc * tg



print(round(h, 3), rc, tg)
