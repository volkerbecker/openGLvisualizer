CXX = g++
LMA = ar crs
RLIB = ranlib

CXXFLAGS =	-O2  -Wall -fmessage-length=0  -std=c++11
#CXXFLAGS =	-O0 -g -Wall -fmessage-length=0  -std=c++11
OBJS =		openGLvisulizer.o Visualizer.o
LOBJS =		Visualizer.o

LIBS =      -lGL -lsfml-system -lsfml-window -lsfml-graphics -lGLEW -lSOIL -pthread 

TARGET =	libminglvisualizer.a

secondTarget = openGLvisulizer

$(TARGET):	$(OBJS)
	$(LMA) $(TARGET) $(LOBJS)
	$(RLIB) $(TARGET)
	$(CXX) -o $(secondTarget) $(OBJS) $(LIBS)
	
%.o:	%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) $(CLFLAGS) -c $*.cpp -o $*.o
	$(CXX) -MM $(CXXFLAGS) $(INCLUDE) $(CLFLAGS) $*.cpp > $*.d

-include $(OBJS:.o=.d)

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET) $(secondTarget)