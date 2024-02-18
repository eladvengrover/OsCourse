#include "message_slot.h"
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>

int main(int argc, char const *argv[])
{
    int channel_id, fd, bytes_read;
    char buffer[BUF_LEN];

    if (argc != 3) {
        perror("Invalid number of args");
        exit(1);
    }

    channel_id = atoi(argv[2]);

    fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        perror("open file error");
        exit(1);
    }
    if (ioctl(fd, MSG_SLOT_CHANNEL, channel_id) < 0) {
        perror("ioctl error");
        exit(1);
    }
    if ((bytes_read = read(fd, buffer, BUF_LEN)) < 0) {
        perror("read error");
        exit(1);
    }
    close(fd);
    if (write(STDOUT_FILENO, buffer, bytes_read) < bytes_read) {
        perror("write to STD output error");
        exit(1);
    }
    exit(0);
}
