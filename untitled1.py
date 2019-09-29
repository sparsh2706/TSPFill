import matplotlib.pyplot as plt
import numpy as np
import re
import matplotlib.path as mplPath

#import tspart
# Last and First Co-ordinate is same

def boundary(filename):
    """ Enter the Filename through input and plot for boundary would be generated """

    file = open(filename,'r')
    x = []
    y = []
    lines = file.readlines()
    length = 0
    list_point=[]
    for line in lines:
        list_point = line.split(" ")
        x.append(list_point[0])
        y.append(list_point[1])
    
    length = len(list_point)
    #print(length)
    
    i=0
    while(i<length):
        x.append(list_point[i])
        i+=1
        if(i>=length):
            break
        y.append(list_point[i])
        i+=1

    # Change from str list to int list
    x = [float(i) for i in x]
    y = [float(i) for i in y]
    #print(x)

    file.close()

    return x,y


if __name__ == '__main__':
    
    #x,y = boundary(input("Enter File Name with Extension\n"))
    x,y = boundary("b.txt")
    
    
    if(len(x)==len(y)):
        print("Co-ordinates Loaded :) ")
        print()

    points = []
    for i in range(len(x)):
        points.append([x[i],y[i]])

    print("Points List has been made\n")

    plt.plot(x,y)

    axes = plt.gca()
    plt.axis('scaled')
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

    for i in range(int(x_init),int(x_end),int(2)):
        for j in range(int(y_init),int(y_end),int(2)):
            all_points.append([i,j])
            #plt.scatter(i,j,c='b',s=2)

    #plt.axis('scaled')
    #plt.show()
    #print(points)
    inside_points=[]
    p= mplPath.Path(points)
    for point in all_points:
        inside=p.contains_point((point))
        if(inside == True):
            inside_points.append([point[0],point[1]])
            plt.scatter(point[0],point[1],c='b',s=2)
            #print(point[0],point[1])
    
    #print(inside_points)
    plt.axis('scaled')
    plt.show()
    #print(inside_points[0][0])
    f= open("pic4.PTS","w")
    f.write("# x-coord y-coord radius\n")
    i=0
    for point in inside_points:
        #print(point)
        f.write(str(point[0])+" "+str(point[1])+"\n")
        
    #print(inside_points)
    
    import tspart
    #import plot_points
    #plot_points.tspPlot('pic5.PTS','optimal_path')
    
    
    
    