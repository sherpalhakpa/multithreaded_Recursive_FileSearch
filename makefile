falsh : file_search.o
	gcc -pthread -o file_search file_search.o 

flash.o : file_search.c 
	gcc -pthread -O -Wall -c file_search.c

clean : 
	rm -f file_search.o file_search
