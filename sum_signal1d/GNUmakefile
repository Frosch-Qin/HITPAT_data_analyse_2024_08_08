PROGNAME = sum_signal1d
SOURCEFILES = sum_signal1d.c
OBJS    = $(patsubst %.c, %.o, $(SOURCEFILES))

ROOTCFLAGS   := $(shell root-config --cflags)
ROOTLIBS     := $(shell root-config --libs)
ROOTGLIBS    := $(shell root-config --glibs)

GSLCFLAGS   := $(shell gsl-config --cflags)
GSLLIBS     := $(shell gsl-config --libs)
GSLGLIBS    := $(shell gsl-config --glibs)



LDFLAGS       = -O
LIBS         += $(ROOTLIBS) $(GSLLIBS)
CFLAGS       += $(ROOTCFLAGS) $(GSLCFLAGS)

#  Not sure why Minuit isn't being included -- put in by hand
# 
LIBS         += -lMinuit
#for gmp the big number library
LIBS         += -lgmp
LIBS         += -lgmpxx	


%.o: %.c
	g++ ${CFLAGS} -c  -g -o $@ $<

$(PROGNAME):    $(OBJS)
	g++ -o $@ $(OBJS) $(LDFLAGS) $(LIBS)

test:
	@echo $(ROOTCFLAGS)

clean:	
	-rm -f ${PROGNAME} ${OBJS}

