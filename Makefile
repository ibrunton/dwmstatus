# dwmstatus makefile
# 2012-07-03 16:50
# by Ian D Brunton <iandbrunton at gmail dot com>

APPNAME=dwmstatus
VERSION=0.1a
CC=gcc
CFLAGS=-c -g -Wall -DVERSION=\"$(VERSION)\" -DAPPNAME=\"$(APPNAME)\" \
       -lX11 -DLAPTOP
LDFLAGS=-lX11
SRC=main.c
OBJ=$(SRC:.c=.o)

all: $(APPNAME)

$(APPNAME): $(OBJ)
	$(CC) $(LDFLAGS) -Wl,--start-group $(OBJ) -Wl,--end-group -o $@

.c.o:
	$(CC) $(CFLAGS)  $< -o $@

install: $(APPNAME)
	install -D -m 755 -o ian -g users $(APPNAME) $(HOME)/bin

clean:
	rm -rf *.o $(APPNAME)
