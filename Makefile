CFLAGS	= -Wall -Wextra -O3
LDFLAGS	= 
LIBS	= 

rekotoppm: rekotoppm.c
	gcc $(CFLAGS) $(LDFLAGS) -o $@ $< $(LIBS)
