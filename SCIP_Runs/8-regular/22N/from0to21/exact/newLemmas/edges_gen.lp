Minimize
 obj: 0
Subject To

ea: a1 + x - d = 0
eab: [r^2] - r - 2 d - p - a2 - b1 - c = 0
eb: [0.5 n * r - r^2] + d - r + p - b2 - y = 0
ea3: [r * p] - 2 p  - a1 - a2 = 0
eb5: [r^2 - r * p] - r + p - b1 - b2 = 0
ea5: [r^2 - r * p] - 2 x - r + p - a1 - b1 - c = 0
eb7: [r * n - 2 r^2 + r * p] - 2 r - 2 y - b2 - a2 - c = 0

plow: p >= 0
phigh: p - r <= 0

b1high: b1 + [p^2 - r^2] <= 0

a1high: a1 + [p^2 - r*p] <= 0

xhigh: x <= 10

Bounds
 22 <= n <= 22
 r = 8
 a1 >= 0
 a2 >= 0
 b1 >= 0
 b2 >= 0
 x >= 0
 y >= 0
 c >= 0
 21 <= d <= 22
 p=3

General
a1
a2
b1
b2
x
y
c
p
n
d
r