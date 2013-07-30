CXX=g++
LIBS=
CXXFLAGS=-Wall -Os
EXECUTABLE=fxtract

ifdef ZLIB
	override CXXFLAGS := $(CXXFLAGS) -DSEQAN_HAS_ZLIB
	override LIBS := $(LIBS) -lz
endif

ifdef BZIP2
	override CXXFLAGS := $(CXXFLAGS) -DSEQAN_HAS_BZIP2
	override LIBS := $(LIBS) -lbz2
endif

all: main.o $(EXECUTABLE)

clean:
	rm main.o fxtract

main.o: main.cpp
	$(CXX) -c $(CXXFLAGS) $(SEQAN) -o $@ main.cpp 

$(EXECUTABLE): main.o
	$(CXX) $(CXXFLAGS) $(SEQAN) -o $(EXECUTABLE) main.o $(LIBS)
