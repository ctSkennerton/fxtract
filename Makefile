LIBS := -lz
EXECUTABLE := fxtract
util := util
OBJECTS := main.o fileManager.o fx.o util.o
PREFIX := /usr/local/bin

ifdef PCRE
	LIBS += -lpcre
	CFLAGS += -DHAVE_PCRE
endif

include $(util)/GNUmakefile

all: $(EXECUTABLE)

install: $(EXECUTABLE)
	$(INSTALL) -dc $< $(PREFIX)

test: $(EXECUTABLE)
	cd test/
	./run.sh

$(EXECUTABLE): $(OBJECTS) $(util)/libmsutil.a
	$(CXX) $(CFLAGS) -o $(EXECUTABLE) $^ $(LIBS)
