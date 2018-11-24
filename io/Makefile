compile: bin bin/image.o bin/filter.o bin/main.o
	gcc -g bin/image.o bin/filter.o bin/main.o -lm -pthread -oblur

bin: 
	mkdir bin/

bin/image.o: src/image.c
	gcc -g -pthread -o bin/image.o -c src/image.c

bin/filter.o: src/filter.c
	gcc -g -pthread -o bin/filter.o -c src/filter.c

bin/main.o: src/main.c
	gcc -g -pthread -o bin/main.o -c src/main.c

clean_compile: compile
	rm -rf bin

run: compile
	./blur ../test/input/in1.ppm ../test/output/out1.ppm 15 3

exec: compile
	./blur

	
