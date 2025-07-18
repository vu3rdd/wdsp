#
# Makefile for WDSP library
#

UNAME_S := $(shell uname -s)

PREFIX ?= /usr/local

debug ?= 0
CC?=gcc
LINK?=$(CC)

LIBDIR=$(PREFIX)/lib
INCLUDEDIR=$(PREFIX)/include

CFLAGS?=-pthread -O3 -D_GNU_SOURCE -Wno-parentheses

FFTWINCLUDE=`pkg-config --cflags fftw3`
FFTWLIB=`pkg-config --libs fftw3`

ifneq ($(NEW_NR_ALGORITHMS),)
NRINCLUDES=`pkg-config --cflags rnnoise`
NRINCLUDES+=`pkg-config --cflags libspecbleach`

NRLIBS=`pkg-config --libs rnnoise`
NRLIBS+=`pkg-config --libs libspecbleach`

CFLAGS+=-DNEW_NR_ALGORITHMS
endif

ifeq ($(UNAME_S), Darwin)
PROGRAM=libwdsp.dylib
NOEXECSTACK=
else
CFLAGS+=-fPIC
PROGRAM=libwdsp.so
NOEXECSTACK=-z noexecstack
endif

LIBS=$(FFTWLIB)

COMPILE=$(CC) $(CFLAGS) $(FFTWINCLUDE) $(NRINCLUDES)
ifeq ($(debug), 1)
	CFLAGS := $(CFLAGS) -g -O0
else
	CFLAGS := $(CFLAGS) -Oz
endif
CFLAGS := $(CFLAGS) $(FFTWINCLUDE) $(NRINCLUDES)

SRC_DIR := `pwd`

OBJS := $(patsubst %.c,%.o, $(wildcard *.c))
OBJS := $(filter-out make_%, $(OBJS))

all: $(PROGRAM) $(HEADERS) $(SOURCES)

$(PROGRAM): $(OBJS)
	$(LINK) -shared $(NOEXECSTACK) $(LDFLAGS) -o $(PROGRAM) $(OBJS) $(LIBS) $(NRLIBS)

%.o: %.c %.h
	$(CC) $(CFLAGS) -c -o $@ $<

install-dirs:
	 mkdir -p $(LIBDIR) $(INCLUDEDIR)

install: $(PROGRAM) install-dirs
	cp wdsp.h $(INCLUDEDIR)
ifeq ($(UNAME_S), Darwin)
	/usr/bin/install_name_tool -id $(LIBDIR)/$(PROGRAM) $(PROGRAM)
	cp $(PROGRAM) $(LIBDIR)
else
	cp $(PROGRAM) $(LIBDIR)
	ldconfig || :
endif

clean:
	-rm -f *.o
	-rm -f $(PROGRAM)

uninstall: clean
	-rm -f $(INCLUDEDIR)/wdsp.h
	-rm -f $(LIBDIR)/$(PROGRAM)
