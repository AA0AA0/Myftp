//
//  myftp.c
//  myftp
//

#include "myftp.h"
#include <errno.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

void print_bytes(void *ptr, int size)
{
    unsigned char *p = ptr;
    int i;
    for (i=0; i<size; i++) {
        printf("%02hhX ", p[i]);
    }
    printf("\n");
}

char* list_files (char* payload)
{
    int create_directory;
    DIR *dir;
    struct dirent *dp;
    dir = opendir("./data");
    if (dir)
    {
        while ((dp = readdir(dir)) != NULL)
        {
            if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0)
            {
                strcat(payload, dp->d_name);
                strcat(payload, "\n");
            }
        }
        strcat(payload, "\0");
        closedir(dir);
        return payload;
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
    closedir(dir);
    return NULL;
}

int find_files(char* filename, int flag)
{
 //   printf("arrive");
    DIR *dir;
    struct dirent *dp;
    if (flag == 1) {
        dir = opendir(".");
    }
    else {
        dir = opendir("./data");
    }
    if (dir)
    {
        while ((dp = readdir(dir)) != NULL)
        {
            //printf("%s\n", dp->d_name);
            if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0 && strcmp(filename, dp->d_name) == 0)
            {
                printf("Successfully find file %s\n", dp->d_name);
                //printf("%p\n",dp->d_name);
 //               print_bytes(dp->d_name, sizeof(dp->d_name));
                closedir(dir);
                return 1;
            }
        }
        closedir(dir);
    }
    else
    {
        printf("Error in opening directory ./data !\n");
    }
    return 0;
}
