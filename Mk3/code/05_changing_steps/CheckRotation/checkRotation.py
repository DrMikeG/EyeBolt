from re import M, S
from tkinter import NS
from typing import final
import matplotlib.pyplot as plt
import matplotlib.lines as lines
import matplotlib.colors as colours
import numpy as np
import sys
import math


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
        return "\n[%u / %u %s %s]" %(self.rev,self.step,str(self.touched),str(self.sweep))

    def minAndMax(self,dir = 1):
        min = 0
        max = len(self.sweep)
        i = 0
        for s in self.sweep[::dir]:
            if s == '1':
                if min == 0:
                    min = i
                    max = i
                else:
                    max = i
            i = i +1
        return min,max
    
    def minAndMaxMirror(self):
        return self.minAndMax(-1)
    
    def centerOfMassInSweep(self):
        # If you have two neighbours, your weight is doubled...
        printStr = self.printSweepAsDots()
        #print(printStr)

        n = len(self.sweep)
        weights = [0.0] * n
        # Add a half for each neighbour you have
        for i in range(n):
            # value_true if <test> else value_false
            weights[i] = 1.0 if self.sweep[i] == '1' else 0.0
            if i-1 > -1 and self.sweep[i-1] == '1':
                weights[i] = weights[i] + 0.5
            if i+1 < len(self.sweep) and self.sweep[i+1] == '1':
                weights[i] = weights[i] + 0.5                
        
        #print(weights)
        halfWeight = sum(weights) / 2.0
        # Find the point where there as many to the left as there are to the right...
        sumWeight = 0.0
        comIndex = 0
        #debugWeights = [0.0] * n
        for i in range(n):
            sumWeight = sumWeight + weights[i]
            #debugWeights[i] = sumWeight
            if sumWeight >= halfWeight:
                comIndex = i
                break
        #print(debugWeights)
        #print("Center of mass index is %u" % (comIndex))        
        strIndex =comIndex + 9
        printStr = printStr[:strIndex] + 'V' + printStr[strIndex+1:] 
        print(printStr)
        return comIndex



    def printSweepAsDots(self, dir = 1, comIndex = -1):
        str = ""
        for s in self.sweep[::dir]:
            if s =='0':
                str = str +'.'
            else:
                str = str +'*'            
        return "%03d/%03d: %s" %(self.rev,self.step,str)

    def printSweepAsDotsMirror(self):
        return self.printSweepAsDots(-1)


def Avg(myList):
    s = sum(myList)
    l = 1.0 * len(myList)
    return s / l

def Find(scan,rev,step):
    for s in scan:
        if s.rev == rev and s.step == step:
            return s

def ComBasedOnRotation(zeroComs,mirrorComs,nMeasuresInSweep):
    # Average all the zero degree
    firstPositionAvg = Avg(zeroComs)
    mirrorPositionAvg = Avg(mirrorComs)
    mirrorMirrorPositionAvg =  nMeasuresInSweep -mirrorPositionAvg
    finalAvg = (firstPositionAvg+mirrorMirrorPositionAvg)/2
    return int(finalAvg)

def Cross(xa,ya,xb,yb):
    return xa*xb + ya*yb


def Normal(x, y):

    # Normalise vector such that vec = length*dir.
    # Return length.
    # Dir is always a valid unit vector, even when vec is length 0.0.
    # Test returned length against local resolution when this matters.
    # Do not test on len squared.  Some optimisers ruin this test.
    
    _len = math.sqrt(Cross(x,y,x,y))
    if not _len == 0.0:
        x = x/_len
        y = y/_len
    else:
        x = 1.0
        y = 0.0    
    
    return x,y

def Line(x1,y1,x2,y2,transform, figure, colour):
    scale = 4
    # Inputs are [-1 - +1]
    # Output are [0 - 1]
    new_x1 = (x1 / scale) + 0.5
    new_x2 = (x2 / scale) + 0.5
    new_y1 = (y1 / scale) + 0.5
    new_y2 = (y2 / scale) + 0.5
    return lines.Line2D([new_x1, new_x2], [new_y1, new_y2], transform=transform, figure=figure, color=colour)

def Rotate(x,y,rotAngle):
    r = math.radians(rotAngle * 360.0)
    new_x = (x * math.cos(r)) - (y * math.sin(r))
    new_y = (x * math.sin(r)) + (y * math.cos(r))
    return new_x, new_y

def onclick(event):
    global comX
    global comY
    global ix, iy
    print("Click")
    ix, iy = event.xdata, event.ydata
    # input [0.0 - 1.0]
    # output [-1.0 to 1.0]
    print(ix, iy)
    comX = ((ix * 2.0) - 1.0)
    comY = ((iy * 2.0) - 1.0)
    print(comX, comY)
    renderToFigureForXY()
    

def renderToFigureForXY():
    global comX
    global comY
    global nMeasuresInSweep
    global nStepsPerRev
    global scan
    global fig
    fig.clear()

    myLines = []

    yDelta = 2.0 / (1.0*nMeasuresInSweep)

    # -0.2,-0.05
    comX = -0.35
    comY = -0.36
    rots = range(0,108)

    for i in range(70):
        #for i in range(50)[0::2]:

        step = rots[i]

        colFrac = (i / (1.0 * nStepsPerRev))
        colour = colours.hsv_to_rgb((colFrac,0.65,0.65))

        sweep = Find(scan,0,step)
        # Rotation [0.0 - 1.0]
        rotation = step * (1.0 / nStepsPerRev)

        # i want to adjust y to account for a virtual center of mess,
        y = -1.0

        comXDelta,comYDelta = Rotate(comX,comY,rotation)

        for measure in range(nMeasuresInSweep):            

            # need to apply a rotation to x and y
            x1 = -1.0 + comXDelta
            y1 = y + comYDelta
            x2 = 1.0 + comXDelta
            y2 = y + comYDelta
            x1,y1 = Rotate(x1,y1,rotation)
            x2,y2 = Rotate(x2,y2,rotation)

            if sweep.sweep[measure] == '0':
                    myLines.append( Line(x1, y1, x2, y2,fig.transFigure,fig,colour) )
                #else:
                #    myLines.append( Line(x1, y1, x2, y2,fig.transFigure,fig,colour) )
            y = y + yDelta

    fig.lines.clear()
    fig.lines.extend(myLines)
    plt


filename = f'{sys.path[0]}/slide_pin_scan_01.txt'
#filename = f'{sys.path[0]}/slide_scan_02.txt'

scan = [
    Measurement(line.strip())
    for line in open(filename, 'r').read().split('\n')
]

#print(scan)

nRevs = scan[-1].rev + 1
nStepsPerRev = scan[-1].step+ 1
nMeasuresInSweep = len(scan[-1].sweep)
nStepsIn180 = int(nStepsPerRev / 2)
print("[%03d, %03d, %03d, %03d" %(nRevs,nStepsPerRev,nMeasuresInSweep,nStepsIn180))



# Aim is to get a radius and an angle, which represents the idealised center of the object positioned at 0 degrees.
# We calculate the radius and angle from and X and Y offset
# X being across the turn table
# Y being depth of the turn table

# With these values, we can project the measurements taken into a neutral space, to account for both
# - the rotation of the table
# - he procession of the object relative to the turn table's center

x_offset = -15
y_offset = -15

x_unit, y_unit = Normal(x_offset,y_offset)
# Four quadrants
# A B
# C D


step = 0
zeroComs = []
oneEightyComs = []

# Calculate the center of mass at 0 degrees and 180 degrees in each row of scan
for row in range(nRevs):
    zeroComs.append(Find(scan,row,step).centerOfMassInSweep())
    #s1 = Find(scan,row,step).centerOfMassInSweep()
    #s2 = Find(scan,row,step+54).centerOfMassInSweep()
    #print("%s %s" %(s1,s2))
for row in range(nRevs):
    oneEightyComs.append(Find(scan,row,step+nStepsIn180).centerOfMassInSweep())
    #s1 = Find(scan,row,step).centerOfMassInSweep()
    #s2 = Find(scan,row,step+54).centerOfMassInSweep()
    #print("%s %s" %(s1,s2))
   
xOffset = ComBasedOnRotation(zeroComs,oneEightyComs,nMeasuresInSweep)

print(xOffset)




#print(Find(scan,row,0).printSweepAsDots())
#min,max = Find(scan,row,0).minAndMax()
#print ("min = %u max = %u" % (min,max))

#print(Find(scan,row,54).printSweepAsDotsMirror())
#min,max = Find(scan,row,54).minAndMaxMirror()
#print ("min = %u max = %u" % (min,max))

#print("Output:")

# I want to print out the 10th steps and the 62nd steps as they should be 180 degrees apart
#for j in range(0,54,2): # 180 degrees of table turn
#    for i in range(10): #rows
#        s1 = Find(scan,i,j).printSweepAsDots()
#        s2 = Find(scan,i,j+54).printSweepAsDotsMirror()
#        print("%s %s" %(s1,s2))

# the default coordinate system is "figure coordinates" where (0, 0) is the bottom-left of the figure and (1, 1) is the top-right of the figure.
# I want my space to be zero origin with -1 to +1 bounds in X and Y, so I need to apply an offset to all lines I create

fig = plt.figure()

renderToFigureForXY()

fig.canvas.mpl_connect('button_press_event', onclick)
plt.title("mouse tied to frame")
plt.show()