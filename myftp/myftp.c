//
//  myftp.c
//  myftp
//

#include <errno.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

int create_directory;
DIR *dir;
struct dirent *dp;
char * file_name;
char payload[1024];
void* list_files ()
{
    dir = opendir("./data");
    if (dir)
    {
        while ((dp = readdir(dir)) != NULL)
        {
            if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0)
            {
                strcat(payload, dp->d_name);
                strcat(payload, "\n");
                printf("%s\n", dp->d_name);
                //return dp->d_name;
            }
            
        }
        strcat(payload, "\0");
        printf("%s\n", payload);
        closedir(dir);
        return NULL;
    }
    else if (ENOENT == errno)
    {
        create_directory = mkdir("./data", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        if (create_directory != 0)
        {
            printf("Error in creating directory!\n");
        }
    }
    else
    {
        printf("Error in opening directory ./data !\n");
    }
    
    return NULL;
}

int main ()
{
    list_files();
}






