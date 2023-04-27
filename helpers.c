// helpers.c: a family of helper functions to deal with binary data files
// by Nicholas Raffone and Juan Piñeros

#include "helpers.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


void read_record(FILE* fh, int segment_number, MyRecord* dest)
{
    FILE* temp = fh;
    //printf("seg: %d\n", segment_number);
    //printf("size: %ld\n", sizeof(MyRecord));
    fseek(temp, segment_number*sizeof(MyRecord), SEEK_SET);   
    fread(dest, sizeof(MyRecord), 1, temp);
}

void write_record(FILE* fh, int segment_number, MyRecord* src)
{
    FILE* temp = fh;
    fseek(temp, segment_number*sizeof(MyRecord), SEEK_SET);   
    fwrite(src, sizeof(MyRecord), 1, temp);
}

int n_records(char* path, int size) // from Prof. Delis
{
    if (access(path, F_OK) == 0) {   // this check from https://stackoverflow.com/questions/230062/whats-the-best-way-to-check-if-a-file-exists-in-c/230068#230068
        // check number of records stats in BIN file
        FILE* fh = fopen(path, "rb");
        fseek (fh, 0 ,SEEK_END);
        long lSize = ftell (fh);
        rewind(fh);
        fclose(fh);
        // check abobe lib calls: fseek, ftell, rewind
   	    return (int) lSize/size;
    } else {

        return 0;
    }
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

double calculate_avg(char* path)
{
    if (access(path, F_OK) == 0) {   // this check from https://stackoverflow.com/questions/230062/whats-the-best-way-to-check-if-a-file-exists-in-c/230068#230068
    // file exists
        double sum = 0;
        FILE* f = fopen(path, "rb");
        int n = n_records(path, sizeof(double));
        for(int i = 0; i < n; i++)
        {
            double record;
            fread(&record, sizeof(double), 1, f);
            sum += record;
        }
        return sum/n;
    } else {

        return 0;
    }
}

double get_max_time(char* path1, char* path2)
{
    
}


