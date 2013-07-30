CXX=g++
LIBS=-lz -lbz2
CXXFLAGS=-Wall -Os
EXECUTABLE=fxtract

all: main.o $(EXECUTABLE)

clean:
	rm main.o fxtract

main.o: main.cpp
	$(CXX) -c $(CXXFLAGS) $(SEQAN) -o $@ main.cpp 

$(EXECUTABLE): main.o
	$(CXX) $(CXXFLAGS) $(SEQAN) -o $(EXECUTABLE) main.o $(LIBS)
