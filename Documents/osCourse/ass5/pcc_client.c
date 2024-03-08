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
    char *server_ip_address, *file_path;
    uint16_t server_port, file_size_net, C_net;
    int file_fd, socket_fd;
    struct sockaddr_in server_address;
    struct stat st;

    char buffer[1024];
    size_t bytes_read;
    

    if (argc != 4)  {
        perror("Invalid number of args");
        exit(1);
    }
    server_ip_address = argv[1];
    server_port = argv[2];
    file_path = argv[3];

    file_fd = open(file_path, O_RDONLY);

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_fd < 0) {
        perror("Failed opening a socket");
        exit(1);
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(server_port);
    if (inet_pton(AF_INET, server_ip_address, &server_address.sin_addr) <= 0) {
        perror("Invalid server IP address");
        exit(1);
    }

    if (connect(socket_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Failed to connect the server");
        exit(1);
    }

    if (stat(file_path, &st) != 0) {
        perror("Failed to get file size");
        exit(1);
    }

    file_size_net = htons(st.st_size);

    if (send(socket_fd, &file_size_net, sizeof(uint16_t), 0) < 0) {
        perror("Failed sending the server");
        exit(1);
    }

    while((bytes_read = read(file_fd, buffer, sizeof(buffer))) > 0) {
        send(socket_fd, buffer, bytes_read, 0);
    }

    recv(socket_fd, &C_net, sizeof(C_net), 0);

    printf("# of printable characters: %hu\n", ntohs(C_net));

    close(socket_fd);

    return 0;



}