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

# define PORT 12345
void list_request();
void get_request();
void put_request();


int main(int argc, char** argv){
    int sd=socket(AF_INET,SOCK_STREAM,0);
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
        char** buffer = (char**)malloc(sizeof(char**));
        struct message_s recv_message;
        memset(&recv_message,0,sizeof(recv_message));
        int len;
        if((len=recv(client_sd, buffer,sizeof(buffer),0))<0){
            printf("receive error: %s (Errno:%d)\n", strerror(errno),errno);
            exit(0);
        }
        buffer[len] = 0;
        /*
        if (strcmp(recv_message.protocol, "myftp") != 0) {
            printf("wrong protocol\n");
            printf("%s\n", recv_message.protocol);
            exit(0);
        }
        if (strcmp(recv_message.type, "0xA1") == 0) {
            printf("list");
            //list_request();
        }
        if (strcmp(recv_message.type, "0xB1") == 0) {
            printf("get");
            //get_request();
        }
        if (strcmp(recv_message.type, "0xC1") == 0) {
            printf("put");
            //put_request();
        }*/
        printf("RECEIVED INFO: ");
        printf("%s\n", buffer);
        free(buffer);
    }
    close(sd);
    return 0;
}
