import matplotlib.image as mpimg
import numpy as np
import matplotlib.pyplot as plt
 
x=[1,2,3,4]
y=[1,4,9,16]
 
plt.close('all')
fig = plt.figure()
ax = fig.add_subplot(111)
ax.plot(x,y,'o')
coords = []
 
def onclick(event):
    print("hello there")
    global ix, iy
    ix, iy = event.xdata, event.ydata
    #print 'x = %f, y = %f'%(ix, iy)
    global coords
    coords.append((ix, iy))
    print(len(coords))
    z=len(coords)-1
    print(coords[z][1])
    per = (10*coords[z][1])/100
    errp = abs(coords[z][1]+per)
    errn = abs(coords[z][1]-per)
    print("errn=%f, errp=%f"%(errn, errp))
    for i in range(0,1000):
        print(i)
        if abs(float(y[i])) >= errn and abs(float(y[i])) <= errp:
            print(y[i])
            # Eventually, after clicking on the point I will plot the imshow as below
            #fig2 = plt.figure()
            #mymap = read_mydata('IMAGE'+str(i+1)+'.txt')
            #mymap = mymap.reshape(dims[0],dims[1])
            #plt.imshow(mymap)
            #fig2.show()
    return coords
 
cid = fig.canvas.mpl_connect('button_press_event', onclick)
plt.show()