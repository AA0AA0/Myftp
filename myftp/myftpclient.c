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
# include <fcntl.h>
# include <sys/stat.h>
# include <pthread.h>

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
    int file_exist;
    char buff[100];
    unsigned char temp[5] = "myftp";
    memset(buff,0,100);
    memset((void *)&message_box,0,sizeof(message_box));
    struct message_s server_reply;
    memset((void *)& server_reply, 0, sizeof(server_reply));
    
    //LIST_REQUEST
    if (strcmp(argv[3],"list") == 0)
    {
        /* Work if only send this
        memcpy(message_box.protocol,protocol,sizeof(protocol));
        send(sd,(char*)protocol,strlen(protocol),0);
        */
        unsigned char type = 0xA1;
        memcpy(message_box.protocol,temp,5);
        char payload[1024] = "";
        message_box.type = type;
        message_box.length = 5+1+4;
        int len;
        if((len=send(sd,(const char *)&message_box,sizeof(message_box),0))<0)
        {
            printf("Send Error: %s (Errno:%d)\n",strerror(errno),errno);
            exit(0);
        }
        if ((len = recv(sd, (const char *)&server_reply, sizeof(server_reply), 0))< 0) {
            printf("Error in recv server reply\n");
            exit(0);
        }
        if (memcmp(server_reply.protocol, temp, 5) != 0) {
            printf("Wrong Protocol\n");
            exit(0);
        }
        
        if ((len = recv(sd, payload, sizeof(payload), 0)) < 0) {
            printf("Receive Error\n");
            exit(0);
        }
        if (strcmp(payload, "") == 0) {
            printf("NO FILE IS FOUND\n");
            exit(0);
        }
        printf("%s", payload);
    }
    //GET_REQUEST
    if (strcmp(argv[3],"get") == 0)
    {
        char * payload;
        payload = (char *)malloc((strlen(argv[4])+1)*sizeof(char));
        memset(payload,0,strlen(argv[4])+1);
        strcpy(payload, argv[4]);
        payload[strlen(argv[4])] = '\0';
        /*
        if (argv[4][strlen(argv[4])] != '\0')
        {
            payload = (char *)malloc(strlen(argv[4])*sizeof(char));
            strcpy(payload,argv[4]);
            payload[strlen(argv[4])] = '\0';
            printf("%d",strlen(payload));
            printf("%d",sizeof(payload));
        }
        else
        {
            payload = (char *)malloc(strlen(argv[4])-1*sizeof(char));
            strcpy(payload,argv[4]);
        }
        */
        unsigned char type = 0xB1;
        memcpy(message_box.protocol,temp,5);
        message_box.type = type;
        message_box.length = 5+1+4+strlen(payload)+1;
        int len;
        int fn_size = strlen(argv[4])+1;
        if((len=send(sd,(const char *)&message_box,sizeof(message_box),0))<0)
        {
            printf("Send Error: %s (Errno:%d)\n",strerror(errno),errno);
            exit(0);
        }
        if((len=send(sd, payload,fn_size,0))<0)
        {
            printf("Send Error: %s (Errno:%d)\n",strerror(errno),errno);
            exit(0);
        }
        if ((len = recv(sd, (const char *)&server_reply, sizeof(server_reply), 0))<0) {
            printf("Cannot recv server reply");
            exit(0);
        }
        if (server_reply.type == 0xB3) {
            printf("Cannot find the file\n");
            exit(0);
        }
        struct message_s file_data;
        memset((void *)&file_data, 0, sizeof(file_data));
        if ((len = recv(sd, (const char *)&file_data, sizeof(file_data), 0)) < 0) {
            printf("Error in recv file data header\n");
            exit(1);
        }
        if (memcmp(file_data.protocol, temp, 5) != 0) {
            printf("Wrong protocol\n");
            exit(0);
        }
        int size;
        size = file_data.length - 10;
        if (size == 0) {
            printf("The file is empty\n");
            exit(1);
        }

        char buff[512];
        bzero(buff, 512);
        int file_desc;
        if ((file_desc = open(payload, O_CREAT | O_EXCL | O_WRONLY, 0666)) < 0) {
            printf("Cannot create file");
            exit(0);
        }
        int fr_block_sz = 0;
        while ((fr_block_sz = recv(sd, buff, 512, 0)) > 0) {
            int write_sz = write(file_desc, buff, fr_block_sz);
            if (write_sz < fr_block_sz) {
                printf("File write failed\n");
                exit(0);
            }
            bzero(buff, 512);
            if (fr_block_sz == 0 || fr_block_sz != 512) {
                break;
            }
            if (fr_block_sz < 0) {
                printf("Error in recv data\n");
                exit(0);
            }
        }

        printf("Finished recv file\n");
        close(file_desc);
        exit(0);
    }
    //PUT_REQUEST
    if (strcmp(argv[3],"put") == 0)
    {
        
        char* payload;
        payload = (char *)malloc(strlen(argv[4])*sizeof(char));
        strcpy(payload,argv[4]);
        file_exist = find_files(payload, 1);
        
        if (file_exist == 0)
            printf("File not found!\n");
        else
        {
            int server_sd, len;
            int addr_len=sizeof(server_addr);
            struct message_s reply_message;
            memset(&reply_message, 0, sizeof(reply_message));
            int file_desc, file_size, fs_block_sz;
            char buff[512];
            struct stat obj;
            
            stat(payload, &obj);
            file_desc = open(payload, O_RDONLY);
            file_size = obj.st_size;
            
            unsigned char type = 0xC1;
            memcpy(reply_message.protocol,temp,5);
            reply_message.type = type;
            reply_message.length = 5+1+4+strlen(payload)+1;
            int fn_size = strlen(argv[4])+1;
            if((len=send(sd,(const char *)&reply_message,sizeof(reply_message),0))<0)
            {
                printf("Send Error: %s (Errno:%d)\n",strerror(errno),errno);
                pthread_exit(0);
            }
            if((len=send(sd, payload,fn_size,0))<0)
            {
                printf("Send Error: %s (Errno:%d)\n",strerror(errno),errno);
                pthread_exit(0);
            }
            if ((len = recv(sd, (const char *)&server_reply, sizeof(server_reply), 0))<0) {
                printf("Cannot recv server reply");
                pthread_exit(0);
            }
            
            struct message_s file_header;
            memset((void *)&file_header, 0, sizeof(file_header));
            memcpy(file_header.protocol, temp, 5);
            file_header.type = 0xFF;
            file_header.length = 10 + file_size;
            if ((len = send(sd, (const char*)&file_header, sizeof(file_header), 0)) < 0) {
                printf("Cannot send file header\n");
                exit(0);
            }
            
            //               sendfile(client_sd, file_desc, NULL, file_size);
            bzero(buff, 512);
            while ((fs_block_sz = read(file_desc, buff, 512)) > 0) {
                if ((len = send(sd, buff, fs_block_sz, 0)) < 0) {
                    printf("Error in sending buffer\n");
                    pthread_exit(NULL);
                }
                bzero(buff, 512);
            }
            printf("The file is successfully sent\n");
            close(file_desc);
            pthread_exit(NULL);
        }
    }
    return 0;
}
