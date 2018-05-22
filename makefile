falsh : file_search.o
	gcc -o file_seacrh file_search.o 

flash.o : file_search.c 
	gcc -O -Wall -c file_search.c

clean : 
	rm -f file_search.o file_search
