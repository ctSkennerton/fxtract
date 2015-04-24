LIBS :=
EXECUTABLE := fxtract
util := util
OBJECTS := main.o fileManager.o fx.o util.o
PREFIX := /usr/local/bin

LIBZ := 1
LIBBZ2 := 1
BOOST_IOSTREAMS := -lboost_iostreams

PACKAGE_VERSION = 1.1
PACKAGE_DATE = "2015-02-19"

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

ifneq "$(wildcard .git)" ""
PACKAGE_VERSION := $(shell git describe --always)
PACKAGE_DATE := $(shell git log -n 1 --pretty="%ai" )

# Force version.h to be remade if $(PACKAGE_VERSION) has changed.
version.h: $(if $(wildcard version.h),$(if $(findstring "$(PACKAGE_VERSION)",$(shell cat version.h)),,force))
endif

version.h:
	echo '#define PACKAGE_VERSION "$(PACKAGE_VERSION)"' > $@
	echo '#define PACKAGE_DATE "$(PACKAGE_DATE)"' >> $@

force:

.PHONY: force

all: $(EXECUTABLE)

install: $(EXECUTABLE)
	$(INSTALL) -dc $< $(PREFIX)

test: $(EXECUTABLE)
	cd test/
	./run.sh

main.o: main.cpp version.h
	$(CXX) $(CFLAGS) -c -o $@ $<

$(EXECUTABLE): version.h $(OBJECTS) $(util)/libmsutil.a
	$(CXX) $(CFLAGS) -o $(EXECUTABLE) $(OBJECTS) $(util)/libmsutil.a $(LIBS) $(BOOST_IOSTREAMS)

