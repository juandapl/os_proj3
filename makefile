all: writer coordinator

clean:
	rm -f writer coordinator

writer: writer.c shared_structs.h
	gcc writer.c -o writer -lpthread

coordinator: coordinator.c shared_structs.h
	gcc coordinator.c -o coordinator -lpthread