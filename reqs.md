### Approach
- Limited number of semaphores (writer-heads) available
- Each writer-head is associated with a writer, which is a process actively modifying a segment of the data file
- We queue the writers on the use of the writer-heads (queue)
- The coordinator is responsible for managing the writer-heads and the queue of writers
- The coordinator is also responsible for managing the shared memory segment

Readers can access things without any restrictions, except if the segment they want to use is being used in one of the writer heads.
In that case, they queue behind the writer-heads through a semaphore.
When the writer is done, the semaphore queue is flushed and the readers can access the segment.

Access to the shared memory segment is a critical section problem.

- Coordinator
    - Establishes the shared memory segment
        - writeheads[10]: 
           - semaphore initialised to 0 (queues read processes)
           - segment number
           - pid of writer
           - done flag
        - readers[]:
           - pid of reader
           - segment currently used
        - int active: number of active writers
    Critical Section Semaphores:
        - cs_mutex (for entire shared memory segment)
    Sleeping Barber:
        implement what delis said but:
        - barber function is to identify when writers are done, and flush the semaphore

- Reader
    - If no write head is active with the desired segment, read
    - If a write head is active (writing or not) with the desired segment, queue behind the write head's semaphore

- Writer
    - Queue Problem (sleeping barber)
    - When in the queue, the writer is waiting for a writer-head to be available
    - When active:
        - Wait for all readers for the segment to end, then write
        - All readers for said segment will be blocked by the write head's semaphore
    - When done:
        - Flush the semaphore

- Monitor:
    Tries to gain lock and prints stuff

1. Create a coordinator that creates a shm and try to access it and verify it works correctly
2. Writers first (and manage the queue)
3. Readers on top
4. Fluff