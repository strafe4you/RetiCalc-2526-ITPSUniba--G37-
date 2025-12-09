#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    WSADATA wsa_data;
    SOCKET sock = INVALID_SOCKET;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[BUFFER_SIZE] = {0};
    char server_name[BUFFER_SIZE];
    char operation;
    int num1, num2;
    
    // Inizializzazione Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
        fprintf(stderr, "Errore nell'inizializzazione di Winsock\n");
        exit(EXIT_FAILURE);
    }
    
    // Punto 2: Richiesta nome del server
    printf("Inserisci il nome del server (es. localhost): ");
    scanf("%s", server_name);
    
    // Punto 3: Risoluzione del nome
    server = gethostbyname(server_name);
    if (server == NULL) {
        fprintf(stderr, "Errore: host non trovato\n");
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    
    printf("Risoluzione DNS completata\n");
    printf("Indirizzo IP: %s\n", inet_ntoa(*((struct in_addr *)server->h_addr)));
    
    // Creazione socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        fprintf(stderr, "Errore nella creazione del socket: %d\n", WSAGetLastError());
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    
    // Configurazione indirizzo server
    serv_addr.sin_family = AF_INET;
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    serv_addr.sin_port = htons(PORT);
    
    // Connessione al server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR) {
        fprintf(stderr, "Errore nella connessione: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    
    printf("Connessione al server riuscita!\n");
    
    // Punto 5: Ricezione messaggio di benvenuto
    memset(buffer, 0, BUFFER_SIZE);
    recv(sock, buffer, BUFFER_SIZE, 0);
    printf("Messaggio dal server: %s\n", buffer);
    
    // Punto 6: Lettura e invio dell'operazione
    printf("\nInserisci l'operazione desiderata:\n");
    printf("A - Addizione\n");
    printf("S - Sottrazione\n");
    printf("M - Moltiplicazione\n");
    printf("D - Divisione\n");
    printf("Qualsiasi altro carattere - Termina\n");
    printf("Scelta: ");
    scanf(" %c", &operation);
    
    send(sock, &operation, 1, 0);
    
    // Punto 8: Ricezione risposta dal server
    memset(buffer, 0, BUFFER_SIZE);
    recv(sock, buffer, BUFFER_SIZE, 0);
    printf("Risposta dal server: %s\n", buffer);
    
    // Verifica se continuare o terminare
    if (strcmp(buffer, "TERMINE PROCESSO CLIENT") != 0) {
        // Lettura dei due numeri
        printf("\nInserisci il primo numero: ");
        scanf("%d", &num1);
        printf("Inserisci il secondo numero: ");
        scanf("%d", &num2);
        
        // Invio dei numeri al server
        sprintf(buffer, "%d %d", num1, num2);
        send(sock, buffer, strlen(buffer), 0);
        
        // Punto 10: Ricezione e visualizzazione del risultato
        memset(buffer, 0, BUFFER_SIZE);
        recv(sock, buffer, BUFFER_SIZE, 0);
        printf("\nRisultato dell'operazione: %s\n", buffer);
    } else {
        printf("\nProcesso client terminato.\n");
    }
    
    closesocket(sock);
    WSACleanup();
    return 0;
}