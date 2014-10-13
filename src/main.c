#include "ca.h"
#include "io.h"
#include "gui.h"
#include "help.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>


void progrep(int p) {
	fprintf(stderr, "%x", p>>4);
}
int main(int argc, const char **argv) { /*{{{*/
	bool error = false;
	int ret = 0;
	bool emptyworld = true;
	const char *seedfn = NULL;
	const char *rules = NULL;
	int turns = 0;
	const char *pbmfile = NULL;
	const char *pbmnfmt = NULL;
	int width = 0;
	int height = 0;
	// read arguments
	for (int i=1; i<argc; ++i) { /*{{{*/
		if (strcmp("-r", argv[i]) == 0) {
			if (i == argc-1) { /*{{{*/
				error = true;
				break;
			} /*}}}*/
			rules = argv[++i];
		}
		else if (strcmp("-n", argv[i]) == 0) {
			if (i == argc-1) { /*{{{*/
				error = true;
				break;
			} /*}}}*/
			turns = atoi(argv[++i]);
		}
		else if (strcmp("-o", argv[i]) == 0) {
			if (i == argc-1) { /*{{{*/
				error = true;
				break;
			} /*}}}*/
			pbmfile = argv[++i];
		}
		else if (strcmp("-O", argv[i]) == 0) {
			if (i == argc-1) { /*{{{*/
				error = true;
				break;
			} /*}}}*/
			pbmnfmt = argv[++i];
		}
		else if (strcmp("-w", argv[i]) == 0) {
			if (i == argc-2) { /*{{{*/
				error = true;
				break;
			} /*}}}*/
			width = atoi(argv[++i]);
			height = atoi(argv[++i]);
			emptyworld = true;
		}
		else {
			seedfn = argv[i];
			emptyworld = false;
		}
	} /*}}}*/
	// check that a seed file is given
	if (error || argc == 0 || (seedfn == NULL && !emptyworld)) { /*{{{*/
		fprintf(stderr, "\n\n" HELPSECTION "\n\n");
		fprintf(stderr, "Exiting due to malformed or missing required commands.\n\n");
		ret = 1;
		goto cleanup_1;
	} /*}}}*/
	ruleset_t *rs = NULL;
	// revert to conway's ruleset if none given
	if (rules == NULL) { /*{{{*/
		fprintf(stderr, "assuming conway ruleset of 23/3\n");
		rs = build_ruleset("23/3");
	}
	else {
		fprintf(stderr, "parsing ruleset\n");
		rs = build_ruleset(rules);
	} /*}}}*/
	// check that ruleset is properly parsed
	if (rs == NULL) { /*{{{*/
		fprintf(
			stderr,
			"error parsing ruleset, memory allocation failure or malformed ruleset string \"%s\"\n",
			argv[1]
		);
		ret = 2;
		goto cleanup_2;
	} /*}}}*/
	fprintf(stderr, "done\n");
	pp_ruleset(rs);
	// create and populate world
	world_t *wo = NULL;
	if (emptyworld) {
		wo = create_world((width+7)/8, height);
	}
	else {
		fprintf(stderr, "loading seed file\n");
		wo = load_seed(seedfn);
	}
	// see that it is good
	if (wo == NULL) { /*{{{*/
		fprintf(stderr, "could not create world\n");
		if (!emptyworld) {
			fprintf(stderr, "could not populate world with seed from \"%s\"\n", seedfn);
		}
		ret = 3;
		goto cleanup_3;
	} /*}}}*/
	// build lut
	fprintf(stderr, "building lookup table\n");
	ruleset_lut_t *rsl = build_ruleset_lut(rs, progrep);
	// check lut
	if (rsl == NULL) { /*{{{*/
		fprintf(stderr, "could not create ruleset lookup table, bad ruleset input or memory allocation error\n");
		ret = 4;
		goto cleanup_4;
	} /*}}}*/
	fprintf(stderr, "\ndone\n");
	// main loop
	if (turns > 0) { /*{{{*/
		int g = 0;
		if (pbmnfmt == NULL) { /*{{{*/
			clock_t start = clock();
			for(int i=0; i<turns; ++i) {
				update_world(wo, rsl, g);
				g ^= 1;
			}
			clock_t stop = clock();
			printf("%dx%d, %d turns, time taken: %.3fs\n", wo->w*8, wo->h, turns, (((double)stop) - ((double)start))*1e-6);
		} /*}}}*/
		else { /*{{{*/
			char *fn = calloc(strlen(pbmnfmt)+128, sizeof(char));
			sprintf(fn, pbmnfmt, 0);
			write_pbm_world(fn, wo, 0);
			clock_t start = clock();
			for(int i=0; i<turns; ++i) {
				update_world(wo, rsl, g);
				g ^= 1;
				sprintf(fn, pbmnfmt, i);
				write_pbm_world(fn, wo, g);
			}
			clock_t stop = clock();
			free(fn);
		} /*}}}*/
		if (pbmfile != NULL) {
			write_pbm_world(pbmfile, wo, g);
		}
	} /*}}}*/
	else { /*{{{*/
#ifndef NOGUI
		ret = gui_main(wo, rsl);
		if (ret != 0) {
			ret += 4;
		}
#else
		fprintf(stderr, "this version is not compiled with gui support.\n");
#endif
	} /*}}}*/
cleanup_4:
	free_ruleset_lut(rsl);
cleanup_3:
	free_world(wo);
cleanup_2:
	free_ruleset(rs);
	fprintf(stderr, "cleanup done\n");
cleanup_1:
	return ret;
} /*}}}*/
