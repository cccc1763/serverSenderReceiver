#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MAXLINE 1024
#define NAMESIZE 20

void *recv_msg(void *arg) ;
void error_handling(char * msg) ;

// msg, name, grpName 
char msg[MAXLINE] ;
char name[NAMESIZE];
char grpName[NAMESIZE];
char* ID;

int main(int argc, char *argv[])
{
    int receiver_sock;
    struct sockaddr_in serv_adr;
    pthread_t recv_thread;
    void * thread_return;
    int i;
    
    if(argc != 5)
    {
        printf("Usage : %s <IP> <port> <id> <group> \n", argv[0]) ;
        exit(1) ;
    }
    
    //clear terminal
    for(i=0; i<30; i++)
        printf("\n");
        
    receiver_sock = socket(PF_INET, SOCK_STREAM, 0) ;
    
    memset(&serv_adr, 0, sizeof(serv_adr)) ;
    serv_adr.sin_family = AF_INET ;
    serv_adr.sin_addr.s_addr = inet_addr(argv[1]) ;
    serv_adr.sin_port = htons(atoi(argv[2])) ;
    
    sprintf(name, "[%s]", argv[3]);
    sprintf(grpName, "%s", argv[4]);
    ID = strcat(argv[3],":receiver:");
    strcat(ID,grpName);
    
    
    if(connect(receiver_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr)) == -1)
    {
        error_handling("connect() error") ;
    }
    
    write(receiver_sock, ID, strlen(ID)+2);
    
    pthread_create(&recv_thread, NULL, recv_msg, (void*)&receiver_sock) ;
    pthread_join(recv_thread, &thread_return) ;
    close(receiver_sock) ;
    return 0 ;
}

void * recv_msg(void * arg)
{
    int receiver_sock = *((int*) arg) ;
    int str_len = 0; 
    char msg[MAXLINE] ;
    
    while(1)
    {
        read(receiver_sock, msg, sizeof(msg));
        //read(receiver_sock, msg, str_len) ;
        printf("%s",msg);
        //puts(msg);
        memset(msg,'\0',MAXLINE);
        if(str_len == -1)
        {
            return (void*) -1 ;
        }
        //msg[str_len] = 0 ;
    }
    return NULL ;
}

void error_handling(char *msg)
{
    fputs(msg, stderr);
    fputc('\n', stderr) ;
    exit(1) ;
}