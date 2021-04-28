import turtle
import sys

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

#instantiate turtle
t = turtle.Turtle()
t.penup()

#main loop printing the tree
if outFilename != "":
    try:
        with open(outFilename) as nodes:
            #print whole tree
            numNodes = int(nodes.readline())

            t.color(255, 0, 0)
            t.pensize(3)
            for i in range(numNodes):
                line = nodes.readline()
                lineSplit = line.split(",")

            #print final path
            pathLength = int(nodes.readline())

            t.color(0, 255, 0)
            for i in range(pathLength):
                line = nodes.readline()
                lineSplit = line.split(",")

    except:
        pass

t.goto(tCoordTocCoord(10,10))

#holds screen opena nd waits to bne clicked on to exit
turtle.Screen().exitonclick()