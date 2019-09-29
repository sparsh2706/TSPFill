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

    print(len(list_point))
    file.close()

    #plt.plot(x,y)
    #plt.xlabel('X')
    #plt.ylabel('Y')
    #plt.title('Normal PTS file')
    #plt.show()

    path_file = open(opt_file,"r")
    #fline = "list_opt = "
    oline = path_file.readlines()
    #oline.insert(0,fline)
    path_file.close()
    #print(oline)
    #path_file = open(str(opt_file) + ".py","w")
    #path_file.writelines(oline)
    #path_file.close()
    #oline = [int(i) for i in oline]
    #oline = list(map(int, oline))
    order = re.findall(r'[+-]?\d+(?:\.\d+)?', oline[0])
    #path2_file = open(str(opt_file) + ".py","r")
    #print(order)
    order = [int(i) for i in order]
    #gline = path2_file.readlines()
   #s print(gline)
    # This was specifically used for function since import name was a string and it was directly looking for the variable name as the Python File
    #opt_module = importlib.import_module(opt_file)

    #list_opt = opt_module.list_opt[:-1] # Last Point is the Very First Point
    
    #list_opt = list(map(int, list_opt))
    #print(list_opt)
    #print(len(list_opt))
    #print(len(x))

    sorted_x = []
    sorted_y = []

    for i in order:
        
        sorted_x.append(x[i])
        sorted_y.append(y[i])

    sorted_x = [float(i) for i in sorted_x]
    sorted_y = [float(i) for i in sorted_y]
    
    axes = plt.gca()
    #xmax = max(x)
    #xmin = min(x)
    #print(xmin)
    
    #ymax = max(y)
    #print(sorted_y)
    

    #axes.set_xlim([xmin,xmax])
    #axes.set_ylim([ymin,ymax])
    #plt.xlabel('X')
    #plt.ylabel('Y')
    #plt.title('TSP')
    
    
    return sorted_x,sorted_y
if __name__ == '__main__':
    tspPlot('pic4.PTS','optimal_path')
