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
#include "helpers.h"

int stored_at;
MemoryState* state;
int in_cs = 0;

void handle_usrsig1();
void handle_segfault();

void readFile(int time, char* path){
    printf("ACCESSING %d\n", state->write_heads[stored_at].segment_number);
    sleep(time);
    MyRecord* incoming_record = (MyRecord*) malloc(sizeof(MyRecord));
    FILE* fh = fopen (path, "rb");

    read_record(fh, state->write_heads[stored_at].segment_number, incoming_record);
    fclose(fh);
    printf("Accessed: %ld %s %s GPA: %.2f\n", incoming_record->custid, incoming_record->FirstName, incoming_record->LastName, incoming_record->GPA);
    printf("DONE!\n");
}

//check whether there is space in the writers/readers to access the desired segment
int canQueue(int segment, MemoryState* state){
    for(int i = 0; i < N_ACTIVE_WRITERS; i++){
        if(
            state->write_heads[i].segment_number==segment &&
            (state->write_heads[i].done==0 ||
                state->write_heads[i].active==1)){
            return 0;
        }
    }
    for(int i = 0; i < N_ACTIVE_READERS; i++){
        if(
            state->readers[stored_at].init ==1 &&
            state->readers[i].segment_number == segment
            && (state->readers[i].active == 1 || state->readers[i].done==0)){
            return 0;
        }
    }
    return 1;
}

//maybe add a queue semaphore for the queue bc bounded waiting TODO???

int main(int argc, char** argv)
{
    int id;
    char path[256];
    int record;
    int time;
    char c;

    signal(SIGINT, handle_segfault);

    // ./writer -f filename -l record-id -d pause time -s shared mem id
    if(argc != 9)
    {
        printf("Too many or too little arguments.\n");
        printf("Usage: ./writer -f filename -l record-id -d pause time -s shared mem id");
        exit(1);
    }


    while ((c = getopt (argc, argv, "f:l:d:s:")) != -1)
        switch (c)
        {
            case 'f':
                strcpy(path, optarg);
                break;
            case 'l':
                record = atoi(optarg);
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

    printf("I AM %d (WRITER), I want to access segment %d\n", getpid(), record);

    state = shmat(id, NULL, 0);

    while(1){
        sem_wait(&(state->cs_mutex));
        in_cs = 1;
        // in critical section
        if(
            state->active_writers < N_ACTIVE_WRITERS && 
            canQueue(record, state)==1 // if cannot queue, do nothing and check next time
        )
        {
            state->active_writers++;
            // join the wait queue
            for(int i = 0; i < N_ACTIVE_WRITERS; i++){
               // place myself in writeheads (ie barber-queue)
               if(state->write_heads[i].current_writer==0 || state->write_heads[i].done==1){
                   state->write_heads[i].segment_number = record;
                   state->write_heads[i].active = 0;
                   state->write_heads[i].current_writer = getpid();
                   state->write_heads[i].done = 0;
                   stored_at = i;
                   break;
               }
           }
           sem_post(&(state->customers));
           in_cs = 0;
           sem_post(&(state->cs_mutex));
           sem_wait(&(state->barber)); 

           //sem_wait(&(state->cs_mutex));
           //printf("active is now %d\n", state->write_heads[stored_at].active);
           //sem_post(&(state->cs_mutex));

           printf("I have been chosen\n");
           readFile(time, path);
           
           // say you're done and not active anymore
           sem_wait(&(state->cs_mutex));
            state->write_heads[stored_at].done = 1;
            state->write_heads[stored_at].active = 0;
            state->active_writers--;

            // todo after exiting, flush all readers queuing behind your writer head: (until sem value reaches 0)
            int writer_head_queue_value;
            sem_getvalue(&(state->write_heads[stored_at].proc_queue), &writer_head_queue_value);
            while (writer_head_queue_value != 0)
            {
                sem_post(&(state->write_heads[stored_at].proc_queue))
            }

           sem_post(&(state->cs_mutex));

           break;
        }  
        else {// exit the lock without doing nothing
           sem_post(&(state->cs_mutex));
        }
    }    
    return 0;
    
}

// void handle_usrsig1(){
//     signal(SIGUSR1, handle_usrsig1);
//     readFile(time);
// }

void handle_segfault()
{
    signal(SIGINT, handle_segfault);
    printf("Juan is dumb, you had to interrupt.\n");
    if(in_cs)
    {
        sem_post(&(state->cs_mutex));
    }
    exit(0);
}
