//
//  myftpclient.c
//  myftp
//

# include "myftp.h"
# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <string.h>
# include <errno.h>
# include <sys/socket.h>
# include <sys/types.h>
# include <netinet/in.h>
# include <arpa/inet.h>

int main(int argc, char** argv){
    int sd=socket(AF_INET,SOCK_STREAM,0);
    struct message_s message_box;
    struct sockaddr_in server_addr;
    /*
     struct sockaddr_in {
     short            sin_family;   // e.g. AF_INET
     unsigned short   sin_port;     // e.g. htons(3490)
     struct in_addr   sin_addr;     // see struct in_addr, below
     char             sin_zero[8];  // zero this if you want to
     };
     struct in_addr {
     unsigned long s_addr;  // load with inet_aton()
     };
    */
    memset(&server_addr,0,sizeof(server_addr));
    server_addr.sin_family=AF_INET;
    /*Change from "127.0.0.1" to argv[1]*/
    server_addr.sin_addr.s_addr=inet_addr(argv[1]);
    /*Change from PORT to (unsigned short)atoi(argv[2])*/
    server_addr.sin_port=htons((unsigned short)atoi(argv[2]));
    if(connect(sd,(struct sockaddr *)&server_addr,sizeof(server_addr))<0){
        printf("connection error: %s (Errno:%d)\n",strerror(errno),errno);
        exit(0);
    }
    
    if (strcmp(argv[3],"list") == 0)
    {
        if (argc != 4)
        {
            printf("ERROR-list: ./myftpclient <server ip addr> <server port> <list>");
            exit(0);
        }
    }
    if (strcmp(argv[3],"get") == 0 || strcmp(argv[3],"put") == 0)
    {
        if (argc != 5)
        {
            printf("ERROR-get|put:./myftpclient <server ip addr> <server port> <get|put> <file>");
            exit(0);
        }
    }
    /*
     Change From host to network
     int le=0x12345678;
     printf("%x\n",le);
     le=htonl(le);
     printf("%x\n",le);
    */
    char buff[100];
    memset(buff,0,100);
    //LIST_REQUEST
    if (strcmp(argv[3],"list") == 0)
    {
        strcpy(message_box.protocol,"myftp");
        printf("%s ",message_box.protocol);
        message_box.type = 0xA1;
        printf("%u ",message_box.type);
        message_box.length = sizeof(struct message_s);
        printf("%d ",message_box.length);
        int len;
        if((len=send(sd,(char **)&message_box,strlen((char*)&message_box),0))<0)
        {
            printf("Send Error: %s (Errno:%d)\n",strerror(errno),errno);
            exit(0);
        }
    }
    /*
    //GET_REQUEST
    if (strcmp(argv[3],"get") == 0)
    {
        int payload_len;
        payload_len = strlen(argv[4]);
        if (argv[4][payload_len-1] != '\n')
        {
            argv[4][payload_len] = '\n';
        }
        strcpy(message_box.protocol,"myftp");
        strcpy(message_box.type,"0xB1");
        message_box.length = sizeof(struct message_s);
        int len;
        if((len=send(sd,(char *)&message_box,strlen((char*)&message_box),0))<0)
        {
            printf("Send Error: %s (Errno:%d)\n",strerror(errno),errno);
            exit(0);
        }
    }
     */
    return 0;
}
