#!/usr/bin/python

import sys
import random

n = int(sys.argv[1])
top = int(sys.argv[2])

random.seed()

print n,

for i in range(n):
    print ("%d %d" % (i, random.randint(0,top))),
