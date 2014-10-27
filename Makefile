LIBS := -lboost_iostreams
EXECUTABLE := fxtract
util := util
OBJECTS := main.o fileManager.o fx.o util.o
PREFIX := /usr/local/bin

LIBZ := 0
LIBBZ2 := 0

ifndef NO_PCRE
	LIBS += -lpcre
	CFLAGS += -DHAVE_PCRE
endif

ifeq ($(LIBZ), 1)
	CFLAGS += -DHAVE_LIBZ
endif

ifeq ($(LIBBZ2), 1)
	CFLAGS += -DHAVE_LIBBZ2
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
