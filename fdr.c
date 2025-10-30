#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TAB_ROT 32
#define IP_SIZE 16
struct tabRot{
    int saltos;
    char destino[IP_SIZE];
    char saida[IP_SIZE];};

struct tabRot tabRots[MAX_TAB_ROT];
char atualizado[MAX_TAB_ROT] = {};
char ip[IP_SIZE] = "000.000.000.000";
int size = 0;

//=======================================================//

void printTab(){ //printar toda vez que atalizar a tabela
    printf("Destino         | Saltos | Saida\n");
    puts("------------------------------------------------");
    for(int i = 0; i < size; i++){
        if(tabRots[i].saltos == -1) continue;
        printf("%-15s |", tabRots[i].destino);
        printf(" %-6d |", tabRots[i].saltos);
        printf(" %-15s |", tabRots[i].saida);
        printf(" %c |\n", ((atualizado[i] == 1) ? '+' : (atualizado[i] == 2) ? 'R' : ' '));
        atualizado[i] = 0;}
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
        atualizado[i++] = 1;}
    size = i;
    fclose(f);
    i = 0;
    char buff[32] = {};
    snprintf(buff, 32, "@%s", ip);
    printf("buff -> %s\n", buff);
    while(i < size){
        if(tabRots[i].saltos == 1){
            //sendTo(buff, tabRots[i].saida);
        }
        i++;}}

//=======================================================//

void send(){ //enviar de 10 em 10 segundos para todos com conexão direta
    for(int i = 0; i < size; i++){
        if(tabRots[i].saltos != 1) continue;
        char aux[32] = "";
        char buff[1024] = "";
        for(int j = 0; j < size; j++)
            if(strcmp(tabRots[i].saida, tabRots[j].saida)){ //Split Horazion
                snprintf(aux, 32,"*%s;%d", tabRots[j].destino, tabRots[j].saltos);
                strcat(buff, aux);}
        //sentTo(buff, tabRots[i].saida);
        }
}

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

    while(buff[count] != ';'){
            ipn[i++] = buff[count];
            count++;}
    ipn[i] = '\0';
    count++;
    i = 0;
    while(buff[count] != '*' && buff[count] != '\0'){
        c[i++] = buff[count];
        count++;}
    c[i] = '\0';
    *saltos = atoi(c);
    return count;}

//=======================================================//

void recive(){
    char buff[1024] = {"@192.160.0.1"}; //{"*666.666.666.666;2*123.123.123.123;12*123.123.123.123;9"};
    char ipn[IP_SIZE];
    char ipMsg[IP_SIZE] = {"111.111.111.111"}; //ip de quem mandou a msg
    int saltos;
    //reciveMsg(buff, ipn);
    if(buff[0] == '@'){ //se for msg de anúncio
        printf("Msg de anúncio com ip %s\n", buff+1);
        int index = estaTab(ipn);
        if(index >= 0){
            tabRots[index].saltos = 1;
            strcpy(tabRots[index].saida, buff+1);
            atualizado[index] = 2;}
        else{
            strcpy(tabRots[size].destino, buff+1);
            strcpy(tabRots[size].saida, buff+1);
            tabRots[size].saltos = 1;
            atualizado[size++] = 1;}
        return;}

    int i = 0;
    while(buff[i] != '\0'){ //enquanto não estiver vazio
        i += separaString(buff+i, ipn, &saltos);
        if(strcmp(ipn, ip) == 0) continue; //se receber rota pra si mesmo
        int index = estaTab(ipn);
        if(index >= 0 && (tabRots[index].saltos > saltos+1 || tabRots[index].saltos == -1)){
            tabRots[index].saltos = saltos+1;
            strcpy(tabRots[index].saida, ipMsg);
            atualizado[index] = 2;}
        else{
            strcpy(tabRots[size].destino, ipn);
            strcpy(tabRots[size].saida, ipMsg);
            tabRots[size].saltos = saltos+1;
            atualizado[size++] = 1;}}}

//=======================================================//

void removeTab(char *ipn){
    for(int i = 0; i < size; i++){
        if(strcmp(ipn, tabRots[i].saida) == 0)
            tabRots[i].saltos = -1;}}

//=======================================================//
            
int main(){
    tabRotInit();
    printTab();
    recive();
    send();
    printTab();
    removeTab("192.160.0.1");
    printTab();
}