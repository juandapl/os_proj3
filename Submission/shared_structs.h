#ifndef STRUCTS
#define STRUCTS

#include <sys/types.h>
#include <sys/ipc.h>
#include <semaphore.h>

#define N_ACTIVE_WRITERS 10
#define N_ACTIVE_READERS 100

typedef struct WriteHead
{
    sem_t proc_queue;
    int segment_number;
    pid_t current_writer;
    int active;
    int done;
    int waiting_readers;
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
    sem_t log_mutex;
    int waiting;
    int next_write_ticket;
    int curr_write_ticket;
    int next_read_ticket;
    int curr_read_ticket;
    int total_records_accessed;
} MemoryState;

// from Prof. Delis

#define SIZEofBUFF 20
#define NumOfCourses 8

typedef struct{
	long  	custid; 
	char 	FirstName[SIZEofBUFF];
	char 	LastName[SIZEofBUFF];
	float   Marks[NumOfCourses];
	float   GPA;
} MyRecord;



#endif
