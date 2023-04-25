// helpers.c: a family of helper functions to deal with binary data files
// by Nicholas Raffone and Juan Piñeros

#include "helpers.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


void read_record(FILE* fh, int segment_number, MyRecord* dest)
{
    FILE* temp = fh;
    fseek(temp, segment_number*sizeof(MyRecord), SEEK_SET);     
    fread(dest, sizeof(MyRecord), 1, temp);
}

int* separate_commas(char* thing, int* size) // this is annoying
{
    int n_things = 1;
    for(int i = 0; i < strlen(thing); i++)
    {
        n_things += (thing[i] == ',');
    }
    int* dest = (int*) malloc(n_things*sizeof(int));


    char* token = strtok(thing, ",");
    dest[0] = atoi(token);
    

    for(int i = 1; i < n_things; i++) 
    {
    	token = strtok(NULL, ",");
        dest[i] = atoi(token);
    }

    *size = n_things;
    return dest;
}


