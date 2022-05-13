CFLAGS=-Wall -pedantic -std=c11 -I.

all: us uc

us:	us.c glue.h
		gcc $(CFLAGS) us.c -o us

uc: uc.c glue.h
		gcc $(CFLAGS) uc.c -lm -o uc

# Install the server on the 68ARM2
# Run: ./us
# Prints a message if any file that is uploaded.
install: us
		cp -f us ~/us

clean:
		rm -f *~ uc us
