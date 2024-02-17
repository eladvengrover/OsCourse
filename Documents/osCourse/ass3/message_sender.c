#include "message_slot.h"
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>

int main(int argc, char const *argv[])
{
    int channel_id, fd;

    if (argc != 4) {
        perror("Invalid number of args");
        exit(1);
    }

    channel_id = atoi(argv[2]);

    fd = open(argv[1], O_WRONLY);
    if (fd != 0) {
        perror("open file error");
        exit(1);
    }
    if (ioctl(fd, MSG_SLOT_CHANNEL, channel_id) != 0) {
        perror("ioctl error");
        exit(1);
    }
    if (write(fd, argv[3], strlen(argv[3])) != strlen(argv[3])) {
        perror("write error");
        exit(1);
    }
    close(fd);
    exit(0);
}
