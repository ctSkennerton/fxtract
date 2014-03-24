CC := cc
LZ := -lz
CFLAGS := -Wall -O2 -ggdb
EXECUTABLE := fxtract
OBJECTS := main.o fileManager.o fx.o pq.o sds/sds.o util.o aho-corasick/msutil.o
PREFIX := /usr/local/bin
acism := aho-corasick

include aho-corasick/GNUmakefile


all: $(EXECUTABLE)

install: $(EXECUTABLE)
	$(INSTALL) -dc $< $(PREFIX)

clean: $(OBJECTS)
	-rm $(OBJECTS)

test: $(EXECUTABLE)
	cd ../test/
	./run.sh

$(EXECUTABLE): $(OBJECTS) $(acism)/libacism.a
	$(CC) $(CFLAGS) -o $(EXECUTABLE) $^ $(LZ)
