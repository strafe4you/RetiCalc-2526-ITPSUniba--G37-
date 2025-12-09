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
    SOCKET server_fd, client_socket;
    struct sockaddr_in server_addr, client_addr;
    int addr_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];
    char operation;
    int num1, num2, result;
    
    // Inizializzazione Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
        fprintf(stderr, "Errore nell'inizializzazione di Winsock\n");
        exit(EXIT_FAILURE);
    }
    
    // Creazione socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        fprintf(stderr, "Errore nella creazione del socket: %d\n", WSAGetLastError());
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    
    // Configurazione indirizzo server
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    
    // Opzione per riutilizzare subito la porta
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) == SOCKET_ERROR) {
        fprintf(stderr, "Errore setsockopt: %d\n", WSAGetLastError());
        closesocket(server_fd);
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    
    // Bind
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        fprintf(stderr, "Errore nel bind: %d\n", WSAGetLastError());
        closesocket(server_fd);
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    
    // Listen
    if (listen(server_fd, 3) == SOCKET_ERROR) {
        fprintf(stderr, "Errore nel listen: %d\n", WSAGetLastError());
        closesocket(server_fd);
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    
    printf("Server in ascolto sulla porta %d...\n", PORT);
    
    // Loop infinito per gestire più client sequenzialmente
    while (1) {
        printf("\nIn attesa di connessioni...\n");
        
        // Accept
        if ((client_socket = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len)) == INVALID_SOCKET) {
            fprintf(stderr, "Errore nell'accept: %d\n", WSAGetLastError());
            continue;
        }
        
        printf("Client connesso!\n");
        
        // Punto 4: Invio messaggio di connessione
        const char *welcome_msg = "connessione avvenuta";
        send(client_socket, welcome_msg, strlen(welcome_msg), 0);
        
        // Punto 7: Ricezione lettera dal client
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_read = recv(client_socket, buffer, BUFFER_SIZE, 0);
        
        if (bytes_read <= 0) {
            printf("Errore nella ricezione o client disconnesso\n");
            closesocket(client_socket);
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
        send(client_socket, response, strlen(response), 0);
        printf("Inviato al client: %s\n", response);
        
        if (valid_operation) {
            // Punto 9: Ricezione dei due numeri
            memset(buffer, 0, BUFFER_SIZE);
            bytes_read = recv(client_socket, buffer, BUFFER_SIZE, 0);
            
            if (bytes_read > 0) {
                sscanf(buffer, "%d %d", &num1, &num2);
                printf("Numeri ricevuti: %d e %d\n", num1, num2);
                
                // Calcolo risultato
                switch (operation) {
                    case 'A':
                    case 'a':
                        result = num1 + num2;
                        break;
                    case 'S':
                    case 's':
                        result = num1 - num2;
                        break;
                    case 'M':
                    case 'm':
                        result = num1 * num2;
                        break;
                    case 'D':
                    case 'd':
                        if (num2 != 0) {
                            result = num1 / num2;
                        } else {
                            printf("Errore: divisione per zero\n");
                            result = 0;
                        }
                        break;
                }
                
                // Invio risultato
                sprintf(buffer, "%d", result);
                send(client_socket, buffer, strlen(buffer), 0);
                printf("Risultato inviato: %d\n", result);
            }
        }
        
        // Chiusura connessione con il client corrente
        closesocket(client_socket);
        printf("Connessione con il client chiusa\n");
    }
    
    closesocket(server_fd);
    WSACleanup();
    return 0;
}