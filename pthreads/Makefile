CC=cc

compile: bin bin/image.o bin/filter.o bin/main.o
	$(CC) -g bin/image.o bin/filter.o bin/main.o -lm -lpthread -oblur

bin: 
	mkdir bin/

bin/image.o: src/image.c
	$(CC) -g -lpthread -o bin/image.o -c src/image.c

bin/filter.o: src/filter.c
	$(CC) -g -lpthread -o bin/filter.o -c src/filter.c

bin/main.o: src/main.c
	$(CC) -g -lpthread -o bin/main.o -c src/main.c

clean_compile: compile
	rm -rf bin

clean:
	rm -rf bin blur

run: compile
	./blur ../test/input/in1.ppm ../test/output/out1.ppm 15 3

exec: compile
	./blur

	
