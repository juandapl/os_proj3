// helpers.h: a family of helper functions to deal with binary data files
// by Nicholas Raffone and Juan Piñeros

#ifndef HELP
#include <stdio.h>
#include "shared_structs.h"
#define HELP

void read_record(FILE* fh, int segment_number, MyRecord* dest);
void write_record(FILE* fh, int segment_number, MyRecord* src);

int* separate_commas(char* thing, int* size);
#endif