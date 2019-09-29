import os
import importlib

#file= open("optimal_path","r")

#lines = file.readlines()
#list_point=[]
#for line in lines:
#    list_point.append(line.split(","))
#print(list_point)
#print(lines)
#file.close()
#opt_file = "optimal_path"

path_file = open(opt_file,"r")
fline = "list_opt = "
oline = path_file.readlines()
oline.insert(0,fline)
path_file.close()
path_file = open(str(opt_file) + ".py","w")
path_file.writelines(oline)
path_file.close()
	# This was specifically used for function since import name was a string and it was directly looking for the variable name as the Python File
opt_module = importlib.import_module(opt_file)

list_opt = opt_module.list_opt[:-1] # Last Point is the Very First Point
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
