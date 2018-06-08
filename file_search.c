#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>//for thread
#include <errno.h>

#define threadNum 4
#define max 256
//takes a file/dir as argument, recurses,
// prints name if empty dir or not a dir (leaves)
void *recur_file_search(void *file);

struct threadPath{
	char *path;
	int threadid;
	int empty;
	int skip;
	char *initialPath;
};

//I listed these here so they are globally accesible
//I assume global variables are bad but
//couldn't think a better way to implement it here


//pointers to the structs
struct threadPath *mythread[threadNum];
//index to tarck treads
int threadIndex = 1;
const char *search_term;
//share search term globally (rather than passing recursively)

//how many times in total found
int occurance = 0;

//mutex helps us make sure a code section is only accessible by one thread at a time
pthread_mutex_t mutex;
pthread_t threads[threadNum];


int main(int argc, char **argv) {
	if(argc != 3)
	{
		printf("Usage: my_file_search <search_term> <dir>\n");
		printf("Performs recursive file search for files/dirs matching\
				<search_term> starting at <dir>\n");
		exit(1);
	}

    //explicit initialization of mutex
    //just use the default behaviour (Null as attribute)
	pthread_mutex_init(&mutex, 0);
	//initial variale and array
	for(int i = 0; i < threadNum; i++){
	mythread[i] = malloc(sizeof(struct threadPath));
	mythread[i]->path = malloc(sizeof(char) * max);
	mythread[i]->empty = 0;
	mythread[i]->threadid = -1;
	mythread[i]->skip = 0;
	}
	//don't need to bother checking if term/directory are swapped, since we can't
	// know for sure which is which anyway
	search_term = argv[1];
	//open the top-level directory
	DIR *dir = opendir(argv[2]);
	//make sure top-level dir is openable (i.e., exists and is a directory)
	if(dir == NULL)
	{
		perror("opendir failed");
		exit(1);
	}

	//start timer for recursive search

	struct timeval start, end;
	gettimeofday(&start, NULL);

	//get values to begin the loop
	mythread[0]->path = argv[2];
	mythread[0]->threadid = 0;
	mythread[0]->empty = 1;

	//first thread to begin the file search
	pthread_create(&threads[0],NULL, recur_file_search,(void*)mythread[0]);

        //wait/join threads
	for(int i = 0; i < threadNum; i++){
	pthread_join(threads[i],NULL);
	}

	//wait for the first thread last
	//in case rest of the thread didn't finish
	//first won't finish
	pthread_join(threads[0],NULL);

	//get end time
	gettimeofday(&end, NULL);

	printf("total:%d\n",occurance);
	printf("Time: %ld\n", (end.tv_sec * 1000000 + end.tv_usec)
			- (start.tv_sec * 1000000 + start.tv_usec));
	//Done with the mutex erase it
	pthread_mutex_destroy(&mutex);
	return 0;
}
//This function takes a path to recurse on, searching for mathes to the
// (global) search_term.  The base case for recursion is when *file is
// not a directory.
//Parameters: the starting path for recursion (char *), which could be a
// directory or a regular file (or something else, but we don't need to
// worry about anything else for this assignment).
//Returns: nothing
//Effects: prints the filename if the base case is reached *and* search_term
// is found in the filename; otherwise, prints the directory name if the directory
// matches search_term.

//most recent implementation of the function uses four pthreads
//to carry on the recursive search
void *recur_file_search(void *file)
{
	//start ctritical code (mutex lock)
	// and create a string to save the path to open file
        pthread_mutex_lock(&mutex);
	char *pathcp[threadNum];

	struct threadPath *cp;
	for(int i = 0; i < threadNum; i++){
	pathcp[i] = malloc(sizeof(char) * max);
	}
	cp = malloc(sizeof(struct threadPath));
	cp->path = malloc(sizeof(char) * max);
	cp = file;
	int id = cp->threadid;
	char *next_file_str[threadNum];

	//create a copy of the input file struct
	pathcp[id] = cp->path;
	DIR *d[threadNum];
	d[id] = opendir(pathcp[id]);
	pthread_mutex_unlock(&mutex);

	//NULL means not a directory (or another, unlikely error)
	if(d[id] == NULL)
	{
		//opendir SHOULD error with ENOTDIR, but if it did something else,
		// we have a problem (e.g., forgot to close open files, got
		// EMFILE or ENFILE)
		if(errno != ENOTDIR)
		{
			perror("Something weird happened!");
			fprintf(stderr, "While looking at[%d]: %s\n",id, pathcp[id]);
			exit(1);
		}
		//nothing weird happened, check if the file contains the search term
		// and if so print the file to the screen (with full path)
		if(strstr(pathcp[id], search_term) != NULL){
			occurance++;
			printf("[%d]%s\n",id, pathcp[id]);}
		//no need to close d (we can't, it is NULL!)

		return NULL;
	}
	//we have a directory, not a file, so check if its name
	// matches the search term
	if(strstr(pathcp[id], search_term) != NULL){
		occurance++;
		printf("[%d]%s/\n",id, pathcp[id]);}
	//call recur_file_search for each file in d
	//readdir "discovers" all the files in d, one by one and we
	// recurse on those until we run out (readdir will return NULL)
	struct dirent *cur_file;
	while((cur_file = readdir(d[id])) != NULL)
	{
		//make sure we don't recurse on . or ..
		if(strcmp(cur_file->d_name, "..") != 0 &&\
				strcmp(cur_file->d_name, ".") != 0)
		{
			//we need to pass a full path to the recursive function,
			// so here we append the discovered filename (cur_file->d_name)
			// to the current path (file -- we know file is a directory at
			// this point)
			next_file_str[id] = malloc(sizeof(char) *
					strlen(cur_file->d_name) +
					strlen(pathcp[id]) + 2);

			strncpy(next_file_str[id],pathcp[id], strlen(pathcp[id]));

			strncpy(next_file_str[id] + strlen(pathcp[id]),
					"/", 1);
			strncpy(next_file_str[id] + strlen(pathcp[id]) + 1,
					cur_file->d_name,
					strlen(cur_file->d_name) + 1);
//recurse on the file

            //only allow first thread to assign
			if(id == 0)
			{
			mythread[0]->skip = 0;
			for(int i = 0; i < threadNum; i++){
			if(mythread[i]->empty == 0){//if thread is doing nothing
			mythread[i]->threadid = i;
			mythread[i]->path = next_file_str[0];//give work
			mythread[i]->empty = 1;//not free anymore thread has work
			pthread_create(&threads[i],NULL,
					recur_file_search,(void*)mythread[i]);
			mythread[0]->skip = 1;//thread is created
			//exit loop. won't create all threads
			break;
			}
			//check if a thread is empty
			mythread[i]->empty = pthread_tryjoin_np(threads[i],NULL);
			}
			//rest of the threads(not thread 0) that hasn't assigned a thread
			//will run in the recursive function

			}
			if(mythread[id]->skip == 0){
			mythread[id]->threadid = id;
			mythread[id]->path = next_file_str[id];

			mythread[id]->empty = 1;
			recur_file_search((void*) mythread[id]);
			}
//free the dynamically-allocated string

			if(id != 0){
			free(next_file_str[id]);
			}

		}

	}
//close the directory, or we will have too many files opened (bad times)
	closedir(d[id]);
}


