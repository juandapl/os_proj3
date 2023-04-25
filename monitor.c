// monitor.c: displays state of affairs of the thing
// ./monitor -s shared mem id -> gives you a snapshot of the state of affairs. 
// by Nicholas Raffone and Juan Piñeros

#include "shared_structs.h"
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

MemoryState* state;

int main(int argc, char** argv)
{
    int id;
    char c;
    // ./monitor -s shared mem id
    if(argc != 3)
    {
        printf("Too many or too little arguments.\n");
        printf("Usage: ./monitor -s shared mem id");
        exit(1);
    }

    while ((c = getopt (argc, argv, "s:")) != -1)
        switch (c)
        {
            case 's':
                id = atoi(optarg);
                break;
            default:
                printf("One or more flags not recognised. Exiting...\n");
                exit(0);
        }

    
    
    state = shmat(id, NULL, 0);
    
    system("clear");

    int semvalue;

    sem_wait(&(state->cs_mutex));
    printf("=== ACTIVE WRITERS: %d ===\n", state->active_writers);

    printf("Writer PID, Segment Number, Active, Done, Readers Queued\n");
    for(int i = 0; i < N_ACTIVE_WRITERS; i++)
    {
        if(state->write_heads[i].current_writer != 0)
        {
            printf("%d, %d, %d, %d, %d\n", state->write_heads[i].current_writer,state->write_heads[i].segment_number,state->write_heads[i].active,state->write_heads[i].done, state->write_heads[i].waiting_readers);
        }
    }
    printf("\n");

    printf("=== ACTIVE READERS: %d ===\n", state->active_readers);

    printf("Reader PID, Segment Number\n");
    for(int i = 0; i < N_ACTIVE_READERS; i++)
    {
        if(state->readers[i].current_reader != 0)
        {
        printf("%d, %d\n", state->readers[i].current_reader, state->readers[i].segment_number);
        }
    }
    printf("\n");

    printf("=== SEMAPHORE VALUES ===\n");
    int barber_value;
    int customers_value;
    int waiting_readers_value;
    int reader_barber_value;

    sem_getvalue(&(state->barber), &barber_value);
    sem_getvalue(&(state->customers), &customers_value); 
    sem_getvalue(&(state->waiting_readers), &waiting_readers_value); 
    sem_getvalue(&(state->reader_barber), &reader_barber_value); 

    printf("barber = %d\n", barber_value);
    printf("customers = %d\n", customers_value);
    printf("waiting_readers = %d\n", waiting_readers_value);
    printf("reader_barber = %d\n", reader_barber_value);


    sem_post(&(state->cs_mutex));



    return 0;
}