#
#

TARGETS=ShutDown

IDIR =.
CC=g++
CFLAGS=-I$(IDIR) -O2

INSTALL=/usr/bin/install

ODIR=.

LDFLAGS=-s
LIBS=-lm -lwiringPi

_DEPS = 
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = main.o 
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

ShutDown: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 

install: $(TARGETS)
	sudo $(INSTALL) -m 755 ShutDown /usr/bin

.PHONY: clean
.PHONY: install

