# cellullar\_automata
A program to simulate Conway's Game of Life and other cellular automata that
thrive on a torus and use a Moore neighborhood.

## how to build the program
On linux:

    make

...after making sure that you have everything necessary to build it, which should be
general c compiling stuff and SDL 2 unless you want to build without gui support
in which case compile as

    make nogui

## how to run the program

<!-- autogen begin -->
COMMAND LINE PARAMETERS

    cellular_automata [-h] [-r ruleset] [-n N [-o file | -O format]] [-w W H | seed_file]

    -h         - Print this help text and exit.

    -r ruleset - Automaton ruleset on the form surviveset/birthset, e.g. 23/3 for
                 Conway's Game of Life (which is assumed if the parameter is omitted).

    -n N       - Run in batch mode for given number of turns. If omitted the program
                 will start in interactive graphics mode.

    -o file    - In batch mode, write the last generation to the assigned file in
                 raw NetPBM (P4) format.

    -O format  - In batch mode, write all frames to enumerated files described by
                 the given format. Example: -O frame_%04d.pbm will give frame_0000.pbm, frame_0001.pbm etc.

    -w W H     - create an empty world of given height and width.
                 The width will be rounded up to the nearest number divisible by eight.

    seed_file  - Load a world from a seed file, either in raw NetPBM (P4) format, or from a plain
                 text file with the format described below.

                 If neither a -w parameter nor a seed file has been specified, an empty world of 100x100 cells
                 is created.

PLAIN TEXT SEED FILE FORMAT

    The file should begin with "txt" followed by (white space separated) width, height,
    horizontal offset, vertical offset, and then data.
    The offset values are for if you want to e.g. make a big world and place a small seed kernel
    in the middle and not have to draw all the empty space around it. The data is one character per
    cell, '#' denotes a live cell, anything else is a dead cell, except for newlines ('\n'), which
    increases the y position, and resets the x position to the offset value.

INTERACTIVE MODE

    The Interactive mode starts in a paused state. Pressing the space bar toggles between paused and
    running state. It is also possible to increase or decrease the delay between iterations with the
<!-- autogen end -->

## examples

There are some example seeds in the example directory.

    mkdir out
    ./cellular_automata -n 32 -O out/pentomino_frame_%02d.pbm examples/r-pentomino
    ./cellular_automata -r 23/3 examples/r-pentomino-large
    ./cellular_automata -w 100 100
