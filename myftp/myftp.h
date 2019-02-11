//
//  myftp.h
//  myftp
//

#ifndef myftp_h
#define myftp_h
#include <dirent.h>
#include <sys/types.h>

struct message_s
{
    unsigned char protocol[5];      /* protocol string (5 bytes) */
    unsigned char type;             /* type (1 byte) */
    unsigned int length;            /* length (header + payload) (4 bytes) */
} __attribute__ ((packed));

char* list_files (char* payload);

int find_files(char* filename, char* addr);
#endif /* myftp_h */
