all: writer coordinator reader monitor

clean:
	rm -f writer coordinator reader helpers.o writer.o

writer: writer.o helpers.o
	gcc writer.o helpers.o -o writer -lpthread

monitor: monitor.c shared_structs.h
	gcc monitor.c -o monitor -lpthread

reader: reader.o helpers.o
	gcc reader.o helpers.o -o reader -lpthread

writer.o: writer.c shared_structs.h
	gcc writer.c -c

reader.o: reader.c shared_structs.h
	gcc reader.c -c

coordinator: coordinator.c shared_structs.h
	gcc coordinator.c -o coordinator -lpthread

helpers.o: helpers.c helpers.h
	gcc helpers.c -c