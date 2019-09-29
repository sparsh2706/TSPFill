#!/usr/bin/env python3

import turtle
from random import randint

RECTANGLE_SIZE = 60, 80
CIRCLE_RADIOUS = 10
DOT_DIAMETER   = 3

t = turtle.Turtle() # turtle object
t.speed(0)          # set the fastest drawing speed

# move turtle to position without drawing
# @param: position is a touple containing `x` and `y` coordinates
def move_turtle_to(position):
    t.up()   # equivalent to .penuo()
    t.goto(position[0], position[1])
    t.down() # equivalent to .pendown()


# draws a rectangle from given origin with given size
# @param origin: is a touple, containing `x` and `y` coordinates
# @param size:   is a touple, containing `width` and `height` of rectangle
def draw_rectangle(origin, size=RECTANGLE_SIZE):
    # movese to the origin
    move_turtle_to(origin)
 # simple way of drawing a rectangle
    for i in range(4):
        t.fd(size[i % 2])
        t.left(90)

# draws a circle from given origin with given radious
# @param origin: is a touple, containing `x` and `y` coordinates
# @param radious: int, radious of circle
def draw_circle(origin, radius=CIRCLE_RADIOUS):
    # moves to the origin
    move_turtle_to(origin)
    # draws the circle
    t.circle(radius)
# Now to what you asked
# draw random dots inside of rectangle
# @param origin: is a touple, containing `x` and `y` coordinates
# @param number_of_dots: int, number of dots
# @param size:   is a touple, containing `width` and `height` of rectangle
def draw_random_dots_in_rectangle(origin, number_of_dots, size=RECTANGLE_SIZE):
    # loops number_of_dots times
    for _ in range(number_of_dots):
        # generate a random position inside of given rectangle
        # using min/max, because of possible negative coordinates
        # weakness - does also place dots on the edges of the rectangle
        rand_x = randint(min(origin[0], origin[0] + size[0]),     max(origin[0], origin[0] + size[0]))
        rand_y = randint(min(origin[1], origin[1] + size[1]),     max(origin[1], origin[1] + size[1]))
        # moves to the random position
        move_turtle_to((rand_x, rand_y))
        # creates a dot
        t.dot(DOT_DIAMETER)

# draw random dot inside of circle
# @param origin: is a touple, containing `x` and `y` coordinates
# @param number_of_dots: int, number of dots
# @param radious: int, radious of circle
def draw_random_dots_in_circle(origin, number_of_dots, radius=CIRCLE_RADIOUS):
    # loops number_of_dots times
    for _ in range(number_of_dots):
        # loops until finds position inside of the circle
        while True:
            # generates random x position
            # subtracting radious and adding double of radious to simulate bounds of square
            # which would be large enought to fit the circle
            rand_x = randint(min(origin[0] - radius, origin[0] + radius * 2),
                             max(origin[0] - radius, origin[0] + radius * 2))
            # generated random y position
            # adding  double of radious to sumulate bounds of square
            # which would be large enought to fit the circle
            rand_y = randint(min(origin[1], origin[1] + radius * 2),
                             max(origin[1], origin[1] + radius * 2))

            # test if the generated position is in the radious
            if (origin[0] - rand_x) ** 2 + (origin[1] + radius - rand_y) ** 2 < radius ** 2:
                # if it is, move to the position
                move_turtle_to((rand_x, rand_y))
                # draw dot
                t.dot(DOT_DIAMETER)
                # break out from the infinite loops
                break
# example code
draw_rectangle((0, 0))

draw_random_dots_in_rectangle((0, 0), 50)

draw_circle((-20, -20))

draw_random_dots_in_circle((-20, -20), 20)

input()