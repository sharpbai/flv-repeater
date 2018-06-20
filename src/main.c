/*
 * @file main.c
 * @author sharpbai
 * @date 2016/07/20
 */

#include "flv-repeater.h"
#include <getopt.h>
#include <stdlib.h>
#include <string.h>

/*void usage(char *program_name) {
    printf("Usage: %s [start_repeat_index] [repeat_count] [input.flv] "
                   "[output.flv] \n",
           program_name);
    exit(-1);
}
 */
/* Prints usage information for this program to STREAM (typically
   stdout or stderr), and exit the program with EXIT_CODE.  Does not
   return.  */

void print_usage (FILE* stream, int exit_code, char *program_name) {
    fprintf (stream, "Usage:  %s options [ inputfile ... ]\n", program_name);
    fprintf (stream,
             "  -i  --input            Read from this file. Default is stdin.\n"
             "  -s  --start_index      Repeat from this index.\n"
             "  -r  --repeat_count     Repeat file count. 0 is infinite.\n"
             "  --re                    Read input file in framerate.\n"
             "  -h  --help             Display this usage information.\n"
             "  -o  --output           Write output to file. Default is stdout.\n"
             "  -v  --verbose          Print verbose messages.\n");
    exit (exit_code);
}


globalArgs_t globalArgs;
static const char *optString = "i:o:s:r:h:f:?";

static const struct option longOpts[] = {
        {"input",        optional_argument, NULL, 'i'},
        {"output",       optional_argument, NULL, 'o'},
        {"start_index",  required_argument, NULL, 's'},
        {"fix_timestamp",required_argument, NULL, 'f'},
        {"verbose",      no_argument,       NULL, 'v'},
        {"repeat_count", required_argument, NULL, 'r'},
        {"help",         no_argument,       NULL, 'h'},
        {"re",           no_argument,       NULL, 0},
        {NULL,           no_argument,       NULL, 0}
};

int main(int argc, char **argv) {

    // Init globalArgs
    memset(&globalArgs, 0, sizeof(globalArgs_t));
    globalArgs.infile = stdin;
    globalArgs.outfile = stdout;

    int longIndex;
    int opt = getopt_long(argc, argv, optString, longOpts, &longIndex);
    while (opt != -1) {
        switch (opt) {
            case 'i': {
                globalArgs.infile = fopen(optarg, "r");
                break;
            }
            case 'o': {
                globalArgs.outfile = fopen(optarg, "w");
                break;
            }
            case 's': {
                char* end;
                globalArgs.startIndex = strtoul(optarg, &end, 10);
                break;
            }
            case 'f': {
                char* end;
                globalArgs.timestampBase = strtoul(optarg, &end, 10);
                break;
            }
            case 'r': {
                char* end;
                globalArgs.repeatCount = strtoul(optarg, &end, 10);
                break;
            }
            case 'v': {
                globalArgs.verbosity = 1;
                break;
            }
            case 0: {
                if (strcmp("re", longOpts[longIndex].name) == 0) {
                    globalArgs.readInFramerate = 1;
                }
                break;
            }
            default: {
                print_usage(stderr, -1, argv[0]);
            }
        }
        opt = getopt_long(argc, argv, optString, longOpts, &longIndex);
    }

    if (!globalArgs.infile || !globalArgs.outfile) {
        print_usage(stderr, -1, argv[0]);
    }


    flv_repeater_init(&globalArgs);
    flv_repeater_run();

    printf("\nFinished repeating\n");

    return 0;
}
