import matplotlib.pyplot as plt
import numpy as np
import re
import matplotlib.path as mplPath
#import importlib
from shapely.geometry import Polygon
from shapely.geometry import Point
#import os

#import tspart
# Last and First Co-ordinate is same
def drange(start, stop, step):
    while start < stop:
            yield start
            start += step
            
def PolyArea(x,y):
    return 0.5*np.abs(np.dot(x,np.roll(y,1))-np.dot(y,np.roll(x,1)))        
            
            
def boundary(filename):
    """ Enter the Filename through input and plot for boundary would be generated """

    file = open(filename,'r')
    x = []
    y = []
    lines = file.readlines()
    #length = 0
    
    for line in lines:
        #list_point = line.split(" ")
        order = re.findall(r'[+-]?\d+(?:\.\d+)?', line)
    #path2_file = open(str(opt_file) + ".py","r")
    #print(order)
        order = [float(i) for i in order]
        #list_point = line.split("\t")
        x.append(order[0])
        y.append(order[1])
    
    #length = len(list_point)
    #print(length)
    
    #i=0
   # while(i<length):
    #    x.append(list_point[i])
   #     i+=1
   #     if(i>=length):
    #        break
    #    y.append(list_point[i])
    #    i+=1

    # Change from str list to int list
    x = [float(i) for i in x]
    y = [float(i) for i in y]
    #print(x)

    file.close()

    return x,y


def Digitize(file_name,stepover):
    
    #x,y = boundary(raw_input("Enter file\n"))
    #x,y = boundary("b (2).txt")
    x,y = boundary(file_name)
    #stepover = input("Enter the stepover distance\n")
    stepover = float(stepover)
    
    if(len(x)==len(y)):
        print("Co-ordinates Loaded :) ")
        #print(len(x))

    points = []
    for i in range(len(x)):
        points.append([x[i],y[i]])
    orig_poly=Polygon(points)
    area_orig = orig_poly.area
    perimeter_orig = orig_poly.length
    print("Points List has been made\n")
    #area=PolyArea(x,y)
    #print(area)
    print("Area of Boundary = " + str(area_orig))
    print("Perimeter of Boundary = " + str(perimeter_orig))
    
    #p = Point(35,-120)
    #d= p.distance(orig_poly)  
    #print(d) 

    plt.plot(x,y)

    axes = plt.gca()
    plt.axis('scaled')
    plt.savefig('figure.png')
    plt.close()
    #plt.show()

    xmin = min(x)-stepover
    xmax = max(x)+stepover
    ymin = min(y)-stepover
    ymax = max(y)+stepover

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
    print("\n\nDigitizing.....")

    all_points = []

    for i in drange(float(xmin),float(xmax),stepover):
        for j in drange(int(ymin),int(ymax),stepover):
            all_points.append([i,j])
            #plt.scatter(i,j,c='b',s=2)
    
    #plt.axis('scaled')
    #plt.show()
    #print(points)
    inside_points=[]
    p= mplPath.Path(points)
    for point in all_points:
        inside=p.contains_point((point))
        e = Point(point)
        d= e.distance(orig_poly)
        if(inside == True):
            inside_points.append([point[0],point[1]])
            plt.scatter(point[0],point[1],c='b',s=2)
        elif (d<= stepover):
            inside_points.append([point[0],point[1]])
            plt.scatter(point[0],point[1],c='b',s=2)
            #print(point[0],point[1])
    
    #print(inside_points)
    plt.axis('scaled')
    plt.savefig('figure' + str(int(stepover)) + '.png')
    plt.close()
    #plt.show()
    print("Digitization complete\n")
    print("running TSP")
    #print(inside_points[0][0])
    f= open("pic4.PTS","w")
    f.write("# x-coord y-coord radius\n")
    i=0
    for point in inside_points:
        #print(point)
        f.write(str(point[0])+" "+str(point[1])+"\n")
    f.close()
    #print(inside_points)
    
    import tspart
    tspart.TSP('','')
    print("plotting result")
    import plot_points
    sorted_x,sorted_y = plot_points.tspPlot('pic4.PTS','optimal_path')
    
    plt.plot(sorted_x,sorted_y)
    #plt.plot(x,y)
    plt.axis('scaled')
    plt.savefig('figure' + str(int(stepover)) + '_' + str(int(stepover))+ '.png')
    plt.close()
    #plt.show()
    
    sorted_points = []
    for i in range(len(sorted_x)):
        sorted_points.append([sorted_x[i],sorted_y[i]])
    final_poly=Polygon(sorted_points)
    area_final = final_poly.area
    perimeter_final = final_poly.length
    print(area_final)
    print(perimeter_final)
    
    effeciency = area_orig/((perimeter_final)*stepover)
    
    return effeciency
    

    
    
if __name__ == '__main__':
    file_name = raw_input("Enter file\n")
    #file_name = "b (2).txt"
    #stepover = input("Enter the stepover distance\n")
    #effeciency = Digitize(file_name,stepover)
    e=[1]
    s=[0]
    xmax = 9
    for i in range(1,xmax,1):
        e.append(Digitize(file_name,i))
        s.append(i)
        print("done for stepover " +str(i))
    print(e)
    print(s)
    
    plt.plot(s,e)
    axes = plt.gca()
    axes.set_xlim([0,xmax])
    axes.set_ylim([0,1])
    plt.xlabel('stepover')
    plt.ylabel('effeciency')
    #plt.axis('scaled')
    #plt.show()
    plt.savefig('effeciency.png')
    plt.close()
       
    #print(effeciency)
    
    input("press any key to exit")
