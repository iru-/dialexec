CFLAGS=-Wall

%.o: %.c
	$(CC) $(CFLAGS) -c $<

dialexec: dialexec.o
	$(CC) $(LDFLAGS) -o $@ $<

install: dialexec
	cp -a dialexec $(HOME)/bin

clean:
	rm -f *.o dialexec
