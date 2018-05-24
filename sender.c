#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MAXLINE 1024
#define NAMESIZE 20

void *send_msg(void *arg) ;
void error_handling(char * msg) ;
// ID = "name:msg:grpName"
char msg[MAXLINE] ;
char name[NAMESIZE];
char grpName[NAMESIZE];
char *ID;

int main(int argc, char *argv[])
{
    int sender_sock ;
    int i;
    struct sockaddr_in serv_adr ;
    pthread_t send_thread ;
    void * thread_return ;
    
    if(argc != 5)
    {
        printf("Usage : %s <IP> <port> <id> <group> \n", argv[0]) ;
        exit(1) ;
    }
    
    // clear terminal
    for(i=0;i<20;i++)
        printf("\n");
     
    if(strlen(argv[3]) > 20 || strlen(argv[4]) > 20){
        printf("namesize is over. set namesize below 20bytes\n");
        exit(1);
    }
    
    sender_sock = socket(PF_INET, SOCK_STREAM, 0) ;
    
    memset(&serv_adr, 0, sizeof(serv_adr)) ;
    serv_adr.sin_family = AF_INET ;
    serv_adr.sin_addr.s_addr = inet_addr(argv[1]) ;
    serv_adr.sin_port = htons(atoi(argv[2])) ;
    sprintf(name, "[%s]", argv[3]);
    sprintf(grpName, "%s", argv[4]);
    ID = strcat(argv[3],":sender:");
    strcat(ID,grpName);
    
    printf("ID = \"%s\"\n", ID);
    
    
    if(connect(sender_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr)) == -1)
    {
        error_handling("connect() error") ;
    }
    
    write(sender_sock, ID, strlen(ID)+2);
    pthread_create(&send_thread, NULL, send_msg, (void*)&sender_sock) ;
    pthread_join(send_thread, &thread_return) ;
    close(sender_sock) ;
    return 0 ;
}

void * send_msg(void * arg)
{
    int sender_sock = *((int*) arg) ;
    char name_msg[NAMESIZE+NAMESIZE+MAXLINE];
    
    while(1)
    {
        fputs("input msg : ",stdout);
        fgets(msg, MAXLINE, stdin) ;
        sprintf(name_msg,"%s %s:%s",name, msg, grpName);
        write(sender_sock, name_msg, strlen(name_msg));
    }
    
    return NULL ;
}

void error_handling(char *msg)
{
    fputs(msg, stderr);
    fputc('\n', stderr) ;
    exit(1) ;
}