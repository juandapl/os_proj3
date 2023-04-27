// monitor.c: displays state of affairs of the thing
// ./monitor -s shared mem id -> gives you a snapshot of the state of affairs. 
// by Nicholas Raffone and Juan Piñeros

#include "shared_structs.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <string.h>
#include "helpers.h"
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <semaphore.h>

MemoryState* state;
int in_cs;
int in_log;

void handle_interrupt()
{
    signal(SIGINT, handle_interrupt);
    printf("See you.\n");
    if(in_cs)
    {
        sem_post(&(state->cs_mutex));
    }
    if(in_log)
    {
        sem_post(&(state->log_mutex));
    }
    exit(0);
}

int main(int argc, char** argv)
{
    signal(SIGINT, handle_interrupt);
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
    
    while(1){
        sleep(1);
        system("clear");

        sem_wait(&(state->cs_mutex));
        in_cs = 1;
        printf("=== ACTIVE WRITERS: %d ===\n", state->active_writers);

        printf("Writer PID, Segment Number, Active, Done, Readers Queued\n");
        for(int i = 0; i < N_ACTIVE_WRITERS; i++)
        {
            if(state->write_heads[i].current_writer != 0 && state->write_heads[i].active != 0)
            {
                printf("%d, %d, %d, %d, %d\n", state->write_heads[i].current_writer,state->write_heads[i].segment_number,state->write_heads[i].active,state->write_heads[i].done, state->write_heads[i].waiting_readers);
            }
        }
        printf("\n");

        printf("=== ACTIVE READERS: %d ===\n", state->active_readers);

        printf("Reader PID, Segment Number, Active\n");
        for(int i = 0; i < N_ACTIVE_READERS; i++)
        {
            if(state->readers[i].current_reader != 0 && state->readers[i].active != 0)
            {
            printf("%d, %d, %d\n", state->readers[i].current_reader, state->readers[i].segment_number, state->readers[i].active);
            }
        }
        printf("\n");

        printf("=== SEMAPHORE VALUES ===\n");
        int customers_value;
        int waiting_readers_value;

        sem_getvalue(&(state->customers), &customers_value); 
        sem_getvalue(&(state->waiting_readers), &waiting_readers_value); 

        printf("customers = %d\n", customers_value);
        printf("waiting_readers = %d\n", waiting_readers_value);

        in_cs = 0;
        sem_post(&(state->cs_mutex));

        printf("\n");

        printf("=== GENERAL STATS ===\n");
        sem_wait(&(state->log_mutex));
        in_log = 1;
        printf("Number of writes performed = %d\n", n_records("write_stats.bin", sizeof(double)));
        printf("Average writing time = %.2fs\n", calculate_avg("write_stats.bin"));
        printf("Number of reads performed = %d\n", n_records("read_stats.bin", sizeof(double)));
        printf("Average reading time = %.2fs\n", calculate_avg("read_stats.bin"));
        printf("Total records accessed = %d\n", state->total_records_accessed);
   

        in_log = 0;
        sem_post(&(state->log_mutex));

    }
    
    return 0;
}
