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
        free(sDescriptor);
        pthread_exit(NULL);
    }
    if (memcmp(recv_message.protocol, temp,sizeof(temp)) != 0) 
    {
        printf("wrong protocol\n");
        free(sDescriptor);
        pthread_exit(NULL);
    }
    else
    {
        //printf("protocol ok\n");
        //printf("%s\n",recv_message.protocol);
        //printf("%u\n",recv_message.type);
        //printf("%u\n",recv_message.length);
    }
    //printf("halo\n");
    struct message_s reply_message;
    memset(&reply_message, 0, sizeof(reply_message));
    char reply_payload[1024] = "";
    if (recv_message.type == 0xA1){
        list_files(reply_payload);
        reply_message.length = 10 + strlen(reply_payload);
        memcpy(reply_message.protocol, temp, sizeof(temp));
        reply_message.type = 0xA2;
        if ((len = send(client_sd,(const char *)&reply_message, sizeof(reply_message), 0))< 0) {
            printf("Error in sending reply message\n");
            free(sDescriptor);
            pthread_exit(NULL);
        }
        if((len = send(client_sd, reply_payload, sizeof(reply_payload), 0)) < 0){
            printf("error in sending payload\n");
        }
        free(sDescriptor);
        pthread_exit(NULL);
    }
    if (recv_message.type == 0xB1) {
        int filename_size;
        filename_size = recv_message.length - 10;
        char *file;
        file = (char *)malloc(filename_size * sizeof(char));
        memset(file,0,(filename_size * sizeof(char)));        //key changes
        //print_bytes(file, sizeof(file) + 10);
        if((len=recv(client_sd,file,filename_size,0))<0){
            printf("receive error: %s (Errno:%d)\n", strerror(errno),errno);
            free(sDescriptor);
            pthread_exit(NULL);
        }
        char* file_name = (char *)malloc((7+ filename_size) * sizeof(char));
        strcpy(file_name, "./data/");
        strcat(file_name, file);
        //printf("%s\n",file);
        //print_bytes(file, sizeof(file) + 10);
        //printf("%d\n",filename_size);
        if (find_files(file, 0) != 1){
            reply_message.length = 10;
            memcpy(reply_message.protocol, temp, 5);
            reply_message.type = 0xB3;
            if ((len = send(client_sd, (const char *)&reply_message, sizeof(reply_message), 0))< 0) {
                printf("Error in sending reply message\n");
                free(sDescriptor);
                pthread_exit(NULL);
            }
            free(sDescriptor);
            pthread_exit(NULL);
        }
        else {
            int file_desc, file_size, fs_block_sz, len_r;
            char buff[BUFF_SIZE];
            unsigned short check_sum;
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
            file_header.length = htonl(file_header.length);
            if ((len = send(client_sd, (const char*)&file_header, sizeof(file_header), 0)) < 0) {
                printf("Cannot send file header\n");
                free(sDescriptor);
                pthread_exit(NULL);
            }
            //               sendfile(client_sd, file_desc, NULL, file_size);
            bzero(buff, BUFF_SIZE);
            while (1) {
                if ((fs_block_sz = read(file_desc, buff, BUFF_SIZE)) <= 0) {
                    break;
                }
                if ((len = send(client_sd, buff, fs_block_sz, 0)) < 0) {
                    printf("Error in sending buffer, Error no:%d\n", errno);
                    printf("%s\n", buff);
                    printf("%d %d\n", len, fs_block_sz);
                    free(sDescriptor);
                    pthread_exit(NULL);
                }
                if ((len_r = recv(client_sd, &check_sum , sizeof(int), 0)) < 0) {
                    printf("Error in recv check sum\n");
                    free(sDescriptor);
                    pthread_exit(NULL);
                }
                else{
                    check_sum = ntohs(check_sum);
                    printf("%d\n", check_sum);
                    if (check_sum != len) {
                        lseek(file_desc, 0-len, SEEK_CUR);
                    }
                }
                printf("%d %d\n", len, fs_block_sz);
                bzero(buff, BUFF_SIZE);
            }
            if ((len = send(client_sd, buff ,BUFF_SIZE, 0))<0) {
                printf("Cannot send E_O_F\n");
                free(sDescriptor);
                pthread_exit(NULL);
            }
            printf("The file is successfully sent\n");
            close(file_desc);
            free(sDescriptor);
            pthread_exit(NULL);
        }
        //get_request();
    }
    if (recv_message.type == 0xC1) 
    {

        reply_message.length = 10;
        memcpy(reply_message.protocol, temp, 5);
        reply_message.type = 0xC2;
        if ((len = send(client_sd, (const char *)&reply_message, sizeof(reply_message), 0))< 0) {
            printf("Error in sending reply message\n");
            exit(1);
        }
        char payload1[1024] = "";
        char buff[BUFF_SIZE];
        bzero(buff, BUFF_SIZE);
        int file_desc;
        if ((len = recv(client_sd, (const char *)&payload1, sizeof(payload1), 0))<0) {
            printf("Cannot recv server reply");
            exit(0);
        }
        
        char payload[1024] = "./data/";
        strcat(payload, payload1);
        struct message_s file_data;
        memset((void *)&file_data, 0, sizeof(file_data));
        if ((len = recv(client_sd, (const char *)&file_data, sizeof(file_data), 0)) < 0) {
            printf("Error in recv file data header\n");
            free(sDescriptor);
            pthread_exit(NULL);
        }
        if (memcmp(recv_message.protocol, temp,sizeof(temp)) != 0) {
            printf("wrong protocol\n");
            free(sDescriptor);
            pthread_exit(NULL);
        }
        int size;
        file_data.length = ntohl(file_data.length);
        size = file_data.length - 10;
        printf("%d\n",size);
        if (size == 0) {
            printf("The file is empty\n");
            free(sDescriptor);
            pthread_exit(NULL);
        }
        if ((file_desc = open(payload, O_WRONLY | O_CREAT | O_TRUNC, 0666)) < 0) {
            printf("Cannot create file");
            free(sDescriptor);
            pthread_exit(NULL);
        }
        int fr_block_sz = 0;
        unsigned short check_sum;
        while (1) {
            if ((fr_block_sz = recv(client_sd, buff, BUFF_SIZE, 0)) <= 0) {
                break;
            }
            printf("%d\n", fr_block_sz);
            check_sum = (unsigned short) fr_block_sz;
            check_sum = htons(check_sum);
            if ((len = send(client_sd, &check_sum , sizeof(unsigned short), 0)) < 0) {
                printf("Error in sending check sum\n");
                exit(0);
            }
            if (check_sum != BUFF_SIZE) {
                if (size - check_sum != 0) {
                    continue;
                }
            }
            int write_sz = write(file_desc, buff, fr_block_sz);
            if (write_sz < fr_block_sz) {
                printf("File write failed\n");
                exit(0);
            }
            printf("%d %d\n", write_sz, fr_block_sz);
            size -= write_sz;
            printf("size: %d\n", size);
            bzero(buff, BUFF_SIZE);
            if (size == 0 /*|| fr_block_sz != BUFF_SIZE*/) {
                break;
            }
            if (fr_block_sz < 0) {
                printf("Error in recv data\n");
                exit(0);
            }
        }
        if (size != 0) {
            printf("Recv file has wrong size\n");
            printf("Remainig file size: %d\n", size);
            free(sDescriptor);
            pthread_exit(NULL);
        }
        printf("Finished recv file\n");
        close(file_desc);
        free(sDescriptor);
        pthread_exit(NULL);
            //put_request();
    }
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
    int *new_sock;
    struct sockaddr_in client_addr;
    memset(&client_addr,0,sizeof(client_addr));
    int addr_len=sizeof(client_addr);
	while(client_sd=accept(sd,(struct sockaddr *) &client_addr,&addr_len))
	{
		pthread_t worker;
        new_sock = malloc(1);
        *new_sock = client_sd;
        pthread_create(&worker, NULL, pthread_prog, (void*) new_sock);
//        pthread_join(worker,NULL);
        memset(&client_addr,0,sizeof(client_addr));
	}
}
