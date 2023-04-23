// writer.c: reading from the records
// by Nicholas Raffone and Juan Piñeros

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <string.h>
#include "shared_structs.h"
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <semaphore.h>

int stored_at;
MemoryState* state;
int canRead(int segment, MemoryState* state){
    for(int i = 0; i < N_ACTIVE_WRITERS; i++){
        if(
            state->write_heads[i].segment_number==segment
           ){
        	if(state->write_heads[i].active==1){
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
    int mynum;
    sscanf(argv[1],"%d", &id);
    sscanf(argv[2],"%d", &mynum);
    printf("I AM %d (READER) \n", mynum);

    state = shmat(id, NULL, 0);
    
    while(1){
    	sem_wait(&(state->cs_mutex));
    	if(canRead(mynum, state)==1 &&
    		state->active_readers < N_ACTIVE_READERS){
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
            sem_wait(&(state->cs_mutex));
            state->write_heads[stored_at].active = 1;
    		sem_post(&(state->cs_mutex));
    		readFile(mynum);
    		sem_wait(&(state->cs_mutex));
            state->readers[stored_at].done = 1;
            state->readers[stored_at].active = 0;
            state->active_readers--;
            sem_post(&(state->cs_mutex));
    		break;
    		printf("I AM READING %d\n", mynum);
    	}else{
    		sem_post(&(state->cs_mutex));
    	}
    }
    readFile(mynum);
}