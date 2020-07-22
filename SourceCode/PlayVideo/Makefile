IDIR =.
CC=g++
CFLAGS=-I$(IDIR) -O2

ODIR=.

LDFLAGS=-s
LIBS=-lm -lwiringPi

TARGET = PlayVideo

_DEPS = ListManager.h PlayVideo.h
DEPS  = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = main.o ExecuteCommand.o ListManager.o PlayVideo.o
OBJ  = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

PlayVideo: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	/bin/rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 

install: $(TARGET)
	sudo /bin/cp PlayVideo /usr/bin
