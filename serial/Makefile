compile: bin bin/image.o bin/filter.o bin/main.o
	gcc -g bin/image.o bin/filter.o bin/main.o -lm -oblur

bin: 
	mkdir bin/

bin/image.o: src/image.c
	gcc -g -o bin/image.o -c src/image.c

bin/filter.o: src/filter.c
	gcc -g -o bin/filter.o -c src/filter.c

bin/main.o: src/main.c
	gcc -g -o bin/main.o -c src/main.c

clean_compile: compile
	rm -rf bin

exec: compile
	./blur

	
