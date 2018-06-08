#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h> 
#include <dirent.h> 
#include <string.h> 
#include <sys/time.h> 
#include <pthread.h>
#include <errno.h>

#define threadNum 4
#define max 256
//takes a file/dir as argument, recurses, 
// prints name if empty dir or not a dir (leaves)

void *recur_file_search(void *file); 

struct pathinfo{
	char *path;
	int threadid;
	int empty;
	int skip;
	char *initpath;
};
struct pathinfo *threadarg[threadNum];
int returnVal;
int threadIndex = 1;
const char *search_term;
int total = 0;

pthread_mutex_t mutex; 
pthread_t threads[threadNum];
//share search term globally (rather than passing recursively) 

int main(int argc, char **argv) {

	pthread_mutex_init(&mutex, 0);
	//initial variale and array
	for(int i = 0; i < threadNum; i++){
	threadarg[i] = malloc(sizeof(struct pathinfo));
	threadarg[i]->path = malloc(sizeof(char) * max);
	threadarg[i]->empty = 0;
	threadarg[i]->threadid = -1;
	threadarg[i]->skip = 0;
	}
//	threadarg[0]->initpath = malloc(sizeof(char) * max);



//	cp->path = malloc(sizeof(char) * max);
//	output = malloc(sizeof(struct pathinfo));
//	output->path = malloc(sizeof(char) * max);

	if(argc != 3)
	{
		printf("Usage: my_file_search <search_term> <dir>\n");
		printf("Performs recursive file search for files/dirs matching\
				<search_term> starting at <dir>\n");
		exit(1);
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

	//assign variable for the beginning loop
	threadarg[0]->path = argv[2];
	threadarg[0]->threadid = 0;
	threadarg[0]->empty = 1;

	//assign thread 0 to the beginning of the recursive
	pthread_create(&threads[0],NULL, recur_file_search,(void*)threadarg[0]);


	for(int i = 0; i < threadNum; i++){
	pthread_join(threads[i],NULL);
	}
	//wait for the first thread last, since it won't be able to finish
	//if rest of the thread didn't finish
	pthread_join(threads[0],NULL);

//	for(){

//	}
	gettimeofday(&end, NULL);
	printf("total:%d\n",total);
	printf("Time: %ld\n", (end.tv_sec * 1000000 + end.tv_usec)
			- (start.tv_sec * 1000000 + start.tv_usec));
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
void *recur_file_search(void *file) {
//check if directory is actually a file


	//create a string to save the path to open file
	pthread_mutex_lock(&mutex);
	char *pathcp[threadNum];
//	int full;
	struct pathinfo *cp;
	for(int i = 0; i < threadNum; i++){
	pathcp[i] = malloc(sizeof(char) * max);
	}
	cp = malloc(sizeof(struct pathinfo));
	cp->path = malloc(sizeof(char) * max);
	cp = file;
	int id = cp->threadid;
	char *next_file_str[threadNum];

	//create a copy of the input file struct
	pathcp[id] = cp->path;
//	printf("open[%d]: %s\n",id,pathcp[id]);
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
			total++;
			printf("[%d]%s\n",id, pathcp[id]);}
		//no need to close d (we can't, it is NULL!)

		return NULL;
	}
	//we have a directory, not a file, so check if its name
	// matches the search term
	if(strstr(pathcp[id], search_term) != NULL){
		total++;
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

			if(id == 0){
			threadarg[0]->skip = 0;
			for(int i = 0; i < threadNum; i++){
			if(threadarg[i]->empty == 0){
			threadarg[i]->threadid = i;
			threadarg[i]->path = next_file_str[0];
			threadarg[i]->empty = 1;
			pthread_create(&threads[i],NULL, 
					recur_file_search,(void*)threadarg[i]);
			threadarg[0]->skip = 1;
			break;
			}
			threadarg[i]->empty = pthread_tryjoin_np(threads[i],NULL);

//			if(threadarg[i]->empty == 0){
//			printf("free[%d]\n",i);
//			free(next_file_str[i]);
//			}

			}

			}
			if(threadarg[id]->skip == 0){
			threadarg[id]->threadid = id;
			threadarg[id]->path = next_file_str[id];

			threadarg[id]->empty = 1;
			recur_file_search((void*) threadarg[id]);
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

