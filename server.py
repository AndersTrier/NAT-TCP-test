#!/usr/bin/env python3
# socat TCP-LISTEN:2000,reuseaddr,fork EXEC:./server.py
import sys, time

try:
    sleeptime = input()
except EOFError:
    # Connection was closed
    sys.exit()

try:
    sleeptimeint = int(sleeptime)
except ValueError:
    # Not an integer
    sys.exit()

if sleeptimeint < 0 or sleeptimeint > 60*60*24:
    sys.exit()

time.sleep(sleeptimeint)

print(sleeptimeint)
