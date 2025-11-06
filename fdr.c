#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_TAB_ROT 32
#define IP_SIZE 16

struct tabRot{
    int saltos;
    char destino[IP_SIZE];
    char saida[IP_SIZE];
    clock_t lastUp;};

struct tabRot tabRots[MAX_TAB_ROT];
char atualizado[MAX_TAB_ROT] = {};
char ip[IP_SIZE] = "000.000.000.000";
int size = 0;
extern struct sockaddr_in roteadorAddr, sendAddr;
extern char tab;
extern int sd;

//=======================================================//

void printTab(){ //printar toda vez que atalizar a tabela
    if(tab == 0) return;
    printf("Destino         | Saltos | Saida\n");
    puts("------------------------------------------------");
    for(int i = 0; i < size; i++){
        atualizado[i] = 0;
        if(tabRots[i].saltos == -1) continue;
        printf("%-15s |", tabRots[i].destino);
        printf(" %-6d |", tabRots[i].saltos);
        printf(" %-15s |", tabRots[i].saida);
        printf(" %c |\n", ((atualizado[i] == 1) ? '+' : (atualizado[i] == 2) ? 'R' : ' '));}
    puts("------------------------------------------------");}

//=======================================================//

void tabRotInit(){
    FILE *f = fopen("roteadores.txt", "r");
    int i = 0;
    char c[IP_SIZE];
    while(i < MAX_TAB_ROT){
        if(fscanf(f, "%s", c) != 1) break;
        strcpy(tabRots[i].saida, c);
        strcpy(tabRots[i].destino, c);
        tabRots[i].saltos = 1;
        tabRots[i].lastUp = clock();
        atualizado[i++] = 1;}
    size = i;
    fclose(f);
    i = 0;
    char buff[32] = {};
    snprintf(buff, 32, "@%s", ip);
    while(i < size){
        if(tabRots[i].saltos == 1){
            memcpy((char *) &sendAddr.sin_addr.s_addr, tabRots[i].saida, sizeof(tabRots[i].saida));
            sendto(sd, buff, strlen(buff)+1, 0, (struct sockaddr *) &sendAddr, sizeof(sendAddr));}
        i++;}}

//=======================================================//

void sendTab(){ //enviar de 10 em 10 segundos para todos com conexão direta
    for(int i = 0; i < size; i++){
        if(tabRots[i].saltos != 1) continue;
        char aux[32] = "";
        char buff[1024] = "";
        for(int j = 0; j < size; j++)
            if(strcmp(tabRots[i].saida, tabRots[j].saida)){ //Split Horazion
                snprintf(aux, 32,"*%s;%d", tabRots[j].destino, tabRots[j].saltos);
                strcat(buff, aux);}
        memcpy((char *) &sendAddr.sin_addr.s_addr, tabRots[i].saida, sizeof(tabRots[i].saida));
        sendto(sd, buff, strlen(buff)+1, 0, (struct sockaddr *) &sendAddr, sizeof(sendAddr));
        }}

//=======================================================//

char estaTab(char *ipn){
    for(int i = 0; i < size; i++)
        if(strcmp(ipn, tabRots[i].destino) == 0)
            return i;
    return -1;}

//=======================================================//

int separaString(char *buff, char *ipn, int *saltos){
    char c[8];
    int count = 0;
    int i = 0;

    if(buff[count] == '*') count++;

    while(buff[count] != ';') ipn[i++] = buff[count++];
    ipn[i] = '\0';
    count++;

    i = 0;
    while(buff[count] != '*' && buff[count] != '\0') c[i++] = buff[count++];
    c[i] = '\0';
    *saltos = atoi(c);
    return count;}

//=======================================================//

void recive(char *msg, char *ipMsg){
    char ipn[IP_SIZE];
    if(msg[0] == '@'){ //se for msg de anúncio
        int index = estaTab(ipn);
        if(index >= 0){
            tabRots[index].saltos = 1;
            strcpy(tabRots[index].saida, msg+1);
            tabRots[index].lastUp = clock();
            atualizado[index] = 2;}
        else{
            strcpy(tabRots[size].destino, msg+1);
            strcpy(tabRots[size].saida, msg+1);
            tabRots[size].saltos = 1;
            tabRots[size].lastUp = clock();
            atualizado[size++] = 1;}
        return;}

    int saltos;
    int i = 0;
    while(msg[i] != '\0'){ //enquanto não estiver vazio
        i += separaString(msg+i, ipn, &saltos);
        if(strcmp(ipn, ip) == 0) continue; //se receber rota pra si mesmo
        int index = estaTab(ipn);
        if(index >= 0 && (tabRots[index].saltos > saltos+1 || tabRots[index].saltos == -1)){
            tabRots[index].saltos = saltos+1;
            strcpy(tabRots[index].saida, ipMsg);
            tabRots[index].lastUp = clock();
            atualizado[index] = 2;}
        else{
            strcpy(tabRots[size].destino, ipn);
            strcpy(tabRots[size].saida, ipMsg);
            tabRots[size].saltos = saltos+1;
            tabRots[size].lastUp = clock();
            atualizado[size++] = 1;}}}

//=======================================================//

void removeTab(char *ipn){
    for(int i = 0; i < size; i++)
        if(strcmp(ipn, tabRots[i].saida) == 0){
            tabRots[i].lastUp = -1;
            tabRots[i].saltos = -1;
            atualizado[i] = -1;}}

//=======================================================//

void checkLastUp(){
    for(int i = 0; i < size; i++)
        if(tabRots[i].lastUp != -1 && (clock() - tabRots[i].lastUp)/(CLOCKS_PER_SEC) >= 15){
            if(tabRots[i].saltos == 1)
                removeTab(tabRots[i].saida);
            tabRots[i].lastUp = -1;
            tabRots[i].saltos = -1;
            atualizado[i] = -1;}}

//=======================================================//
            
// int main(){
//     tabRotInit();
//     printTab();
//     recive();
//     sendTab();
//     printTab();
//     removeTab("192.160.0.1");
//     printTab();
// }