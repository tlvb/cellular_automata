# cellullar\_automata
Simulate Conway's Game of Life and other automata
that have two states and work in a Manhattan
neighbourhood.

The world is implemented as a torus.

## how to build the program
On linux:

    make

...after making sure that you have everything necessary to build it, which should be
general c compiling stuff and SDL 2 unless you want to build without gui support
in which case compile as

    make nogui

## how to run the program
./cellular\_automata \[**-r ruleset**\] \[**-n N** \[**-o file** | **-O fmt**\]\] **seed\_file**  
	**-r ruleset** is expected on the form surviveset/birthset e.g. -r 23/3  
	   for Conway's Game of Life (which is what is selected if -r is omitted).  
	**-n** is for benchmark/non-interactive mode, evolving the seed N times while measuring the time.  
	   Time is not reported when -O is active because that would be pointless.  
	   NOTE: if -n is NOT set, graphics mode is selected, and the automaton runs until  
	   the window is closed.  
	**-o** will write the final frame to the assigned file in raw NetPBM format  
	**-O** will write every frame to enumerated files (e.g. -O frame_%04d.pbm)  
	   NOTE: -o/-O has no effect when the program runs in graphics mode.

### examples
	mkdir out
    ./cellular_automata -n 32 -O out/pentomino_frame_%02d.pbm examples/r-pentomino
    ./cellular_automata -r 23/3 examples/r-pentomino-large

## format of the seed file
Currently two seed file formats are accepted:

* A simple text file.  
The file should begin with "txt", followed by width, height, x-offset and y-offset, all separated by whitespace,
and then the data. the offset values are for if you want to e.g. make a big world and place a small
seed kernel in the middle and not have to draw all the empty space around it. The data is one character
per cell, '#' denotes a live cell, anything else is a dead cell, except for newlines ('\n'), which
increases the y position, and resets the x position to the offset value.
*Some seed files in this format are available in the examples/ directory.*

* NetPBM raw bitmap (P4)

*NOTE:* Because of how the world and associated functions are implemented, only world widths divisible
by eight are possible, and any seed file not conforming with this will have the width rounded up.
This is especially important when using NetPBM seeds, as any pad bits at the end of a row
are *not* scrubbed.
