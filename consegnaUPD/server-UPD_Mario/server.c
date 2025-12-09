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
    struct sockaddr_in server_addr, client_addr;
    int addr_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];
    char operation;
    int num1, num2, result;
    
    // Creazione socket UDP
    sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd == INVALID_SOCKET) {
        fprintf(stderr, "Errore nella creazione del socket UDP: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    
    // Configurazione indirizzo server
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    
    // Bind
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        fprintf(stderr, "Errore nel bind: %d\n", WSAGetLastError());
        closesocket(sockfd);
        WSACleanup();
        return 1;
    }
    
    printf("Server UDP in ascolto sulla porta %d...\n", PORT);
    
    // Loop infinito per gestire richieste
    while (1) {
        printf("\n=== In attesa di richieste UDP ===\n");
        
        // Ricezione richiesta di connessione dal client
        memset(buffer, 0, BUFFER_SIZE);
        int n = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0, 
                        (struct sockaddr *)&client_addr, &addr_len);
        
        if (n == SOCKET_ERROR) {
            fprintf(stderr, "Errore nella ricezione: %d\n", WSAGetLastError());
            continue;
        }
        
        printf("Richiesta ricevuta da %s:%d\n", 
               inet_ntoa(client_addr.sin_addr), 
               ntohs(client_addr.sin_port));
        
        // Punto 4: Invio messaggio di connessione
        const char *welcome_msg = "connessione avvenuta";
        sendto(sockfd, welcome_msg, (int)strlen(welcome_msg), 0,
               (struct sockaddr *)&client_addr, addr_len);
        printf("Messaggio di benvenuto inviato\n");
        
        // Punto 7: Ricezione lettera operazione
        memset(buffer, 0, BUFFER_SIZE);
        n = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0,
                    (struct sockaddr *)&client_addr, &addr_len);
        
        if (n <= 0) {
            printf("Errore nella ricezione dell'operazione\n");
            continue;
        }
        
        operation = buffer[0];
        printf("Operazione ricevuta: %c\n", operation);
        
        char response[BUFFER_SIZE];
        int valid_operation = 1;
        
        // Determinazione operazione
        switch (operation) {
            case 'A':
            case 'a':
                strcpy(response, "ADDIZIONE");
                break;
            case 'S':
            case 's':
                strcpy(response, "SOTTRAZIONE");
                break;
            case 'M':
            case 'm':
                strcpy(response, "MOLTIPLICAZIONE");
                break;
            case 'D':
            case 'd':
                strcpy(response, "DIVISIONE");
                break;
            default:
                strcpy(response, "TERMINE PROCESSO CLIENT");
                valid_operation = 0;
                break;
        }
        
        // Invio risposta al client
        sendto(sockfd, response, (int)strlen(response), 0,
               (struct sockaddr *)&client_addr, addr_len);
        printf("Inviato al client: %s\n", response);
        
        if (valid_operation) {
            // Punto 9: Ricezione dei due numeri
            memset(buffer, 0, BUFFER_SIZE);
            n = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0,
                        (struct sockaddr *)&client_addr, &addr_len);
            
            if (n > 0) {
                sscanf(buffer, "%d %d", &num1, &num2);
                printf("Numeri ricevuti: %d e %d\n", num1, num2);
                
                // Calcolo risultato
                switch (operation) {
                    case 'A':
                    case 'a':
                        result = num1 + num2;
                        printf("Calcolo: %d + %d = %d\n", num1, num2, result);
                        break;
                    case 'S':
                    case 's':
                        result = num1 - num2;
                        printf("Calcolo: %d - %d = %d\n", num1, num2, result);
                        break;
                    case 'M':
                    case 'm':
                        result = num1 * num2;
                        printf("Calcolo: %d * %d = %d\n", num1, num2, result);
                        break;
                    case 'D':
                    case 'd':
                        if (num2 != 0) {
                            result = num1 / num2;
                            printf("Calcolo: %d / %d = %d\n", num1, num2, result);
                        } else {
                            printf("Errore: divisione per zero\n");
                            result = 0;
                        }
                        break;
                }
                
                // Invio risultato
                sprintf(buffer, "%d", result);
                sendto(sockfd, buffer, (int)strlen(buffer), 0,
                       (struct sockaddr *)&client_addr, addr_len);
                printf("Risultato inviato: %d\n", result);
            }
        }
        
        printf("Richiesta completata\n");
    }
    
    closesocket(sockfd);
    WSACleanup();
    return 0;
}