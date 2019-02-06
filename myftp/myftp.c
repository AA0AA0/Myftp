//
//  myftp.c
//  myftp
//

#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

int create_directory;

int main ()
{
    create_directory = mkdir("data/", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if (create_directory != 0)
    {
        printf("Error in creating directory!");
    }
    
    opendir("data");
}



