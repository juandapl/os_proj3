// Coordinator.c: a coordinator program for multiple readers and writers to a data file
// by Nicholas Raffone and Juan Piñeros

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <string.h>
#include <semaphore.h>
#include "shared_structs.h"
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

void initialize_shared_struct(MemoryState* state)
{
    int retval;
    for(int i = 0; i < N_ACTIVE_WRITERS; i++)
    {
        state->write_heads[i].segment_number = 0;
        state->write_heads[i].active = 0;
        state->write_heads[i].done = 0;
        state->write_heads[i].current_writer = 0;
        retval = sem_init(&(state->write_heads[i].proc_queue), 1, 0);

    }
    state->active_writers = 0;
    state->active_readers = 0;
    state->waiting = 0;
    for(int i = 0; i < N_ACTIVE_READERS; i++)
    {
        state->readers[i].done = 0;
        state->readers[i].segment_number = 0;
    }
    
    // initialize semaphores TODO
    int isVal;
    state->customers = (sem_t*) malloc(sizeof(sem_t));
    state->barber = (sem_t*) malloc(sizeof(sem_t));
    state->cs_mutex = (sem_t*) malloc(sizeof(sem_t));
    sem_unlink("/customers");
    sem_unlink("/barber");
    sem_unlink("/cs_mutex");
    state->customers = sem_open("/customers", O_CREAT, S_IRUSR, 0);
    state->barber = sem_open("/barber", O_CREAT, S_IRUSR, 0);
    state->cs_mutex = sem_open("/cs_mutex", O_CREAT, S_IRUSR, 1);
}

void destroy_shared_struct(MemoryState* state)
{

}

int main()
{
    // create shared memory segment
    int id = shmget(IPC_PRIVATE, sizeof(MemoryState), 0666);
    if(id==-1){
        printf("error opening shared memory\n");
        return 1;
    }
    printf("%d\n", (int) id);

    // attach shared mem and initialize the shared struct
    MemoryState* state = shmat(id, NULL, 0);
    initialize_shared_struct(state);
    // strcpy(state->test, "test");

    // // pause for test
    // printf("Enter when you're done: ");
    // char inputbuff[256];
    // fgets(inputbuff, 256, stdin);

    while(1){
        printf("barber sleep\n");
        sem_wait(state->customers);
        printf("barber woke\n");
        sem_wait(state->cs_mutex);
        state->waiting--;
        sem_post(state->barber);
        sem_post(state->cs_mutex);
        for(int i = 0; i < N_ACTIVE_WRITERS; i++){
            if(state->write_heads[i].current_writer!=0&&state->write_heads[i].active==0){
                state->write_heads[i].active=1;
                kill(state->write_heads[i].current_writer, SIGUSR1);
                break;
            }
        }
        // give-a-haircut(); -> allow writer to write to x if another writer isn't already writing to x
    }
    int err = shmctl(id, IPC_RMID, 0);
}
