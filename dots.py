import sys
sys.path.remove('/opt/ros/kinetic/lib/python2.7/dist-packages')

import cv2 as cv2
import numpy as np
import matplotlib.pyplot as plt

def rgb2gray()

im = cv2.imread("ryan.jpg")
im_black = cv2.rgb2gray(im)
imshow(im_black)
input()
pixels = np.argwhere(im == 255)
print(len(pixels))


x,y,z = pixels.T

plt.scatter(x,y)
plt.show() 