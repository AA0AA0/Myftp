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

int main ()
{
    create_directory = mkdir("./data", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if (create_directory != 0)
    {
        printf("Error in creating directory!");
    }
    
    dir = opendir("./data");
    while ((dp = readdir(dir)) != NULL)
    {
        printf("filename: %s", dp->d_name);
    }
    
    closedir(dir);
}



