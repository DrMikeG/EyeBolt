import matplotlib.pyplot as plt
import numpy as np
import sys


class Measurement:
    def __init__(self, inputString):
        self.inputString = inputString.replace(", ",",")
        strs = self.inputString.split(' ')
        # "0:16","beam","Broken","True","Steps"
        revStrAndstepStr, _, _, touchedStr, _, arrayStr = strs
        self.rev = int(revStrAndstepStr.split(':')[0])
        self.step = int(revStrAndstepStr.split(':')[1])
        self.touched = touchedStr == "True"
        # arrayStr - remove "[" and "]"
        # split by ,
        self.sweep = arrayStr[1:-1].split(',')

    def __repr__(self):
        return "\n[%u / %u %s %s]" %(self.rev,self.step,str(self.touched),str(self.sweep))

    def __str__(self):
        return "representation"

# This scan is of the short wall.
# The wall is 35mm long, so each end is 17.5mm from the COR
# The two long sides of the wall, one is offset 0 from COR, one is offset 2.5mm
# Watching the video, measurement 0:3 is the first that looks to touch the dead center / COR
# The bar is 10mm high
scan = [
    Measurement(line.strip())
    for line in open(f'{sys.path[0]}/slide_scan_01.txt', 'r').read().split('\n')
]

print(scan)

nValuesDeep = 100

# There are 51 steps per revolution#
# Only process the first step of each level

X = 115
Y = 1
Z = 23

x,y,z = np.indices((X,Y,Z))
voxels = (x < X) & (y < Y) & (z < Z)

z = 0 
for s in scan[0::51]:
    for x in range(len(s.sweep)):
        if scan[0].sweep[x]=='0':
            voxels[x][0][z] = False
    z = z + 1
print(z)


# and plot everything
ax = plt.figure().add_subplot(projection='3d')
ax.voxels(voxels,linewidth=0.5)
#ax.set(xlabel='z', ylabel='g', zlabel='b')

plt.show()