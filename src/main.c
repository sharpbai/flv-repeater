/*
 * @file main.c
 * @author sharpbai
 * @date 2016/07/20
 */

#include "flv-repeater.h"
#include <stdio.h>
#include <stdlib.h>

void usage(char *program_name) {
  printf("Usage: %s [start_repeat_index] [repeat_count] [input.flv] "
         "[output.flv] \n",
         program_name);
  exit(-1);
}

int main(int argc, char **argv) {

  FILE *infile = NULL;
  FILE *outfile = NULL;

  if (argc == 3) {
    infile = stdin;
    outfile = stdout;
  } else if (argc == 5) {
    infile = fopen(argv[3], "r");
    outfile = fopen(argv[4], "w");
    if (!infile || !outfile) {
      usage(argv[0]);
    }
  } else {
    usage(argv[0]);
  }

  // 从某个FLV Tag的字节偏移开始
  size_t start_index = atoi(argv[1]);
  // 重复次数
  size_t repeat_count = atoi(argv[2]);

  flv_repeater_init(infile, outfile, start_index, repeat_count);
  flv_repeater_run();

  printf("\nFinished repeating\n");

  return 0;
}
