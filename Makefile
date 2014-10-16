CC=gcc
CFLAGS=--std=gnu99 -Wextra -O2
vpath %.c src/
vpath %.h src/

.PHONY: default
default: cellular_automata

cellular_automata: main.c ca.o io.o gui.o bitmap.o help.h
	$(CC) $(CFLAGS) -o $@ $(filter-out %.h, $^) -lSDL2

nogui: main.c ca.o io.o bitmap.o help.h
	$(CC) $(CFLAGS) -o cellular_automata -DNOGUI $(filter-out %.h, $^)

sdl_gui.o: gui.c gui.h ca.h bitmap.h
	$(CC) $(CFLAGS) -c $<
io.o: io.c io.h ca.h bitmap.h
	$(CC) $(CFLAGS) -c $<
ca.o: ca.c ca.h bitmap.h
%.o: %.c %.h
	$(CC) $(CFLAGS) -c $<

.PHONY: clean
clean:
	-rm *.o
.PHONY: cleaner
cleaner: clean
	-rm cellular_automata
