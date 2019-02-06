//
//  myftp.c
//  myftp
//

#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

struct message_s
{
    unsigned char protocol[5];      /* protocol string (5 bytes) */
    unsigned char type;             /* type (1 byte) */
    unsigned int length;            /* length (header + payload) (4 bytes) */
} __attribute__ ((packed));

typedef struct message_s message_s;

int create_directory;

int main ()
{
    create_directory = mkdir("data/", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if (create_directory != 0)
    {
        printf("Unsuccessful!");
    }
}



