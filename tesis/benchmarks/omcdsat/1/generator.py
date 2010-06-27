#!/usr/bin/env python

import sys

first = 3
top = 4

levels = int(sys.argv[1])

while top <= 2**levels:
    for i in xrange(first, top+1):
        parent = (i+1)/2

        print "r%d(X1, X2) :- r%d(X1,X2)" % (i, parent)

    first = top+1
    top *= 2
