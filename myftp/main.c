//
//  Test_multithread.c
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
# include <dirent.h>
# include <pthread.h>
# include <assert.h>

void *pthread_prog(void *sDescriptor) 
{
  	int client_sd = *(int *)sDescriptor;
  	struct message_s recv_message;
    unsigned char temp[5] = "myftp";
    memset(&recv_message,0,sizeof(recv_message));
    int len;
    if((len=recv(client_sd,(char*)&recv_message,sizeof(recv_message),0))<0)
    {
        printf("receive error: %s (Errno:%d)\n", strerror(errno),errno);
        pthread_exit(NULL);
    }
    if (memcmp(recv_message.protocol, temp,sizeof(temp)) != 0) 
    {
        printf("wrong protocol\n");
        printf("%s\n", recv_message.protocol);
        pthread_exit(NULL);
    }
    else
    {
        printf("protocol ok\n");
    }
    struct message_s reply_message;
    memset(&reply_message, 0, sizeof(reply_message));
    char reply_payload[1024] = "";
    if (recv_message.type == 0xA1){
        printf("list\n");
        list_files(reply_payload);
        reply_message.length = 10 + strlen(reply_payload);
        memcpy(reply_message.protocol, temp, sizeof(temp));
        reply_message.type = 0xA2;
        if ((len = send(client_sd,(const char *)&reply_message, sizeof(reply_message), 0))< 0) {
            printf("Error in sending reply message\n");
            pthread_exit(NULL);
        }
        if((len = send(client_sd, reply_payload, sizeof(reply_payload), 0)) < 0){
            printf("error in sending payload\n");
        }
        pthread_exit(NULL);
    }
    if (recv_message.type == 0xB1) {
        printf("get");
        printf("%d",recv_message.length);
        char file[10];
        if((len=recv(client_sd,(char*)&file,sizeof(file),0))<0){
            printf("receive error: %s (Errno:%d)\n", strerror(errno),errno);
            pthread_exit(NULL);
        }
        printf("%s",file);
        pthread_exit(NULL);
        //get_request();
    }
    if (recv_message.type == 0xC1) 
    {
        printf("put");
        printf("%d",recv_message.length);
        pthread_exit(NULL);
            //put_request();
    }
      /*  if(strcmp("exit",buff)==0){
            close(client_sd);
            break;
        }*/
    pthread_exit(NULL);
}

int main(int argc, char** argv)
{
	int sd=socket(AF_INET,SOCK_STREAM,0);
    long val = 1;
    if (setsockopt(sd,SOL_SOCKET,SO_REUSEADDR, &val, sizeof(long)) == -1)
    {
        perror("setsocopt");
        exit(1);
    }
    struct sockaddr_in server_addr;
    memset(&server_addr,0,sizeof(server_addr));
    server_addr.sin_family=AF_INET;
    server_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    server_addr.sin_port=htons(atoi(argv[1]));
    if(bind(sd,(struct sockaddr *) &server_addr,sizeof(server_addr))<0){
        printf("bind error: %s (Errno:%d)\n",strerror(errno),errno);
        exit(0);
    }
    if(listen(sd,3)<0){
        printf("listen error: %s (Errno:%d)\n",strerror(errno),errno);
        exit(0);
    }
	while(1)
	{
		int client_sd;
		struct sockaddr_in client_addr;
		int addr_len=sizeof(client_addr);
		pthread_t worker;
    	if((client_sd=accept(sd,(struct sockaddr *) &client_addr,&addr_len))<0)
    	{
        	printf("accept erro: %s (Errno:%d)\n",strerror(errno),errno);
        	exit(0);
        }
        pthread_create(&worker, NULL, pthread_prog, &client_sd);
        printf("hi");
	}
}
