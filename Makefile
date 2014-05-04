CC := cc
LZ := -lz
CFLAGS := -Wall -O2 -ggdb
EXECUTABLE := fxtract
util := util
OBJECTS := main.o fileManager.o fx.o util.o
PREFIX := /usr/local/bin

include $(util)/GNUmakefile


all: $(EXECUTABLE)

install: $(EXECUTABLE)
	$(INSTALL) -dc $< $(PREFIX)

clean: $(OBJECTS)
	-rm $(OBJECTS)

test: $(EXECUTABLE)
	cd ../test/
	./run.sh

$(EXECUTABLE): $(OBJECTS) $(util)/libmsutil.a
	$(CXX) $(CFLAGS) -o $(EXECUTABLE) $^ $(LZ)
