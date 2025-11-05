/* fpont 12/99 */
/* pont.net    */
/* udpServer.c */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h> /* close() */
#include <string.h> /* memset() */
#include <pthread.h>
#include "fdr.c"

#define LOCAL_SERVER_PORT 6000
#define MAX_MSG 256
#define IP_SIZE 16

char flag = 1;
char tab = 1;
char msgLida = 1;
char msg[MAX_MSG] = "!111.111.111.111;000.000.000.000;Ola\0";
int sd;
struct sockaddr_in roteadorAddr, sendAddr;
pthread_mutex_t mutex_stdin;
pthread_mutex_t mutex_recive;

//=======================================================//

void separaStr(char *buff, char *ipo, char *ipd){
    int count = 1;
    int i = 0;
    while(buff[count] != ';') ipo[i++] = buff[count++];
    ipo[i] = '\0';
    count++;

    i = 0;
    while(buff[count] != ';') ipd[i++] = buff[count++];
    ipd[i] = '\0';
    count++;
    
    i = 0;
    while(buff[count] != '\0') buff[i++] = buff[count++];
    buff[i] = '\0';}

//=======================================================//

void *reciveThread(void*){
    char ipo[IP_SIZE] = ""; //ip origem
    char ipd[IP_SIZE] = ""; //ip destino
    while(flag){
        if(msgLida){
            pthread_mutex_lock(&mutex_recive);
            msgLida = 0;
            //recvfrom(sd, msg, MAX_MSG, 0, (struct sockaddr *) &cliAddr, &cliLen); //mudar os endereços
            if(msg[0] == '!'){
                separaStr(msg, ipo, ipd);
                msgLida = 1;
                pthread_mutex_lock(&mutex_stdin);
                printf("\nIp origem: %s\nIp destino: %s\nMensagem: %s\n%s\n", ipo, ipd, msg, (strcmp(ip, ipd) ? "Mensagem repassada\0" : "Mensagem chegou ao destino\n\0"));
                pthread_mutex_unlock(&mutex_stdin);}
            pthread_mutex_unlock(&mutex_recive);}}}

//=======================================================//

void *terminalThread(void*){
    while(flag){
        char buff[128];
        fgets(buff, 128, stdin);
        if(strcmp(buff, "Tab\n\0") == 0)
            tab = !tab;

        if(strcmp(buff, "exit\n\0") == 0){
            flag = 0;
            return;}
        if(buff[0] != '!') continue;
        int i = 0;
        while(buff[i++] != '\n');
        buff[i-1] = '\0';
        //fazer enviar para a msg para o ip destino
      }
}

//=======================================================//

int main(int argc, char *argv[]) {
    int rc;

    sd=socket(AF_INET, SOCK_DGRAM, 0);
    if(sd<0){
        printf("%s: cannot open socket \n",argv[0]);
        exit(1);}

    sendAddr.sin_family = AF_INET;
    sendAddr.sin_port = htons(LOCAL_SERVER_PORT);

    roteadorAddr.sin_family = AF_INET;
    roteadorAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    roteadorAddr.sin_port = htons(LOCAL_SERVER_PORT);
    rc = bind(sd, (struct sockaddr *) &roteadorAddr, sizeof(roteadorAddr));

    if(rc<0){
        printf("%s: cannot bind port number %d \n", argv[0], LOCAL_SERVER_PORT);
        exit(1);}

    pthread_t thread1;
    pthread_t thread2;
    pthread_create(&thread1, NULL, reciveThread, NULL);
    pthread_create(&thread2, NULL, terminalThread, NULL);

    tabRotInit();
    pthread_mutex_lock(&mutex_stdin);
    printTab();
    pthread_mutex_unlock(&mutex_stdin);
    clock_t t = clock();

    while(flag){
        if((clock() - t)/(CLOCKS_PER_SEC*2) >= 10){
            t = clock();
            sendTab();
            pthread_mutex_lock(&mutex_stdin);
            printTab();
            pthread_mutex_unlock(&mutex_stdin);}

        checkLastUp();
        
        pthread_mutex_lock(&mutex_recive);
        if(msgLida == 0){
            //recive(msg, ""); //descobrir como pegar o ip
            msgLida = 1;}
        pthread_mutex_unlock(&mutex_recive);}

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
return 0;}