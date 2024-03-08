#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/stat.h>


int main(int argc, char *argv[]) {
    // uint16_t pcc_total[95] = { 0 };
    uint16_t N_net, N;
    int server_socket_fd, client_socket_fd;
    struct sockaddr_in server_address, client_address;
    socklen_t client_len;
    char *buffer;
    ssize_t bytes_read, total_read;

    if (argc != 2)  {
        perror("Invalid number of args");
        exit(1);
    }

    server_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_fd < 0) {
        perror("Failed opening a socket");
        exit(1);
    }
    // if (setsockopt(server_socket_fd, SOL_SOCKET, SO_REUSEADDR, NULL, sizeof(int)) < 0) {
    //     perror("failure");
    //     exit(1);
    // }

    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons((uint16_t)atoi(argv[1]));

    if (bind(server_socket_fd, (struct sockaddr *) &server_address, sizeof(server_address) < 0)) {
        perror("Failed binding the server");
        exit(1);
    }

    listen(server_socket_fd, 10);

    while (1) {
        client_len = sizeof(client_address);
        client_socket_fd = accept(server_socket_fd, (struct sockaddr *)&client_address, &client_len);
        if (client_socket_fd < 0) {
            perror("Failed accepting a client");
            continue;
        }
        // Read the number N
        bytes_read = read(client_socket_fd, &N_net, sizeof(N_net));
        if (bytes_read < 0) {
            perror("Failed reading N from the client\n");
            continue;
        }
        N = ntohs(N_net);
        buffer = malloc(N);
        if (buffer == NULL) {
            perror("Failed allocating memory\n");
            continue;
        }
        memset(buffer, 0, N);
        total_read = 0;
        while (total_read < N) {
            bytes_read = read(client_socket_fd, buffer + total_read, N - total_read);
            if (bytes_read < 0) {
                perror("Failed reading N from the client\n");
                break;
            }
            total_read += bytes_read;
        }
        if (total_read == N) {
            printf("Counting printable chars");
        }
        free(buffer);
        close(client_socket_fd);
    }
    



}