// reader.c: reading from the records (predictably)
// by Nicholas Raffone and Juan Piñeros

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <string.h>
#include "shared_structs.h"
#include "helpers.h"
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/times.h> /* times() */

int stored_at;
MemoryState* state;
int canRead(int segment, MemoryState* state, int* blocking_writer){
    for(int i = 0; i < N_ACTIVE_WRITERS; i++){
        if(
            state->write_heads[i].segment_number==segment
           )
        {
        	if(state->write_heads[i].active==1){
                *blocking_writer = i; // which writer is blocking me?
                printf("Blocked by WriteHead Active %d\n", *blocking_writer);
                state->write_heads[i].waiting_readers ++;
            	return 0;
        	}
        }
    }
    return 1;
}

void readFile(int time, char* path, int segment_number){
    printf("ACCESSING %d\n", segment_number);
    sleep(time);
    MyRecord* incoming_record = (MyRecord*) malloc(sizeof(MyRecord));
    FILE* fh = fopen (path, "rb");
    read_record(fh, segment_number, incoming_record);
    fclose(fh);
    printf("Accessed: %ld %s %s GPA: %.2f, delay: %ds\n", incoming_record->custid, incoming_record->FirstName, incoming_record->LastName, incoming_record->GPA, time);
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

int main(int argc, char** argv)
{
    int id;
    char segment_numbers[4096]; // Do you want a buffer overflow? You'll have to work for it.
    int time;
    char path[256];
    char c;
    int* segments;
    int n_segments;
    double t1, t2, cpu_time;
    struct tms tb1, tb2;
    double ticspersec;
    int i, sum = 0;
    ticspersec = (double) sysconf(_SC_CLK_TCK);
    t1 = (double) times(&tb1);

    // ./reader -f filename -l record-id,[,record-id,record-id...] -d pause time -s shared mem id
    if(argc != 9)
    {
        printf("Too many or too little arguments.\n");
        printf("Usage: ./reader -f filename -l record-id,[,record-id,record-id...] -d pause time -s shared mem id\n");
        exit(1);
    }

    while ((c = getopt (argc, argv, "f:l:d:s:")) != -1)
        switch (c)
        {
            case 'f':
                strcpy(path, optarg);
                break;
            case 'l':
                strcpy(segment_numbers, optarg);
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

    printf("I AM %d (READER) \n", getpid());

    segments = separate_commas(segment_numbers, &n_segments);
    FILE* logFile;
    t2 = (double) times(&tb2);

    sem_wait(&(state->log_mutex));
    logFile = fopen("log.txt", "a");

    fprintf(logFile, "%lf READER STARTED PID: %d, ACCESSING: ", t2, getpid());

    for(int i = 0; i < n_segments; i++)
    {
        fprintf(logFile, "%d ", segments[i]);
    }
    fprintf(logFile, "\n");

    fclose(logFile);

    sem_post(&(state->log_mutex));

    printf("I want to read segments: ");

    for(int i = 0; i < n_segments; i++)
    {
        printf("%d ", segments[i]);
    }
    printf("\n");

    int ticket_id;
    for(int n = 0; n < n_segments; n++){
        // do this for every segment i asked for!
        int mynum = segments[n];
        int blocking_writer = -1;
        ticket_id = -1;
        while(1){
            sem_wait(&(state->cs_mutex));
            if(ticket_id==-1){
                ticket_id = state->next_read_ticket;
                state->next_read_ticket++;
            }
            // Check if we can read. If we can read, read! (duh).
            // If we cannot read, line up behind the writer currently accessing our record.
            if(
                canRead(mynum, state, &blocking_writer)==1 &&
                state->active_readers < N_ACTIVE_READERS &&
                ticket_id == state->curr_read_ticket
            ){
                state->active_readers++;
                state->curr_read_ticket++;
                for(int i = 0; i < N_ACTIVE_READERS; i++){
                    if(state->readers[i].done==1 || state->readers[i].init==0){
                        state->readers[i].active = 1;
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

                // i'm allowed to read! better tell everyone --
                sem_wait(&(state->cs_mutex));
                state->readers[stored_at].active = 1;
                sem_post(&(state->cs_mutex));

                t2 = (double) times(&tb2);
                sem_wait(&(state->log_mutex));
                logFile = fopen("log.txt", "a");
                fprintf(logFile, "%lf READER READING PID: %d, ACCESSING: %d, WITH DELAY: %d\n", t2, getpid(), mynum, time);
                fclose(logFile);
                sem_post(&(state->log_mutex));

                readFile(time, path, mynum);

                // i'm out!
                sem_wait(&(state->cs_mutex));
                state->readers[stored_at].done = 1;
                state->readers[stored_at].active = 0;
                state->active_readers--;
                sem_post(&(state->cs_mutex));


                break;
                printf("I AM READING %d\n", mynum);
            }
            else{
                sem_post(&(state->cs_mutex));
                if(blocking_writer != -1){
                    // if blocking writer, line up behind the writer head that is currently using my segment:
                    printf("%d is blocking me\n",  state->write_heads[blocking_writer].current_writer);
                    sem_wait(&(state->write_heads[blocking_writer].proc_queue)); 
                }
                // else, either full queue or not my turn (come back later)
            }
        }
    }
    t2 = (double) times(&tb2);
    printf("Run time was %lf sec.\n",(t2 - t1) / ticspersec);
    sem_wait(&(state->log_mutex));
    logFile = fopen("log.txt", "a");
    fprintf(logFile, "%lf READER COMPLETE PID: %d, ACCESSING: ", t2, getpid());
    for(int i = 0; i < n_segments; i++)
    {
        fprintf(logFile, "%d ", segments[i]);
    }
    fprintf(logFile, "\n");

    FILE* statFile = fopen("read_stats.bin", "ab");
    double t_total = (t2-t1) / ticspersec;
    fwrite(&t_total, sizeof(double), 1, statFile);
    fclose(statFile);

    fclose(logFile);


    sem_post(&(state->log_mutex));
    // todo after done, delete urself from active readers
    free(segments);
    return 0;
}
