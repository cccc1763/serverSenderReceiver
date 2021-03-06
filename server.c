#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MAXLINE 1024
#define MAX_CLNT 30
#define MAX_GROUP 10

void *handle_sender_clnt(void *arg) ;
void *handle_receiver_clnt(void *arg) ;
void send_msg(char *msg, int len,char *msgtok) ;
void error_handling(char *msg) ;

const char s[2] = ":"; 

// clientCount
int sender_cnt = 0 ;
int receiver_cnt = 0 ;

// client_socket array
int sender_socks[MAX_CLNT] ;
int receiver_socks[MAX_CLNT] ;

// name, roomName array
char* userName[MAX_CLNT];
char* grp[MAX_GROUP];

// token
char *token1, *token2, *token3;
char *msgtok1, *msgtok2;

pthread_mutex_t mutex;

// Queue를 위한 구조체
typedef struct Node {
    char* msg;
    struct Node *next;
}Node;

typedef struct Queue{
    Node* head;
    Node* tail;
    int count;
}Queue;

// Queuefunc
void InitQueue(Queue* queue);   // Initialize Queue
int IsEmpty(Queue* queue);      // emptyCheck
void EnQueue(Queue* queue, char* m);  // EnQueue
char* DeQueue(Queue* queue);              // DeQueue

Queue queue;

int main(int argc, char *argv[])
{
    // id = Name:Role:RoomName
    char id[MAXLINE];
    int str_len, i;
    int serv_sock, clnt_sock, sender_sock, receiver_sock;
    struct sockaddr_in serv_adr, clnt_adr, sender_adr, receiver_adr;
    int clnt_adr_sz, sender_adr_sz, receiver_adr_sz;
    pthread_t t_id ;
    
    InitQueue(&queue);
    
    if(argc !=2 )
    {
        printf("Usage  : %s <port>\n", argv[0]) ;
        exit(1) ;
    }
    
    // clear terminal
    for(i=0;i<20;i++)
        printf("\n");
    
    pthread_mutex_init(&mutex, NULL) ;
    serv_sock = socket(PF_INET, SOCK_STREAM, 0) ;
    
    // initialize socket
    memset(&serv_adr, 0, sizeof(serv_adr)) ;
    serv_adr.sin_family = AF_INET ;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY) ;
    serv_adr.sin_port = htons(atoi(argv[1])) ;
    
    if((bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr))) < 0)
    {
        error_handling("bind() error") ;
    }
    if(listen(serv_sock, 5) < 0)
    {
        error_handling("listen() error") ;
    }
    
    while(1)
    {
		clnt_adr_sz=sizeof(clnt_adr);
		sender_adr_sz = sizeof(sender_adr);
		receiver_adr_sz = sizeof(receiver_adr);
		
		clnt_sock=accept(serv_sock, (struct sockaddr*)&clnt_adr,&clnt_adr_sz);
		// read first data(id) from client
		read(clnt_sock, id, MAXLINE);
        EnQueue(&queue, id);
        
        // token1:token2:token3 = Name:Role:RoomName
	    token1 = strtok(DeQueue(&queue), s); 
	    token2 = strtok(NULL, s);
	    token3 = strtok(NULL, s);
	    
	    // compare String Role, "sender"
	    if(strcmp(token2,"sender") == 0) {
	        sender_sock = clnt_sock;
	        printf("sender \"%s\" connected\n", token1);
	        if(sender_sock < 0){
	            error_handling("sender accept err");
	        }
	        pthread_mutex_lock(&mutex);
            userName[sender_cnt] = token1;
            grp[sender_cnt] = token3;
	        sender_socks[sender_cnt++]=sender_sock;
		    pthread_mutex_unlock(&mutex);
		    
		    pthread_create(&t_id, NULL, handle_sender_clnt, (void*)&sender_sock);
		    pthread_detach(t_id);
	    }
        // compare String Role, "sender"
	    else if(strcmp(token2, "receiver") == 0) {
	        receiver_sock = clnt_sock;
	        printf("receiver \"%s\" connected\n",token1);
	        
	        if(receiver_sock < 0){
	            error_handling("receiver accept err");
	        }
	        pthread_mutex_lock(&mutex);
	        receiver_socks[receiver_cnt++]=receiver_sock;
		    pthread_mutex_unlock(&mutex);
		    
	        pthread_create(&t_id, NULL, handle_receiver_clnt, (void*)&receiver_sock);
		    pthread_detach(t_id);
	    }
	    
    }
    close(serv_sock) ;
    return 0 ;
}

void *handle_sender_clnt(void *arg)
{
    int sender_sock = *((int*) arg) ;
    int str_len = 0, i ;
    char msg[MAXLINE] ;
    int cmpRoomName=2;
    
    pthread_mutex_lock(&mutex) ;
    for(i = 0; i < sender_cnt-1; i++){
        if(strcmp(grp[i], token3) == 0){
            // is there "RoomName" in array of grp?
            cmpRoomName = 1;
            break;
        }
        cmpRoomName = 0;
    }
    
    //first client makes first room
    if(sender_cnt == 1){
        printf("\n[%s] created room [%s]\n\n", token1, token3);
    }
    //roomName is already in grp
    if(cmpRoomName == 1)
        printf("\n[%s] joined room [%s]\n\n", token1, token3);
    //roomName is not in grp
    else if(cmpRoomName == 0)
        printf("[%s] created room [%s]\n", token1, token3);
    pthread_mutex_unlock(&mutex) ;
    
    // message read
    while((str_len = read(sender_sock, msg, sizeof(msg))) != 0)
    {
        msgtok1 = strtok(msg, s);
        msgtok2 = strtok(NULL, s);
        EnQueue(&queue, msg) ;
        //fputs(DeQueue(&queue),stdout);
        send_msg(DeQueue(&queue), str_len, msgtok2);
        memset(msg,'\0',MAXLINE);
    }
    
    pthread_mutex_lock(&mutex) ;
    // disconnected client management
    for(i = 0; i < sender_cnt; i++)
    {
        if(sender_sock == sender_socks[i])
        {
            while(i++ < sender_cnt-1)
                sender_socks[i] = sender_socks[i+1] ;
            break ;
        }
    }
    sender_cnt-- ;
    pthread_mutex_unlock(&mutex) ;
    
    close(sender_sock) ;
    return NULL ;
}

void *handle_receiver_clnt(void *arg)
{
    int receiver_sock = *((int*) arg) ;
    int str_len = 0, i ;
    char msg[MAXLINE] ;
    str_len = read(receiver_sock, msg, sizeof(msg));

    pthread_mutex_lock(&mutex) ;
    for(i = 0; i < receiver_cnt; i++)
    {
        if(receiver_sock == receiver_socks[i])
        {
            while(i++ < receiver_cnt-1)
                receiver_socks[i] = receiver_socks[i+1] ;
            break ;
        }
    }
    receiver_cnt-- ;
    pthread_mutex_unlock(&mutex) ;
    
    close(receiver_sock) ;
    return NULL ;
}

void send_msg(char *msg, int len,char *msgtok)
{
    int i;
    pthread_mutex_lock(&mutex) ;
    
	for(i=0; i<receiver_cnt; i++) {
	    if(strcmp(grp[i], msgtok) == 0)
		    write(receiver_socks[i], msg, len);
	}
    pthread_mutex_unlock(&mutex) ;
}

void InitQueue(Queue* queue){  
    queue->head = NULL;
    queue->tail = NULL;
    queue->count = 0;
}

int IsEmpty(Queue* queue){
    if(queue->count == 0)
        return 1;
    else
        return 0;
}

void EnQueue(Queue* queue, char* m){
    Node* now = (Node*)malloc(sizeof(Node));
    now->msg = malloc(strlen(m) + 1);  // malloc 미사용 시 헤드가 이동하지 않음
    strcpy(now->msg, m);
    //now->msg = m;   
    now->next = NULL;
    
    if(IsEmpty(queue)){
        //printf("EnQueue empty enter\n");
        queue->head = now;
    }else{
        //printf("EnQueue not empty enter\n");
        queue->tail->next = now;
    }
    queue->tail = now;
    queue->count++;
    
    //printf("now head is %s \n", queue->head->msg);
    //printf("now tail is %s \n", queue->tail->msg);
}

char* DeQueue(Queue* queue){
    char* rtn;
    Node* now;
    if(IsEmpty(queue)){
        return 0;
    }
    now = queue->head;
    rtn = now->msg;
    queue->head = now->next;
    free(now);
    queue->count--;
    return rtn;
}

void error_handling(char *msg)
{
    fputs(msg, stderr);
    fputc('\n', stderr) ;
    exit(1) ;
}