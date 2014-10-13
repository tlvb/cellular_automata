#ifndef __HELP_H__
#define __HELP_H__

#define HELPSECTION \
	"\x1b[1;4mCOMMAND LINE PARAMETERS\x1b[0m\n" \
	"\n" \
	"    cellular_automata [\x1b[1m-r\x1b[0m \x1b[4mruleset\x1b[0m] [\x1b[1m-n\x1b[0m \x1b[4mN\x1b[0m [\x1b[1m-o\x1b[0m \x1b[4mfile\x1b[0m | \x1b[1m-O\x1b[0m \x1b[4mformat\x1b[0m]] {\x1b[1m-w\x1b[0m \x1b[4mW\x1b[0m \x1b[4mH\x1b[0m | \x1b[4mseed_file\x1b[0m}\n" \
	"\n" \
	"    \x1b[1m-r\x1b[0m \x1b[4mruleset\x1b[0m - Automaton ruleset on the form surviveset/birthset, e.g. 23/3 for\n" \
	"                 Conway's Game of Life (which is assumed if the parameter is omitted).\n" \
	"\n" \
	"    \x1b[1m-n\x1b[0m \x1b[4mN\x1b[0m       - Run in batch mode for given number of turns. If omitted the program\n" \
	"                 will start in interactive graphics mode.\n" \
	"\n" \
	"    \x1b[1m-o\x1b[0m \x1b[4mfile\x1b[0m    - In batch mode, write the last generation to the assigned file in\n" \
	"                 raw NetPBM (P4) format.\n" \
	"\n" \
	"    \x1b[1m-O\x1b[0m \x1b[4mformat\x1b[0m  - In batch mode, write all frames to enumerated files described by\n" \
	"                 the given format. Example: -O frame_%%04d.pbm will give frame_0000.pbm, frame_0001.pbm etc.\n" \
	"\n" \
	"    \x1b[1m-w\x1b[0m \x1b[4mW\x1b[0m \x1b[4mH\x1b[0m     - create an empty world of given height and width.\n" \
	"                 The width will be rounded up to the nearest number divisible by eight.\n" \
	"\n" \
	"    \x1b[4mseed_file\x1b[0m  - Load a world from a seed file, either in raw NetPBM (P4) format, or from a plain\n" \
	"                 text file with the format described below.\n" \
	"\n" \
	"\x1b[1;4mPLAIN TEXT SEED FILE FORMAT\x1b[0m\n" \
	"\n" \
	"    The file should begin with \"txt\" followed by (white space separated) width, height,\n" \
	"    horizontal offset, vertical offset, and then data.\n" \
	"    The offset values are for if you want to e.g. make a big world and place a small seed kernel\n" \
    "    in the middle and not have to draw all the empty space around it. The data is one character per\n" \
    "    cell, '#' denotes a live cell, anything else is a dead cell, except for newlines ('\\n'), which\n" \
	"    increases the y position, and resets the x position to the offset value.\n" \
	"\n" \
	"\x1b[1;4mINTERACTIVE MODE\x1b[0m\n" \
	"\n" \
	"    The Interactive mode starts in a paused state. Pressing the space bar toggles between paused and\n" \
	"    running state. It is also possible to increase or decrease the delay between iterations with the\n" \
	"    up and down arrow keys respectiveley. The program exits when either escape is pressed, or the window\n" \
	"    is closed. Left clicking on a cell with the mouse will bring it alive, and right clicking will kill it.\n"

#endif
