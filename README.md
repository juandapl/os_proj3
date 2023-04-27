# Readers and Writers (CS-UH 3010 Project 3)
### Created by Juan Piñeros and Nicholas Raffone

#### The coordinator, reader, writer, and monitor programs enables the user to coordinate, reader, write, and monitor the status of the students grades.

## Usage

### Compilation
Run the command `make` to assemble all files

### Running the programs

#### Coordinator
Run `./coordinator` to execute the program. The shared memory id will be generated and can be used for the other programs. On termination, it will delete the shared memory data and output statistics.

#### Writer
Run `./writer -f {binary_filepath} -l {record_id} -d {delay_in_s} -s {shared_mem_id}` to run the writer. It will modify the file and describe its usage statistics.

#### Reader
Run `./writer -f {binary_filepath} -l {record_ids} -d {delay_in_s} -s {shared_mem_id}` to run the reader. It will read from the file and describe its usage statistics.

#### Monitor
Run `./monitor -s {shared_mem_id}` to run the monitor. Keep this running to see real-time updates across all processes.

### Removing files
Run the command `make clean` to delete all files

## Design Choices

The overall structure of the programs is the coordinator initializing the shared memory, the readers and writers doing their operations with respect to the critical section problem, and the monitor monitoring. The data is logged to log.txt while the readers and writers execute.

Upon invoking a reader/writer, the process checks whether or not it is able to enter the waiting queue for its process type. This is determined by whether or not there are conflicting processes that already exists in the waiting queue. Once done, the process is free to continue. Once done, it sets its flags in the waiting queue to indicate completion, and exits as it describe its usage statistics.

### Semaphores

Critical Section Mutex: Only 1 process at a time should be able to access the critical section. This is done by the critical section mutex semaphore.




### Critical Section Problem

#### Mutual Exclusion

#### Progress

#### Bounded Waiting


*For a working draft of the project's outline and pseudocode, see `reqs.md`.*

@juandapl, @NicholasRaffone, 

27/04/2023
