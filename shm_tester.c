// shm_tester.c: a test utility for shared memory content
// by Nicholas Raffone and Juan Piñeros

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <string.h>
#include "shared_structs.h"

int main(int argc, char** argv)
{
    int id;
    sscanf(argv[1],"%d", &id);

    MemoryState* state =  shmat(id, NULL, 0);
    printf("%s", state->test);

    int err = shmdt(state);
}
