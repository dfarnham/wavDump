CC     = gcc
CFLAGS = -O2
RM     = rm -f

wavDump: wavDump.c
	$(CC) $(CFLAGS) wavDump.c -o $@

clean:
	$(RM) wavDump wavDump.o
