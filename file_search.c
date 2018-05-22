#include<stdio.h>
#include<sys/types.h>
#include<dirent.h>
#include<string.h>
#include<errno.h>
#include<time.h>
//recursive function
//takes in the starting directory and the term to look for as parameters 
void file_search(char * path, char * term)
{
	DIR * dir;//DIR type for starting directory
	struct dirent *dit;//other files or directorirs in the parent directory
	if((dir = opendir(path))==NULL)//if couldn't open directory
        {
	printf("error");
	return;
	}
        while((dit = readdir(dir))!=NULL)//until the last child file/directory is read
        {
		char * current = dit->d_name;
		//strcasestr is case incensetive
		if(strcasestr(current,term)!=NULL)//look for search term
     		{
                    printf("%s",current);//if substr found print the dirent name
		}
		//if dirent is directory
		//recursively do the same function
		if(dit->d_type == DT_DIR)
                {
		//if directory print ":" at last
                printf(":\n");
                file_search(current,term);
		}
		else
		//not a directory just go to next line
                printf("\n");
        }
	//didn't close
	if ((closedir(dir))==-1)
	{
		printf("error");
		return;
	}

}
int main(int argc, char * argv[])
{
	if(argc > 3)//if more than 3 args passed
	{
		printf("Invalid command!\n");
		printf("usage: file_search <search term> <starting directory>");
        }
        clock_t start,end;
	start = clock();//keep track of time
        file_search(argv[2],argv[1]);//call file search function
        end = clock();//end time
        double timeTaken = (double)(end - start);//time taken
	printf("Time: %f seconds\n",timeTaken);
	return 1;
}
