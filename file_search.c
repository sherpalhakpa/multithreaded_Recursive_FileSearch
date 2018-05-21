#include<stdio.h>
#include<sys/types.h>
#include<dirent.h>
#include<string.h>
#include<errno.h>
#include<time.h>
void file_search(char * path, char * term)
{
	DIR * dir;
	struct dirent *dit;
	if((dir = opendir(path))==NULL)
        {
	printf("error");
	return;
	}
        while((dit = readdir(dir))!=NULL)
        {
		if(strcasestr(dit->d_name,term)!=NULL)
     		{
                    printf("%s",dit->d_name);
		}
		if(dit->d_type == DT_DIR)
                {
                printf(":\n");
                file_search(dit->d_name,term);
		}
		else
                printf("\n");
        }
	int closedir(DIR * dir);
}
int main(int argc, char * argv[])
{
	if(argc > 3)
	{
		printf("Invalid command!\n");
		printf("usage: file_search <search term> <starting directory>");
        }
        clock_t start,end;
	start = clock();
        file_search(argv[2],argv[1]);
        end = clock();
        double timeTaken = (double)(end - start);
	printf("Time: %f seconds\n",timeTaken);
	return 1;
}
