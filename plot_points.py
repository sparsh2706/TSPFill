import matplotlib.pyplot as plt
import numpy as np
import re
import matplotlib.animation as animation
from matplotlib import style
import importlib

def tspPlot(pts_file,opt_file):
	""" PLotting of the the points """
	file = open(pts_file,'r')
	lines = file.readlines()[1:]
	x=[]
	y=[]

	for line in lines:
		list_point = re.split(' +',line)
		x.append(list_point[0])
		y.append(list_point[1])


	file.close()

	plt.plot(x,y)
	plt.xlabel('X')
	plt.ylabel('Y')
	plt.title('Normal PTS file')
	plt.show()

	path_file = open(opt_file,"r")
	fline = "list_opt = "
	oline = path_file.readlines()
	oline.insert(0,fline)
	path_file.close()

	path_file = open(str(opt_file) + ".py","w")
	path_file.writelines(oline)
	path_file.close()

	opt_module = importlib.import_module(opt_file)

	list_opt = opt_module.list_opt[:-1] #Last Point is the Very First Point
	list_opt = list(map(int, list_opt))


	sorted_x = []
	sorted_y = []

	for i in list_opt:
		sorted_x.append(x[i])
		sorted_y.append(y[i])


	plt.plot(sorted_x,sorted_y)
	plt.xlabel('X')
	plt.ylabel('Y')
	plt.title('TSP')
	plt.show()

if __name__ == '__main__':
	tspPlot('pic.PTS','optimal_pat')