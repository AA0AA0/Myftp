//
//  myftpserver.c
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
# include <sys/stat.h>
#include <dirent.h>

# define PORT 12345
void list_request();
void get_request();
void put_request();


void* list_files ()
{
    int create_directory;
    DIR *dir;
    struct dirent *dp;
    char * file_name;
    
    dir = opendir("./data");
    if (dir)
    {
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

int main(int argc, char** argv){
    int sd=socket(AF_INET,SOCK_STREAM,0);
    long val = 1;
    if (setsockopt(sd,SOL_SOCKET,SO_REUSEADDR, &val, sizeof(long)) == -1)
    {
        perror("setsocopt");
        exit(1);
    }
    int client_sd;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    memset(&server_addr,0,sizeof(server_addr));
    server_addr.sin_family=AF_INET;
    server_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    server_addr.sin_port=htons(PORT);
    if(bind(sd,(struct sockaddr *) &server_addr,sizeof(server_addr))<0){
        printf("bind error: %s (Errno:%d)\n",strerror(errno),errno);
        exit(0);
    }
    if(listen(sd,3)<0){
        printf("listen error: %s (Errno:%d)\n",strerror(errno),errno);
        exit(0);
    }
    int addr_len=sizeof(client_addr);
    if((client_sd=accept(sd,(struct sockaddr *) &client_addr,&addr_len))<0){
        printf("accept erro: %s (Errno:%d)\n",strerror(errno),errno);
        exit(0);
    }
    while(1){
        /* Work if only this part
        unsigned char myftp[5] = "myftp";
        unsigned char protocol[5];
        recv(client_sd,(char*)&protocol,sizeof(protocol),0);
        if (memcmp(protocol, myftp,sizeof(protocol)) != 0) {
            printf("wrong protocol\n");
            exit(0);
        }
        else
        {
            printf("ok");
            exit(0);
        }
        */
        struct message_s recv_message;
        unsigned char temp[5] = "myftp";
        memset(&recv_message,0,sizeof(recv_message));
        int len;
        if((len=recv(client_sd,(char*)&recv_message,sizeof(recv_message),0))<0){
            printf("receive error: %s (Errno:%d)\n", strerror(errno),errno);
            exit(0);
        }
        if (memcmp(recv_message.protocol, temp,sizeof(temp)) != 0) {
            printf("wrong protocol\n");
            printf("%s\n", recv_message.protocol);
            exit(0);
        }
        else
        {
            printf("protocol ok");
        }
        if (recv_message.type == 0xA1){
            printf("list\n");
            list_files();
            exit(0);
            //list_request();
        }
        /*
        if (strcmp(recv_message.type, "0xB1") == 0) {
            printf("get");
            //get_request();
        }
        if (strcmp(recv_message.type, "0xC1") == 0) {
            printf("put");
            //put_request();
        }*/
    }
    close(sd);
    return 0;
}
