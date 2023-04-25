// reader.c: reading from the records (predictably)
// by Nicholas Raffone and Juan Piñeros

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <string.h>
#include "shared_structs.h"
#include "helpers.h"
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <semaphore.h>


int stored_at;
MemoryState* state;
int canRead(int segment, MemoryState* state, int* blocking_writer){
    for(int i = 0; i < N_ACTIVE_WRITERS; i++){
        if(
            state->write_heads[i].segment_number==segment
           ){
        	if(state->write_heads[i].active==1){
                *blocking_writer = i; // which writer is blocking me?
            	return 0;
        	}
        }
    }
    return 1;
}

void readFile(int segnum){
	printf("reading: %d\n", segnum);
}

int main(int argc, char** argv)
{
    int id;
    char segment_numbers[4096]; // Do you want a buffer overflow? You'll have to work for it.
    int time;
    char path[256];
    char c;
    int* segments;
    int n_segments;

    // ./reader -f filename -l record-id,[,record-id,record-id...] -d pause time -s shared mem id
    if(argc != 9)
    {
        printf("Too many or too little arguments.\n");
        printf("Usage: ./reader -f filename -l record-id,[,record-id,record-id...] -d pause time -s shared mem id\n");
        exit(1);
    }

    while ((c = getopt (argc, argv, "f:l:d:s:")) != -1)
        switch (c)
        {
            case 'f':
                strcpy(path, optarg);
                break;
            case 'l':
                strcpy(segment_numbers, optarg);
                break;
            case 'd':
                time = atoi(optarg);
                break;
            case 's':
                id = atoi(optarg);
                break;
            default:
                printf("One or more flags not recognised. Exiting...\n");
                exit(0);
        }

    printf("I AM %d (READER) \n", getpid());

    segments = separate_commas(segment_numbers, &n_segments);

    printf("I want to read segments: ");

    for(int i = 0; i < n_segments; i++)
    {
        printf("%d ", segments[i]);
    }
    printf("\n");

    state = shmat(id, NULL, 0);
    int blocking_writer;

    while(1){
        // do this for every segment id asked for!
        for(int n = 0; n < n_segments; n++)
        {
            mynum = segments[n];

            sem_wait(&(state->cs_mutex));
            // Check if we can read. If we can read, read! (duh).
            // If we cannot read, line up behind the writer currently accessing our record.
            if(canRead(mynum, state, &blocking_writer)==1 &&
                state->active_readers < N_ACTIVE_READERS)
                {
                state->active_readers++;
                for(int i = 0; i < N_ACTIVE_READERS; i++){
                if(state->readers[i].done==1 || state->readers[i].init==0){
                    state->readers[i].active = 0;
                    state->readers[i].segment_number = mynum;
                    state->readers[i].current_reader = getpid();
                    state->readers[i].done = 0;
                    state->readers[i].init = 1;
                    stored_at = i;
                    break;
                }
                }
                sem_post(&(state->waiting_readers));
                sem_post(&(state->cs_mutex));
                sem_wait(&(state->reader_barber));

                // i'm allowed to read! better tell everyone --
                sem_wait(&(state->cs_mutex));
                state->readers[stored_at].active = 1;
                sem_post(&(state->cs_mutex));

                readFile(mynum);

                // i'm out!
                sem_wait(&(state->cs_mutex));
                state->readers[stored_at].done = 1;
                state->readers[stored_at].active = 0;
                state->active_readers--;
                sem_post(&(state->cs_mutex));


                break;
                printf("I AM READING %d\n", mynum);
            }else{
                sem_post(&(state->cs_mutex));
                // line up behind the writer head that is currently using my segment:
                sem_wait(&(state->write_heads[blocking_writer].proc_queue));
            }
        }
    }
    readFile(mynum);
}