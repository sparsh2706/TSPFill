import matplotlib.pyplot as plt
import numpy as np
import re

# Last and First Co-ordinate is same

file = open('star.txt','r')
x = []
y = []
lines = file.readlines()
length = 0

for line in lines:
	list_point = line.split(',')
	length = len(list_point)

i=0
while(i<length):
	x.append(list_point[i])
	i+=1
	if(i>=length):
		break
	y.append(list_point[i])
	i+=1

# Change from str list to int list
x = [int(i) for i in x]
y = [int(i) for i in y]


plt.plot(x,y)
plt.show()	
file.close()
