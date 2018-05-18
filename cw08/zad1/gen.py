import sys
import random

if len(sys.argv) < 2:
    raise ValueError("specyfy number of records")

f = open('data.txt', 'w')

for i in range(0, int(sys.argv[1])):
    for x in range(0, 1024):
        f.write(chr(random.randint(ord('A'),ord('Z'))))

f.close()

