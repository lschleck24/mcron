CFLAGS= -Wall -Wextra -Werror -ggdb
LDFLAGS= #-lrt

prog = mcron
objects = mcron.o mu.o
headers = mu.h list.h

$(prog): $(objects)
	$(CC) -o $@ $^ $(LDFLAGS)

$(objects) : %.o : %.c $(headers)
	$(CC) -o $@ -c $(CFLAGS) $<

clean:
	rm -rf mcron $(objects)
