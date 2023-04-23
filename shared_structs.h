#include <sys/types.h>
#include <sys/ipc.h>
#include <semaphore.h>

#define N_ACTIVE_WRITERS 2
#define N_ACTIVE_READERS 100

typedef struct WriteHead
{
    sem_t proc_queue;
    int segment_number;
    pid_t current_writer;
    int active;
    int done;
} WriteHead;

typedef struct Reader
{
    int segment_number;
    pid_t current_reader;
    int done;
    int active;
    int init;
} Reader;

typedef struct MemoryState
{
    WriteHead write_heads[N_ACTIVE_WRITERS];
    int active_writers;
    int active_readers;
    Reader readers[N_ACTIVE_READERS];
    char test[6];
    sem_t cs_mutex; // protects memory access
    sem_t writers_mutex; // protects active_writers
    sem_t barber;
    sem_t customers;
    sem_t waiting_readers;
    sem_t reader_barber;
    int waiting;
} MemoryState;


// todo change all to unnamed semaphores