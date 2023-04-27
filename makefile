all: writer coordinator reader monitor

clean:
	rm -f writer coordinator reader monitor helpers.o writer.o reader.o coordinator.o monitor.o

writer: writer.o helpers.o
	gcc writer.o helpers.o -o writer -lpthread -lm

monitor: monitor.o helpers.o
	gcc monitor.o helpers.o -o monitor -lpthread -lm

reader: reader.o helpers.o
	gcc reader.o helpers.o -g -o reader -lpthread -lm

coordinator: coordinator.o helpers.o
	gcc coordinator.o helpers.o -o coordinator -lpthread -lm

monitor.o: monitor.c shared_structs.h
	gcc monitor.c -c

writer.o: writer.c shared_structs.h
	gcc writer.c -c

reader.o: reader.c shared_structs.h
	gcc reader.c -c

coordinator.o: coordinator.c shared_structs.h
	gcc coordinator.c -c

helpers.o: helpers.c helpers.h
	gcc helpers.c -c