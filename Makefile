CC=gcc
CFLAGS=--std=gnu99 -Wextra -O2
vpath %.c src/
vpath %.h src/

.PHONY: default
default: cellular_automata

cellular_automata: main.c ca.o
	$(CC) $(CFLAGS) -o $@ $(filter-out %.h, $^)

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $<

.PHONY: clean
clean:
	-rm *.o
.PHONY: cleaner
cleaner: clean
	-rm cellular_automata
