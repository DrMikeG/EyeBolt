import matplotlib.pyplot as plt
import numpy as np
import sys
import math


class Measurement:
    def __init__(self, inputString):
        self.inputString = inputString
        strs = self.inputString.split(' ')
        revStrAndstepStr, _, touchedStr, _, distanceStr = strs
        self.rev = int(revStrAndstepStr.split(':')[0])
        self.step = int(revStrAndstepStr.split(':')[1])
        self.touched = touchedStr == "True"
        self.distanceInSteps = int(distanceStr)

    def __repr__(self):
        return "\n[%u / %u %s %u]" %(self.rev,self.step,str(self.touched),self.distanceInSteps)

    def __str__(self):
        return "representation"


def GetXsYsAndZsForScan(scan):
    # Period of scan is 51 steps
    # increase Z by 1mm per revolution
    # object is mapped in X/Y
    print(scan)

    stepstoCOR = 478
    stepsPerRev51 = 51
    revolutionHeight = 10/6 # 1.66mm
    distanceToCOR = 87.3 # mm
    factor = 6.17669
    nRevolutions = scan[-1].rev + 1 # zero indexed
    assert(nRevolutions * stepsPerRev51 == len(scan))

    xs = []
    ys = []
    zs = []

    xHome = 0
    yHome = 0
    radius = distanceToCOR
    stepAngleDegrees = 360.0 / stepsPerRev51
    stepAngleRads = math.radians(stepAngleDegrees)
    for rev_i in range(nRevolutions):
        for step_i in range(stepsPerRev51):
            measurement = scan[(rev_i * stepsPerRev51) + step_i]
            step = measurement.distanceInSteps
            if measurement.touched:
                step = stepstoCOR - step # nearer things are positive numbers
                measurementInMM = step / factor
                measuredRadius = measurementInMM
                x = xHome + (measuredRadius * math.cos(stepAngleRads*step_i))
                y = yHome + (measuredRadius * math.sin(stepAngleRads*step_i))
                z = rev_i * revolutionHeight
                xs.append(x)
                ys.append(y)
                zs.append(z)
    return xs,ys,zs

#478 steps to COR?


# This scan is of the short wall.
# The wall is 35mm long, so each end is 17.5mm from the COR
# The two long sides of the wall, one is offset 0 from COR, one is offset 2.5mm
# Watching the video, measurement 0:3 is the first that looks to touch the dead center / COR
# The bar is 10mm high
scan = [
    Measurement(line.strip())
    for line in open(f'{sys.path[0]}/nineRevs.txt', 'r').read().split('\n')
]

xs, ys, zs = GetXsYsAndZsForScan(scan)


# Fixing random state for reproducibility
np.random.seed(19680801)


def randrange(n, vmin, vmax):
    """
    Helper function to make an array of random numbers having shape (n, )
    with each number distributed Uniform(vmin, vmax).
    """
    return (vmax - vmin)*np.random.rand(n) + vmin

fig = plt.figure()
ax = fig.add_subplot(projection='3d')

n = 100

# For each set of style and range settings, plot n random points in the box
# defined by x in [23, 32], y in [0, 100], z in [zlow, zhigh].
#for m, zlow, zhigh in [('o', -50, -25), ('^', -30, -5)]:
    #xs = randrange(n, 23, 32)
    #ys = randrange(n, 0, 100)
    #zs = randrange(n, zlow, zhigh)
ax.scatter(xs, ys, zs, marker='o')

ax.set_xlabel('X Label')
ax.set_ylabel('Y Label')
ax.set_zlabel('Z Label')

plt.show()