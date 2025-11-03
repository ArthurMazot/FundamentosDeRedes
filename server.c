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
#include "fdr.c"

#define LOCAL_SERVER_PORT 1500 // trocar pra 6000
#define MAX_MSG 256

int main(int argc, char *argv[]) {
  
  int sd, rc, n, cliLen;
  struct sockaddr_in cliAddr, servAddr;
  char msg[MAX_MSG];

  /* socket creation */
  sd=socket(AF_INET, SOCK_DGRAM, 0);
  if(sd<0){
    printf("%s: cannot open socket \n",argv[0]);
    exit(1);}

  /* bind local server port */
  servAddr.sin_family = AF_INET;
  servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servAddr.sin_port = htons(LOCAL_SERVER_PORT);
  rc = bind(sd, (struct sockaddr *) &servAddr,sizeof(servAddr));
  if(rc<0){
    printf("%s: cannot bind port number %d \n", argv[0], LOCAL_SERVER_PORT);
    exit(1);}

  printf("%s: waiting for data on port UDP %u\n", argv[0],LOCAL_SERVER_PORT);

  /* init buffer */
  memset(msg,0x0,MAX_MSG);
  tabRotInit(); //inicia a tabela de roteamento
  printTab(); //printa a tabela de roteamento
  clock_t t = clock();
  /* server infinite loop */
  while(1){
    /* receive message */
    if((clock() - t)/CLOCKS_PER_SEC >= 10){ //envia a tabela de roteamento de 10 em 10 segundos
      t = clock();
      sendTab();}
    
    checkLastUp(); //verifica se alguma rota não foi atualizada nos ultimos 15 segundos
    
    //fazer um while para limpar a fila de msg
    cliLen = sizeof(cliAddr);
    n = recvfrom(sd, msg, MAX_MSG, 0, (struct sockaddr *) &cliAddr, &cliLen);
    //recive(msg, ???) Precisa de um ip
    printTab(); //imprime a tebela de roteamento se foi atualizada

    if(n<0){
      printf("%s: cannot receive data \n",argv[0]);
      continue;}
      
    /* print received message */
    printf("%s: from %s:UDP%u : %s \n", argv[0],inet_ntoa(cliAddr.sin_addr), ntohs(cliAddr.sin_port),msg);
    
  }/* end of server infinite loop */

return 0;

}