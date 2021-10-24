EXECUTABLE := fxtract
util := util
OBJECTS := main.o fileManager.o fx.o util.o kseq.o
PREFIX := /usr/local/bin

LIBZ := 1

PCRE_PATH := -lpcre
LIBZ_PATH := -lz

LIBS :=

SYSTEM_NAME := $(shell uname -s)

PACKAGE_VERSION = 2.3
PACKAGE_DATE = "2016-01-12"

ifndef NO_PCRE
	LIBS += $(PCRE_PATH)
	CFLAGS += -DHAVE_PCRE
endif

ifeq ($(LIBZ), 1)
	CFLAGS += -DHAVE_LIBZ
	LIBS += $(LIBZ_PATH)
endif


#include $(util)/GNUmakefile
$(util)/libmsutil.a:
	$(MAKE) -C $(util)

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

.PHONY: force static

all: $(EXECUTABLE)

static: $(EXECUTABLE)$(PACKAGE_VERSION)-$(SYSTEM_NAME)-64bit-static

install: $(EXECUTABLE)
	$(INSTALL) -dc $< $(PREFIX)

fxtract_test: $(EXECUTABLE)
	cd $@ && ./run.sh

main.o: main.cpp version.h
	$(CXX) $(CFLAGS) -c -o $@ $<

$(EXECUTABLE): version.h $(OBJECTS) $(util)/libmsutil.a
	$(CXX) $(CFLAGS) -o $(EXECUTABLE) $(OBJECTS) $(util)/libmsutil.a $(LIBS)

$(EXECUTABLE)$(PACKAGE_VERSION)-$(SYSTEM_NAME)-64bit-static: version.h $(OBJECTS) $(util)/libmsutil.a
	$(CXX) $(CFLAGS) -static -o $@ $(OBJECTS) $(util)/libmsutil.a $(LIBS)

