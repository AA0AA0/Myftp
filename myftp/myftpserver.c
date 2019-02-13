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
# include <fcntl.h>
# include <sys/socket.h>
# include <sys/types.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <sys/stat.h>
# include <dirent.h>
# include <sys/uio.h>

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
    server_addr.sin_port=htons(atoi(argv[1]));
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
    while(1)
    {
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
            printf("protocol ok\n");
        }
        struct message_s reply_message;
        memset((void *)&reply_message, 0, sizeof(reply_message));
        char reply_payload[1024] = "";
        if (recv_message.type == 0xA1){
            printf("list\n");
            list_files(reply_payload);
            reply_message.length = 10 + strlen(reply_payload);
            memcpy(reply_message.protocol, temp, sizeof(temp));
            reply_message.type = 0xA2;
            if ((len = send(client_sd,(const char *)&reply_message, sizeof(reply_message), 0))< 0) {
                printf("Error in sending reply message\n");
                exit(0);
            }
            if((len = send(client_sd, reply_payload, sizeof(reply_payload), 0)) < 0){
                printf("error in sending payload\n");
            }
            exit(0);
        }
        if (recv_message.type == 0xB1) {
            int filename_size;
            filename_size = recv_message.length - 10;
            char *file;
            file = (char *)malloc(filename_size * sizeof(char));
            printf("%d\n", filename_size);
            if((len=recv(client_sd,file,filename_size,0))<0){
                printf("receive error: %s (Errno:%d)\n", strerror(errno),errno);
                exit(0);
            }
            char* file_name = (char *)malloc((7+ filename_size) * sizeof(char));
            strcpy(file_name, "./data/");
            strcat(file_name, file);
            if (find_files(file, 0) != 1){
             reply_message.length = 10;
             memcpy(reply_message.protocol, temp, 5);
             reply_message.type = 0xB3;
             if ((len = send(client_sd, (const char *)&reply_message, sizeof(reply_message), 0))< 0) {
                 printf("Error in sending reply message\n");
                 exit(1);
                }
                exit(0);
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
                     exit(0);
                 }
//               sendfile(client_sd, file_desc, NULL, file_size);
                 bzero(buff, 512);
                 while ((fs_block_sz = read(file_desc, buff, 512)) > 0) {
                     if ((len = send(client_sd, buff, fs_block_sz, 0)) < 0) {
                         printf("Error in sending buffer\n");
                         exit(0);
                     }
                     bzero(buff, 512);
                 }
                 printf("The file is successfully sent\n");
                 close(file_desc);
                 exit(0);
             }
             exit(0);
             //get_request();
        }
        if (recv_message.type == 0xC1) {
            printf("put");
            printf("%d",recv_message.length);
            reply_message.length = 10;
            memcpy(reply_message.protocol, temp, 5);
            reply_message.type = 0xC2;
            if ((len = send(client_sd, (const char *)&reply_message, sizeof(reply_message), 0))< 0) {
                printf("Error in sending reply message\n");
                exit(1);
            }
            char payload1[1024] = "";
            char buff[512];
            bzero(buff, 512);
            int file_desc;
            if ((len = recv(client_sd, (const char *)&payload1, sizeof(payload1), 0))<0) {
                printf("Cannot recv server reply");
                exit(0);
            }
            printf("%s", payload1);
            char payload[1024] = "./data/";
            strcat(payload, payload1);
            printf("%s", payload);
            struct message_s file_data;
            memset((void *)&file_data, 0, sizeof(file_data));
            if ((len = recv(client_sd, (const char *)&file_data, sizeof(file_data), 0)) < 0) {
                printf("Error in recv file data header\n");
                exit(1);
            }
            if (memcmp(recv_message.protocol, temp,sizeof(temp)) != 0) {
                printf("wrong protocol\n");
                printf("%s\n", recv_message.protocol);
                exit(0);
            }
            int size;
            size = file_data.length - 10;
            if (size == 0) {
                printf("The file is empty\n");
                exit(1);
            }
            if ((file_desc = open(payload, O_CREAT | O_EXCL | O_WRONLY, 0666)) < 0) {
                printf("Cannot create file");
                exit(0);
            }
            int fr_block_sz = 0;
            while ((fr_block_sz = recv(client_sd, buff, 512, 0)) > 0) {
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
            //put_request();
        }
      /*  if(strcmp("exit",buff)==0){
            close(client_sd);
            break;
        }*/
    }
    close(sd);
    return 0;
}
