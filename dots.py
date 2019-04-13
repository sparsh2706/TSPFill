import matplotlib.pyplot as plt
import numpy as np
import re

# Last and First Co-ordinate is same

def boundary(filename):
	""" Enter the Filename through input and plot for boundary would be generated """

	file = open(filename,'r')
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


	file.close()

	return x,y


if __name__ == '__main__':
	
	x,y = boundary(input("Enter File Name with Extension\n"))
	
	if(len(x) == len(y)):
		print("Co-ordinates Loaded :) ")
		print()

	points = []
	for i in range(len(x)):
		points.append([x[i],y[i]])

	print("Points List has been made\n")

	plt.plot(x,y)

	axes = plt.gca()
	plt.show()

	xmin = min(x)
	xmax = max(x)
	ymin = min(y)
	ymax = max(y)

	print("Minimum X: " + str(xmin))
	print("Maximum X: " + str(xmax))
	print("Minimum Y: " + str(ymin))
	print("Maximum X: " + str(ymax))


	y_plot_min,y_plot_max = axes.get_ylim()
	x_plot_min,x_plot_max = axes.get_xlim()

	x_init = x_plot_min
	y_init = y_plot_min
	x_end = x_plot_max
	y_end = y_plot_max

	print("Plot Min Lim: " + str(x_init) + " , " + str(y_init))
	print("Plot Max Lim: " + str(x_end) + " , " + str(y_end))

	all_points = []

	for i in range(int(x_init),int(x_end),int(5)):
		for j in range(int(y_init),int(y_end),int(5)):
			all_points.append([i,j])
			plt.scatter(i,j)


	plt.show()

	print(all_points)

	for point in all_points:
		if(point[0] >= xmin and point[0] <= xmax and point[1] >= ymin and point[1] <= ymax):
			plt.scatter(point[0],point[1])

	plt.show()

