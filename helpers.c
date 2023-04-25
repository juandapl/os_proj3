// helpers.c: a family of helper functions to deal with binary data files
// by Nicholas Raffone and Juan Piñeros

#include "helpers.h"


void read_record(FILE* fh, int segment_number, MyRecord* dest)
{
    FILE* temp = fh;
    fseek(temp, segment_number*sizeof(MyRecord), SEEK_SET);     
    fread(dest, sizeof(MyRecord), 1, temp);
}

