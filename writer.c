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
#include <sys/times.h> /* times() */

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
    float old_gpa = incoming_record->GPA;

    // modify a random mark:
    struct tms tb1, tb2;
    double t2 = (double) times(&tb2);
    srand((unsigned int) t2);
    int to_modify = rand()%8;
    float new_mark = ((float)rand()/(float)(RAND_MAX)) * 4.0;
    float old_mark = incoming_record->Marks[to_modify];
    incoming_record->Marks[to_modify] = new_mark;

    // log change

    sem_wait(&(state->log_mutex));
    FILE* logFile = fopen("log.txt", "a");
    fprintf(logFile, "%lf WRITER PID: %d, ACCESSING: %d, MODIFIED MARK %d from %.2f to %.2f\n", t2, getpid(), state->write_heads[stored_at].segment_number, to_modify, old_mark, new_mark);
    fclose(logFile);
    sem_post(&(state->log_mutex));

    // recalculate gpa:
    float sum = 0; float avg;
    for(int i = 0; i < NumOfCourses; i++)
    {
        sum += incoming_record->Marks[i];
    }
    avg = sum / NumOfCourses;
    incoming_record->GPA = avg;

    // write change:
    fh = fopen(path, "r+b");
    write_record(fh, state->write_heads[stored_at].segment_number, incoming_record);
    fclose(fh);
   
    printf("Accessed: %ld %s %s Old GPA: %.2f, Modified GPA: %.2f, delay: %ds\n", incoming_record->custid, incoming_record->FirstName, incoming_record->LastName, old_gpa, incoming_record->GPA, time);
    printf("Marks: ");
    for(int i=0; i < NumOfCourses; i++)
    {
        printf("%.2f ", incoming_record->Marks[i]);
    }
    printf("\n");
    free(incoming_record);
    state->total_records_accessed++;
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

    double t1, t2, cpu_time;
    struct tms tb1, tb2;
    double ticspersec;
    int i, sum = 0;
    ticspersec = (double) sysconf(_SC_CLK_TCK);
    t1 = (double) times(&tb1);
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

    state = shmat(id, NULL, 0);

    FILE* logFile;
    t2 = (double) times(&tb2);

    sem_wait(&(state->log_mutex));
    logFile = fopen("log.txt", "a");
    fprintf(logFile, "%lf WRITER STARTED PID: %d, ACCESSING: %d\n", t2, getpid(), record);
    fclose(logFile);
    sem_post(&(state->log_mutex));

    printf("I AM %d (WRITER), I want to access segment %d\n", getpid(), record);

    int ticket_id = -1;
    while(1){
        sem_wait(&(state->cs_mutex));
        if(ticket_id==-1){
            ticket_id = state->next_write_ticket;
            state->next_write_ticket++;
        }
        in_cs = 1;
        // in critical section
        if(
            state->active_writers < N_ACTIVE_WRITERS && 
            canQueue(record, state)==1 &&// if cannot queue, do nothing and check next time
            ticket_id == state->curr_write_ticket
        )
        {
            state->active_writers++;
            state->curr_write_ticket++;
            // join the wait queue
            for(int i = 0; i < N_ACTIVE_WRITERS; i++){
               // place myself in writeheads (ie barber-queue)
               if(state->write_heads[i].current_writer==0 || state->write_heads[i].done==1){
                   state->write_heads[i].segment_number = record;
                   state->write_heads[i].active = 1;
                   state->write_heads[i].current_writer = getpid();
                   state->write_heads[i].done = 0;
                   stored_at = i;
                   break;
               }
           }
           in_cs = 0;
           sem_post(&(state->cs_mutex));

           //sem_wait(&(state->cs_mutex));
           //printf("active is now %d\n", state->write_heads[stored_at].active);
           //sem_post(&(state->cs_mutex));

            t2 = (double) times(&tb2);
            sem_wait(&(state->log_mutex));
            logFile = fopen("log.txt", "a");
            fprintf(logFile, "%lf WRITER WRITING PID: %d, ACCESSING: %d, WITH DELAY: %d\n", t2, getpid(), record, time);
            fclose(logFile);
            sem_post(&(state->log_mutex));

            readFile(time, path);
           
           // say you're done and not active anymore
            sem_wait(&(state->cs_mutex));
            state->write_heads[stored_at].done = 1;
            state->write_heads[stored_at].active = 0;
            state->active_writers--;

            // todo after exiting, flush all readers queuing behind your writer head: (until sem value reaches 0)
            printf("I have %d waiting readers behind me. Setting them free: \n", state->write_heads[stored_at].waiting_readers);
            while (state->write_heads[stored_at].waiting_readers > 0)
            {
                state->write_heads[stored_at].waiting_readers--;
                sem_post(&(state->write_heads[stored_at].proc_queue));   // test
            }

           sem_post(&(state->cs_mutex));

           break;
        }  
        else {// exit the lock without doing nothing
           sem_post(&(state->cs_mutex));
        }
    }    

    // log stats here in console and in file
    t2 = (double) times(&tb2);
    printf("Run time was %lf sec.\n",(t2 - t1) / ticspersec);
    sem_wait(&(state->log_mutex));
    logFile = fopen("log.txt", "a");
    fprintf(logFile, "%lf WRITER COMPLETE PID: %d, ACCESSING: %d\n", t2, getpid(), record);
    fclose(logFile);

    FILE* statFile = fopen("write_stats.bin", "ab");
    double t_total = (t2-t1) / ticspersec;
    fwrite(&t_total, sizeof(double), 1, statFile);
    fclose(statFile);

    sem_post(&(state->log_mutex));
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
