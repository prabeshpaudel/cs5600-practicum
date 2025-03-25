all: main  

cache.o: cache.c cache.h
	gcc -c cache.c

message.o: message.c message.h 
	gcc -c message.c

main: message.o cache.o main.o  
	gcc -o main  message.o cache.o main.o

main.o: main.c message.h cache.h 
	gcc -c main.c

clean:
	rm -f main.o message.o cache.o main 