CC= gcc
DEFS= -DFPM_INTEL
AR= ar cq
RANLIB= ranlib
LIBNAME= libmad.a

INCLUDES=  -I.

# replace -O with -g in order to debug

DEFINES= $(INCLUDES) $(DEFS)
CFLAGS= -O3 $(DEFINES)

SRCS = bit.c decoder.c fixed.c frame.c huffman.c layer12.c layer3.c stream.c synth.c timer.c version.c

OBJS = bit.o decoder.o fixed.o frame.o huffman.o layer12.o layer3.o stream.o synth.o timer.o version.o

.c.o:
	$(CC) $(CFLAGS) -c $*.c

all: $(LIBNAME)

$(LIBNAME) : $(OBJS)
	$(AR) $@ $(OBJS)
	$(RANLIB) $@


clean:
	rm -f $(OBJS) $(LIBNAME) core
