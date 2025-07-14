#
# Makefile for WDSP library
#

UNAME_S := $(shell uname -s)

PREFIX?=/usr/local
LIBDIR=$(PREFIX)/lib
INCLUDEDIR=$(PREFIX)/include

CC?=gcc
LINK?=$(CC)

CFLAGS?=-pthread -g -O3 -D_GNU_SOURCE -Wno-parentheses

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

SOURCES= amd.c\
ammod.c\
amsq.c\
analyzer.c\
anf.c\
anr.c\
bandpass.c\
calcc.c\
calculus.c\
cblock.c\
cfcomp.c\
cfir.c\
channel.c\
compress.c\
delay.c\
dexp.c\
div.c\
eer.c\
emnr.c\
emph.c\
eq.c\
fcurve.c\
fir.c\
firmin.c\
fmd.c\
fmmod.c\
fmsq.c\
gain.c\
gen.c\
icfir.c\
iir.c\
impulse_cache.c\
iobuffs.c\
iqc.c\
linux_port.c\
lmath.c\
main.c\
meter.c\
meterlog10.c\
nbp.c\
nob.c\
nobII.c\
osctrl.c\
patchpanel.c\
resample.c\
rmatch.c\
RXA.c\
sender.c\
shift.c\
siphon.c\
slew.c\
snb.c\
ssql.c \
syncbuffs.c\
TXA.c\
utilities.c\
varsamp.c\
version.c\
wcpAGC.c\
wisdom.c \
zetaHat.c

ifneq ($(NEW_NR_ALGORITHMS),)
SOURCES+= rnnr.c\
sbnr.c
endif

HEADERS=amd.h\
ammod.h\
amsq.h\
analyzer.h\
anf.h\
anr.h\
bandpass.h\
calcc.h\
calculus.h\
cblock.h\
cfcomp.h\
cfir.h\
channel.h\
comm.h\
compress.h\
delay.h\
dexp.h\
div.h\
eer.h\
emnr.h\
emph.h\
eq.h\
fastmath.h\
fcurve.h\
fir.h\
firmin.h \
fmd.h\
fmmod.h\
fmsq.h\
gain.h\
gen.h\
icfir.h\
iir.h\
impulse_cache.h\
iobuffs.h\
iqc.h\
linux_port.h\
lmath.h\
main.h\
meter.h\
meterlog10.h\
nbp.h\
nob.h\
nobII.h\
osctrl.h\
patchpanel.h\
resample.h\
resource.h\
rmatch.h\
RXA.h\
sender.h\
shift.h\
siphon.h\
slew.h\
snb.h\
ssql.h \
syncbuffs.h\
TXA.h\
utilities.h\
wcpAGC.h \
zetaHat.h

ifneq ($(NEW_NR_ALGORITHMS),)
HEADERS+= rnnr.h\
sbnr.h
endif

OBJS=linux_port.o\
amd.o\
ammod.o\
amsq.o\
analyzer.o\
anf.o\
anr.o\
bandpass.o\
calcc.o\
calculus.o\
cblock.o\
cfcomp.o\
cfir.o\
channel.o\
compress.o\
delay.o\
dexp.o\
div.o\
eer.o\
emnr.o\
emph.o\
eq.o\
fcurve.o\
fir.o\
firmin.o\
fmd.o\
fmmod.o\
fmsq.o\
gain.o\
gen.o\
icfir.o\
iir.o\
impulse_cache.o\
iobuffs.o\
iqc.o\
lmath.o\
main.o\
meter.o\
meterlog10.o\
nbp.o\
nob.o\
nobII.o\
osctrl.o\
patchpanel.o\
resample.o\
rmatch.o\
RXA.o\
sender.o\
shift.o\
siphon.o\
slew.o\
snb.o\
ssql.o \
syncbuffs.o\
TXA.o\
utilities.o\
version.o\
varsamp.o\
wcpAGC.o\
wisdom.o \
zetaHat.o

ifneq ($(NEW_NR_ALGORITHMS),)
OBJS+=rnnr.o\
sbnr.o
endif

all: $(PROGRAM) $(HEADERS) $(SOURCES)

$(PROGRAM): $(OBJS)
	$(LINK) -shared $(NOEXECSTACK) $(LDFLAGS) -o $(PROGRAM) $(OBJS) $(LIBS) $(NRLIBS)

.c.o:
	$(COMPILE) $(OPTIONS) $(CFLAGS) -c -o $@ $<

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
	-rm -f $(INCLUDEDIR)/wdsp.h
	-rm -f $(PROGRAM)
