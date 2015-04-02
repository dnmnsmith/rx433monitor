IDIR =/home/pi/PIGPIO/
CC=g++
CFLAGS=-I$(IDIR) -Wall -g

LDIR =

LIBS=-lds_notify_event -lboost_thread -lboost_system  -lboost_filesystem -lcxxtools -lcxxtools-json -L/home/PI/PIGPIO -lpigpio -lrt -lm  -pthread 

DEPS = RcOok.h ClimeMetDecoder.h

OBJ = rx433monitor.o RcOok.o ClimeMetDecoder.o

%.o: %.c $(DEPS)
	$(CC) -c $(CFLAGS) -o $@ $< 

%.o: %.cpp $(DEPS)
	$(CC) -c $(CFLAGS) -o $@ $< 


rx433monitor: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean all install

clean:
	rm -f $(OBJ) 

all: rx433monitor

install:
	cp -f rx433monitor /usr/local/sbin
	cp -f rx433monitord /etc/init.d
	@echo "To start at bootup run:"
	@echo "sudo update-rc.d rx433monitord defaults"

