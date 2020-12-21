#!/usr/bin/env python3
# socat TCP-LISTEN:27182,reuseaddr,fork EXEC:"timeout $((60*60*6)) ./tcptester.py"
import sys, time

try:
    sleeptime = sys.stdin.readline(64)
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
