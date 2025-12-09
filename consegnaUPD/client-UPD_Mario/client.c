#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#define PORT 8081
#define BUFFER_SIZE 1024

int main() {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        fprintf(stderr, "WSAStartup failed\n");
        return 1;
    }

    SOCKET sockfd;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    int addr_len = sizeof(serv_addr);
    char buffer[BUFFER_SIZE];
    char server_name[BUFFER_SIZE];
    char operation;
    int num1, num2;
    
    // Punto 2: Richiesta nome del server
    printf("Inserisci il nome del server (es. localhost): ");
    if (scanf("%s", server_name) != 1) {
        fprintf(stderr, "Input non valido\n");
        WSACleanup();
        return 1;
    }
    
    // Punto 3: Risoluzione del nome
    server = gethostbyname(server_name);
    if (server == NULL) {
        fprintf(stderr, "Errore: host non trovato\n");
        WSACleanup();
        return 1;
    }
    
    printf("Risoluzione DNS completata\n");
    printf("Indirizzo IP: %s\n", inet_ntoa(*((struct in_addr *)server->h_addr)));
    
    // Creazione socket UDP
    sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd == INVALID_SOCKET) {
        fprintf(stderr, "Errore nella creazione del socket UDP: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    
    // Configurazione indirizzo server
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    serv_addr.sin_port = htons(PORT);
    
    printf("Socket UDP creato\n");
    
    // Invio richiesta di connessione al server
    const char *connect_msg = "CONNECT";
    sendto(sockfd, connect_msg, (int)strlen(connect_msg), 0,
           (struct sockaddr *)&serv_addr, addr_len);
    printf("Richiesta di connessione inviata al server\n");
    
    // Punto 5: Ricezione messaggio di benvenuto
    memset(buffer, 0, BUFFER_SIZE);
    int n = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0,
                    (struct sockaddr *)&serv_addr, &addr_len);
    
    if (n == SOCKET_ERROR) {
        fprintf(stderr, "Errore nella ricezione del messaggio di benvenuto: %d\n", WSAGetLastError());
        closesocket(sockfd);
        WSACleanup();
        return 1;
    }
    
    buffer[n] = '\0';
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
    
    sendto(sockfd, &operation, 1, 0,
           (struct sockaddr *)&serv_addr, addr_len);
    
    // Punto 8: Ricezione risposta dal server
    memset(buffer, 0, BUFFER_SIZE);
    n = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0,
                (struct sockaddr *)&serv_addr, &addr_len);
    
    if (n == SOCKET_ERROR) {
        fprintf(stderr, "Errore nella ricezione della risposta: %d\n", WSAGetLastError());
        closesocket(sockfd);
        WSACleanup();
        return 1;
    }
    
    buffer[n] = '\0';
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
        sendto(sockfd, buffer, (int)strlen(buffer), 0,
               (struct sockaddr *)&serv_addr, addr_len);
        
        // Punto 10: Ricezione e visualizzazione del risultato
        memset(buffer, 0, BUFFER_SIZE);
        n = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0,
                    (struct sockaddr *)&serv_addr, &addr_len);
        
        if (n == SOCKET_ERROR) {
            fprintf(stderr, "Errore nella ricezione del risultato: %d\n", WSAGetLastError());
            closesocket(sockfd);
            WSACleanup();
            return 1;
        }
        
        buffer[n] = '\0';
        printf("\nRisultato dell'operazione: %s\n", buffer);
    } else {
        printf("\nProcesso client terminato.\n");
    }
    
    closesocket(sockfd);
    WSACleanup();
    printf("Socket chiuso\n");
    return 0;
}