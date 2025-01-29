#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 7979
#define BUFFER_SIZE 1024
#define OUTPUT_FILE "alindi.txt"

void receive_file(int client_sock) {
    FILE *file = fopen(OUTPUT_FILE, "ab");
    if (!file) {
        perror("Dosya açılamadı");
        return;
    }

    char buffer[BUFFER_SIZE];
    int bytesReceived;

    while ((bytesReceived = recv(client_sock, buffer, BUFFER_SIZE, 0)) > 0) {
        fwrite(buffer, 1, bytesReceived, file);
        if (buffer[bytesReceived - 1] != '\n') {
            fputc('\n', file);
        }
    }

    printf("Veri başarıyla kaydedildi\n");
    fclose(file);
}

int main() {
    int server_sock, client_sock;
    struct sockaddr_in server, client;
    socklen_t client_len = sizeof(client);

    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket oluşturulamadı");
        exit(1);
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    if (bind(server_sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Bind hatası");
        exit(1);
    }

    if (listen(server_sock, 5) < 0) {
        perror("Listen hatası");
        exit(1);
    }

    printf("Server 7979 portunu dinliyor\n");

    while (1) {
        client_sock = accept(server_sock, (struct sockaddr *)&client, &client_len);
        if (client_sock < 0) {
            perror("Bağlantı kabul hatası");
            continue;
        }

        printf("Bağlantı alındı: %s\n", inet_ntoa(client.sin_addr));
        receive_file(client_sock);
        close(client_sock);
    }

    close(server_sock);
    return 0;
}