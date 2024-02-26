#ifndef MESSAGE_SLOT_H
#define MESSAGE_SLOT_H

#include <linux/ioctl.h>

#define MAJOR_NUM 235
#define MSG_SLOT_CHANNEL _IOW(MAJOR_NUM, 0, unsigned long)

#define MAX_CHANNELS_PER_SLOT 0x100000
#define BUF_LEN 128
#define DEVICE_FILE_NAME "message_slot"

typedef struct channel {
    int channel_id;
    char last_message[BUF_LEN];
    int last_message_size;
    struct channel *next;
} channel;

typedef struct message_slot {
    channel *head_channel;
    int size;
    int is_open;
} message_slot;

#endif
