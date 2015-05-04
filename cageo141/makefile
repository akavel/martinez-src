CXX = g++
CXXFLAGS = -O3
LDFLAGS = -lm
TARGET = clip
OBJS = $(TARGET).o greiner.o polygon.o timer.o utilities.o connector.o gpc.o martinez.o

$(TARGET): $(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LDFLAGS)

greiner.o: greiner.cpp greiner.h utilities.h
	$(CXX) -c greiner.cpp $(CXXFLAGS)

polygon.o: polygon.cpp polygon.h utilities.h
	$(CXX) -c polygon.cpp $(CXXFLAGS)

timer.o: timer.cpp timer.h
	$(CXX) -c timer.cpp $(CXXFLAGS)

utilities.o: utilities.cpp utilities.h segment.h
	$(CXX) -c utilities.cpp $(CXXFLAGS)

connector.o: connector.cpp connector.h
	$(CXX) -c connector.cpp $(CXXFLAGS)

gpc.o: gpc.cpp gpc.h
	$(CXX) -c gpc.cpp $(CXXFLAGS)

martinez.o: martinez.cpp martinez.h connector.h
	$(CXX) -c martinez.cpp $(CXXFLAGS)

$(TARGET).o: $(TARGET).cpp polygon.h  utilities.h martinez.h connector.h greiner.h gpc.h 
	$(CXX) -c $(TARGET).cpp $(CXXFLAGS)

clean:
	rm $(TARGET) $(OBJS)
