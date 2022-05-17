import matplotlib.pyplot as plt
import numpy as np

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
    return coords


fig, ax1=plt.subplots()
t = np.linspace(0, 1)
for i in range(4):
    y1 = (2+i*2) * np.sin(2*np.pi*pow(2,i)*t)
    ax1.plot(t, y1, lw=2, label= f'{i+1} HZ')

leg=ax1.legend(loc="upper left")
cid = fig.canvas.mpl_connect('button_press_event', onclick)
plt.title("press 'h' to hide legend")
plt.show()