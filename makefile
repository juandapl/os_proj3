all: writer coordinator reader

clean:
	rm -f writer coordinator reader

reader: reader.c shared_structs.h
	gcc reader.c -o reader -lpthread

writer: writer.c shared_structs.h
	gcc writer.c -o writer -lpthread

coordinator: coordinator.c shared_structs.h
	gcc coordinator.c -o coordinator -lpthread