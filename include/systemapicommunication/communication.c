#include "../communication.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

static int sock = -1;

void connect(int port) {
    struct sockaddr_in server;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return;
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr("127.0.0.1"); 

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("connect");
        close(sock);
        sock = -1;
        return;
    }

    printf("Connected to server on port %d\n", port);
    
}

void sendcmd(const char* cmd, ...) {
    if (sock == -1) {
        printf("Not connected to server\n");
        return;
    }

    char buffer[4096];

    va_list args;
    va_start(args, cmd);
    vsnprintf(buffer, sizeof(buffer), cmd, args);
    va_end(args);

    send(sock, buffer, strlen(buffer), 0);
}


void ioctl(...) {
    sendcmd("ioctl", ...)
}

bool ping() {
    sendcmd(
        "GET / HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "\r\n"
    );

    char buffer[4096];
    ssize_t bytes = recv(sock, buffer, sizeof(buffer), 0);

    if (bytes <= 0) {
        printf("No response from server\n");
        return 0;
    }

    buffer[bytes] = '\0';

    if (strstr(buffer, "Pong") != NULL) {
        if (output) {
            printf("Server replied with Pong\n");
            return 1;
        }
    } else {
        if (output) {
            printf("Unexpected response:\n%s\n", buffer);
            return 0;
        }
    }
}