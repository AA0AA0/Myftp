//
//  myftp.c
//  myftp
//

#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

int create_directory;
DIR *dir;
struct dirent *dp;
char * file_name;

void* list_files ()
{
    
    create_directory = mkdir("./data", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if (create_directory != 0)
    {
        printf("Error in creating directory or directory already exists!\n");
    }
    
    dir = opendir("./data");
    
    while ((dp = readdir(dir)) != NULL)
    {
        if (strcmp(dp->d_name, ".") == 1 && strcmp(dp->d_name, "..") == 1)
        {
            printf("%s\n", dp->d_name);
            //return dp->d_name;
        }
        
    }
    closedir(dir);
    return NULL;
    
}

int main ()
{
    list_files();
}






