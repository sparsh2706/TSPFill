from shapely.geometry import Polygon
import numpy as np
polygon =  Polygon([(0,0), (4,0), (2,4)])
print(polygon.area)
print(polygon.length)
print (polygon)
e=[1]
s=[0]
for i in range(2,6,3):
    e.append(i)
    s.append(5*i+10)
plt.plot(e,s)
plt.axis('scaled')
plt.show()
print(e)
print(s)