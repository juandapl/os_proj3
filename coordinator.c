// Coordinator.c: a coordinator program for multiple readers and writers to a data file
// by Nicholas Raffone and Juan Piñeros

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <string.h>
#include <semaphore.h>
#include "shared_structs.h"
#include "helpers.h"
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/wait.h>

MemoryState* state;
int id;
int pid;

void initialize_shared_struct(MemoryState* state)
{
    int retval;
    for(int i = 0; i < N_ACTIVE_WRITERS; i++)
    {
        state->write_heads[i].segment_number = 0;
        state->write_heads[i].active = 0;
        state->write_heads[i].done = 0;
        state->write_heads[i].current_writer = 0;
        state->write_heads[i].waiting_readers = 0;
        retval = sem_init(&(state->write_heads[i].proc_queue), 1, 0);

    }
    state->active_writers = 0;
    state->active_readers = 0;
    state->next_write_ticket = 0; // for process to grab
    state->curr_write_ticket = 0; // the ticket that can go in next
    state->next_read_ticket = 0;
    state->curr_read_ticket = 0;
    for(int i = 0; i < N_ACTIVE_READERS; i++)
    {
        state->readers[i].done = 0;
        state->readers[i].segment_number = 0;
        state->readers[i].init = 0;
        state->readers[i].active = 0;
        state->readers[i].current_reader = 0;
    }
    state->total_records_accessed = 0;
    
    // initialize semaphores
    int isVal;
    sem_init(&(state->cs_mutex), 1, 1);
    sem_init(&(state->barber), 1, 0);
    sem_init(&(state->customers), 1, 0);
    sem_init(&(state->waiting_readers), 1, 0);
    sem_init(&(state->reader_barber), 1, 0);
    sem_init(&(state->log_mutex), 1, 1);
}

void destroy_shared_struct(MemoryState* state)
{
    sem_destroy(&(state->cs_mutex));
    sem_destroy(&(state->barber));
    sem_destroy(&(state->customers));
    sem_destroy(&(state->waiting_readers));
    sem_destroy(&(state->reader_barber));
    sem_destroy(&(state->log_mutex));
}

void show_final_stats()
{
    printf("=== FINAL STATS ===\n");
    sem_wait(&(state->log_mutex));

    int writes = n_records("write_stats.bin", sizeof(double));
    int reads =  n_records("read_stats.bin", sizeof(double));
    printf("Number of writes performed = %d\n", writes);
    printf("Average writing time = %.2fs\n", calculate_avg("write_stats.bin"));
    printf("Number of reads performed = %d\n", reads);
    printf("Average reading time = %.2fs\n", calculate_avg("read_stats.bin"));




    sem_post(&(state->log_mutex));
}

void initialize_log_files()
{
    FILE* logFile = fopen("log.txt", "w");
    fclose(logFile);
    FILE* write_stats = fopen("write_stats.bin", "wb");
    fclose(write_stats);
    FILE* read_stats = fopen("read_stats.bin", "wb");
    fclose(read_stats);
}

void handle_exit()
{
    signal(SIGINT, handle_exit);
    if(pid > 0)
    {
        show_final_stats();
    }
    destroy_shared_struct(state);
    if(pid > 0){
        int status;
        kill(pid, SIGKILL);
        waitpid(pid, &status, 0);
    }
    int err = shmctl(id, IPC_RMID, 0);
    exit(0);
}

int main()
{
    // create shared memory segment
    id = shmget(IPC_PRIVATE, sizeof(MemoryState), 0666);
    if(id==-1){
        printf("error opening shared memory\n");
        return 1;
    }
    printf("Shared memory ID: %d\n", (int) id);

    // attach shared mem and initialize the shared struct
    state = shmat(id, NULL, 0);
    initialize_shared_struct(state);
    initialize_log_files();
    strcpy(state->test, "test");

    // // pause for test
    // printf("Enter when you're done: ");
    // char inputbuff[256];
    // fgets(inputbuff, 256, stdin);
    pid = fork();
    if(pid==0){
        //printf("IN CHILD: readers\n");
        while(1){
            printf("barber sleep: readers\n");
            sem_wait(&(state->waiting_readers));
            printf("barber woke (new reader!)\n");
            sem_post(&(state->reader_barber));
        }
    }
    else{
        signal(SIGINT, handle_exit);
        //printf("in parent: readers\n");
        while(1){
            printf("barber sleep: writers\n");
            sem_wait(&(state->customers));
            printf("barber woke (new writer!)\n");
            sem_post(&(state->barber));
            // give-a-haircut(); -> allow writer to write to x if another writer isn't already writing to x
        }
    }
}
