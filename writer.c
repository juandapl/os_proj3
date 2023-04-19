// writer.c: writing to the records
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

void handle_usrsig1();

void readFile(){
    printf("ACCESSING %d\n", state->write_heads[stored_at].segment_number);
    sleep(10);
    printf("DONE!\n");
}

int canQueue(int segment, MemoryState* state){
    for(int i = 0; i < N_ACTIVE_WRITERS; i++){
        if(
            state->write_heads[i].segment_number==segment &&
            (state->write_heads[i].done==0 ||
                state->write_heads[i].active==1)){
            return 0;
        }
    }
    return 1;
}

int main(int argc, char** argv)
{
    signal(SIGUSR1, handle_usrsig1);
    int id;
    int mynum;
    sscanf(argv[1],"%d", &id);
    sscanf(argv[2],"%d", &mynum);
    printf("I AM %d\n", mynum);

    state = shmat(id, NULL, 0);
    

    while(1){
        sem_wait(&(state->cs_mutex)); // segfault here??
        // in critical section
        if(
            state->waiting < N_ACTIVE_WRITERS && 
            canQueue(mynum, state)==1
        ){
            state->waiting++;
            // join the wait queue
           for(int i = 0; i < N_ACTIVE_WRITERS; i++){
               // place myself in writeheads (ie queue)
               if(state->write_heads[i].current_writer==0 || state->write_heads[i].done==1){
                   state->write_heads[i].segment_number = mynum;
                   state->write_heads[i].active = 0;
                   state->write_heads[i].current_writer = getpid();
                   state->write_heads[i].done = 0;
                   stored_at = i;
                   break;
               }
           }
           sem_post(&(state->customers));
           sem_post(&(state->cs_mutex));
           sem_wait(&(state->barber));
           printf("I can execute when called now!\n");
           readFile();
           sem_wait(&(state->cs_mutex));
            state->write_heads[stored_at].done = 1;
            state->write_heads[stored_at].active = 0;
            state->waiting--;
           sem_post(&(state->cs_mutex));
           break;
        }  else {
           sem_post(&(state->cs_mutex));
        }
    }
    return 0;
}

void handle_usrsig1(){
    signal(SIGUSR1, handle_usrsig1);
    readFile();
}
