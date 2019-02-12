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
# include <fcntl.h>
# include <sys/socket.h>
# include <sys/types.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <sys/stat.h>
# include <dirent.h>
# include <pthread.h>
# include <assert.h>
# include <sys/uio.h>

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
    printf("halo\n");
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
        int filename_size;
        filename_size = recv_message.length - 10;
        char *file;
        file = (char *)malloc(filename_size * sizeof(char));
        memset(file,0,sizeof(file));
        if((len=recv(client_sd,file,filename_size,0))<0){
            printf("receive error: %s (Errno:%d)\n", strerror(errno),errno);
            pthread_exit(NULL);
        }
        char* file_name = (char *)malloc((7+ filename_size) * sizeof(char));
        strcpy(file_name, "./data/");
        strcat(file_name, file);
        printf("%s",file);
        if (find_files(file, 0) != 1){
            reply_message.length = 10;
            memcpy(reply_message.protocol, temp, 5);
            reply_message.type = 0xB3;
            if ((len = send(client_sd, (const char *)&reply_message, sizeof(reply_message), 0))< 0) {
                printf("Error in sending reply message\n");
                pthread_exit(NULL);
            }
            pthread_exit(NULL);
        }
        else {
            int file_desc, file_size, fs_block_sz;
            char buff[512];
            struct stat obj;
            stat(file_name, &obj);
            file_desc = open(file_name, O_RDONLY);
            file_size = obj.st_size;
            reply_message.length = 10;
            reply_message.type = 0xB2;
            memcpy(reply_message.protocol, temp, 5);
            send(client_sd, (const char *)&reply_message, sizeof(reply_message), 0);
            struct message_s file_header;
            memset((void *)&file_header, 0, sizeof(file_header));
            memcpy(file_header.protocol, temp, 5);
            file_header.type = 0xFF;
            file_header.length = 10 + file_size;
            if ((len = send(client_sd, (const char*)&file_header, sizeof(file_header), 0)) < 0) {
                printf("Cannot send file header\n");
                pthread_exit(NULL);
            }
            //               sendfile(client_sd, file_desc, NULL, file_size);
            bzero(buff, 512);
            while ((fs_block_sz = read(file_desc, buff, 512)) > 0) {
                if ((len = send(client_sd, buff, fs_block_sz, 0)) < 0) {
                    printf("Error in sending buffer\n");
                    pthread_exit(NULL);
                }
                bzero(buff, 512);
            }
            printf("The file is successfully sent\n");
            close(file_desc);
            pthread_exit(NULL);
        }
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
    
    int client_sd;
    struct sockaddr_in client_addr;
    int addr_len=sizeof(client_addr);
	while(client_sd=accept(sd,(struct sockaddr *) &client_addr,&addr_len))
	{
		pthread_t worker;
        pthread_create(&worker, NULL, pthread_prog, &client_sd);
        printf("hi");
        pthread_join(worker,NULL);
	}
}
