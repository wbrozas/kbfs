# ################################################################
# Makefile
# ################################################################


CC = gcc
CFLAGS = `pkg-config fuse --cflags --libs`


all: mount.kbfs

debug:
	make DEBUG=TRUE

mount.kbfs: mount.kbfs.c
	$(CC) $(CFLAGS) -o mount.kbfs mount.kbfs.c

ifeq ($(DEBUG), TRUE)
CC += -g
endif
clean:
	rm -f mount.kbfs
