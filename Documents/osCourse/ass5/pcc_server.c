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


uint16_t pcc_total[95] = { 0 };

uint16_t calc_printable_count(char *buffer) {
    int i, curr_char_value;
    uint16_t printable_count = 0;
    for (i = 0; i < strlen(buffer); i++) {
        curr_char_value = (int)buffer[i];
        if (curr_char_value < 127 && curr_char_value > 31) {
            printable_count++;
        }
    }
    return printable_count;
}

void update_pcc_total(char *buffer) {
    int i, curr_char_value;
    for (i = 0; i < strlen(buffer); i++) {
        curr_char_value = (int)buffer[i];
        if (curr_char_value < 127 && curr_char_value > 31) {
            pcc_total[curr_char_value - 32]++;
        }
    }
}

void print_pcc_toal() {
    int i;
    for (i = 0; i < 95; i++) {
        printf("char '%c' : %hu times\n", (char)(i + 32), pcc_total[i]);
    }
}


int main(int argc, char *argv[]) {
    uint16_t int_net, N, curr_printable_count;
    int server_socket_fd, client_socket_fd, rt = 1;
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
    if (setsockopt(server_socket_fd, SOL_SOCKET, SO_REUSEADDR, &rt, sizeof(int)) < 0) {
        perror("failure");
        exit(1);
    }

    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons((uint16_t)atoi(argv[1]));

    if (bind(server_socket_fd, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
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
        printf("A new client has been connected!\n");
        // Read the number N
        bytes_read = read(client_socket_fd, &int_net, sizeof(int_net));
        if (bytes_read < 0) {
            perror("Failed reading N from the client\n");
            close(client_socket_fd);
            continue;
        }
        N = ntohs(int_net);
        buffer = malloc(N);
        if (buffer == NULL) {
            perror("Failed allocating memory\n");
            close(client_socket_fd);
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
        printf("Client message: %s\n", buffer);
        printf("total - %ld, N - %d", total_read, N);
        if (total_read == N) {
            curr_printable_count = calc_printable_count(buffer);
            printf("# of printable characters: %hu\n", curr_printable_count);
            int_net = htons(curr_printable_count);
            
            if (send(client_socket_fd, &int_net, sizeof(uint16_t), 0) < 0) {
                perror("Failed sending the client");
                continue;
            }
            update_pcc_total(buffer);
        } else {
            perror("Failed reading N from the client\n");
            free(buffer);
            close(client_socket_fd);
        }
        
        free(buffer);
        close(client_socket_fd);
    }
    



}