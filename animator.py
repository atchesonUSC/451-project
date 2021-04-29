import turtle
import sys
import time

def tCoordTocCoord(x, y):
    return x - width/2, height/2 - y

def drawEdge(cX, cY, pX, pY):
    t.penup()
    t.goto(tCoordTocCoord(pX,pY))
    t.pendown()
    t.goto(tCoordTocCoord(cX,cY))
    t.penup()

#command line args
bgFilename = str(sys.argv[1])
width = int(sys.argv[2])
height = int(sys.argv[3])
outFilename = ""
if len(sys.argv) == 5:
    outFilename = str(sys.argv[4])

#setup screen
screen = turtle.Screen()
screen.setup(width, height)
screen.bgpic(bgFilename)
screen.update()
screen.colormode(255)

#instantiate turtle
t = turtle.Turtle()
t.ht()
t.speed(10)
t.penup()
time.sleep(4)

#main loop printing the tree
if outFilename != "":
    try:
        with open(outFilename) as nodes:
            # print("animating tree")
            #print whole tree
            numNodes = int(nodes.readline())
            t.pencolor(255,0,0)
            t.pensize(3)
            line = nodes.readline()
            lineSplit = line.split()
            r = 255
            t.goto(tCoordTocCoord(int(lineSplit[0]), int(lineSplit[1])))
            for i in range(numNodes-1):
                # print("drawing node %d", i)
                line = nodes.readline()
                lineSplit = line.split()
                # print(lineSplit)
                r = r - (200.0/numNodes)
                # print("r = %d" % (r))
                t.pencolor(r,0,0)
                drawEdge(int(lineSplit[0]), int(lineSplit[1]), int(lineSplit[2]), int(lineSplit[3]))

            #print final path
            pathLength = int(nodes.readline())

            t.pencolor("green")
            t.pendown()
            for i in range(pathLength):
                line = nodes.readline()
                lineSplit = line.split()
                print(lineSplit)
                t.goto(tCoordTocCoord(int(lineSplit[0]), int(lineSplit[1])))
            t.penup()
    except Exception as e:
        print(e)

t.goto(tCoordTocCoord(10,10))

#holds screen opena nd waits to bne clicked on to exit
turtle.Screen().exitonclick()