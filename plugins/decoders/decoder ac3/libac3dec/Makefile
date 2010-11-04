OBJDIR=.
OUTNAME=./libac3dec.a
SONAME =libac3dec.a
INCPATH=-I./
DEFINE=-D_DEBUG -D_REENTRANT -D__USE_LARGEFILE64 
COMPILE=gcc -g -Wall $(INCPATH) $(DEFINE) -c -o
LINK=ar cr $(OUTNAME)

LIBOBJS=$(OBJDIR)/bit_allocate.o \
		$(OBJDIR)/bitstream.o \
		$(OBJDIR)/crc.o \
		$(OBJDIR)/dither.o \
		$(OBJDIR)/downmix.o \
		$(OBJDIR)/exponent.o \
		$(OBJDIR)/imdct.o \
		$(OBJDIR)/libac3dec.o \
		$(OBJDIR)/mantissa.o \
		$(OBJDIR)/parse.o \
		$(OBJDIR)/rematrix.o


all : $(OUTNAME)
$(OUTNAME) : $(LIBOBJS)
	$(LINK)	$(LIBOBJS)
	ranlib $(OUTNAME)

$(OBJDIR)/bit_allocate.o : bit_allocate.c 
	$(COMPILE) $(OBJDIR)/bit_allocate.o bit_allocate.c

$(OBJDIR)/bitstream.o : bitstream.c
	$(COMPILE) $(OBJDIR)/bitstream.o bitstream.c

$(OBJDIR)/crc.o : crc.c
	$(COMPILE) $(OBJDIR)/crc.o crc.c

$(OBJDIR)/dither.o : dither.c
	$(COMPILE) $(OBJDIR)/dither.o dither.c

$(OBJDIR)/downmix.o : downmix.c
	$(COMPILE) $(OBJDIR)/downmix.o downmix.c

$(OBJDIR)/exponent.o : exponent.c
	$(COMPILE) $(OBJDIR)/exponent.o exponent.c

$(OBJDIR)/imdct.o : imdct.c
	$(COMPILE) $(OBJDIR)/imdct.o imdct.c

$(OBJDIR)/libac3dec.o : libac3dec.c
	$(COMPILE) $(OBJDIR)/libac3dec.o libac3dec.c

$(OBJDIR)/mantissa.o : mantissa.c
	$(COMPILE) $(OBJDIR)/mantissa.o mantissa.c

$(OBJDIR)/parse.o : parse.c
	$(COMPILE) $(OBJDIR)/parse.o parse.c

$(OBJDIR)/rematrix.o : rematrix.c
	$(COMPILE) $(OBJDIR)/rematrix.o rematrix.c

clean :
	rm $(LIBOBJS) $(OUTNAME)


