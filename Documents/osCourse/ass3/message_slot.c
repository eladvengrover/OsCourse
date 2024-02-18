#undef __KERNEL__
#define __KERNEL__
#undef MODULE
#define MODULE


#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/string.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");

#include "message_slot.h"

static message_slot ms[256];

//================== DEVICE FUNCTIONS ===========================
static int device_open( struct inode* inode,
                        struct file*  file )
{
    ms[iminor(inode) - 1].is_open = 1;
    return 0;
}

static int device_release( struct inode* inode,
                           struct file*  file)
{
    ms[iminor(inode) - 1].is_open = 0;
    return 0;
}

static ssize_t device_read( struct file* file,
                            char __user* buffer,
                            size_t       length,
                            loff_t*      offset )
{
    int bytes_read;
    channel *open_channel;
    open_channel = (channel*) file->private_data;

    if (open_channel == NULL || buffer == NULL) {
        return -EINVAL;
    }
    if (open_channel->last_message_size == 0) {
        return -EWOULDBLOCK;
    }
    if (length < open_channel->last_message_size) {
        return -ENOSPC;
    }

    for(bytes_read = 0; bytes_read < open_channel->last_message_size; bytes_read++) {
        if (put_user(open_channel->last_message[bytes_read], &buffer[bytes_read]) != 0) {
            return -EIO;
        }
    }
    return bytes_read;
}

static ssize_t device_write( struct file*       file,
                             const char __user* buffer,
                             size_t             length,
                             loff_t*            offset)
{
    int bytes_write, i;
    channel *open_channel;
    char msg_holder[BUF_LEN];
    open_channel = (channel*) file->private_data;

    if (open_channel == NULL || buffer == NULL) {
        return -EINVAL;
    }
    if (length > BUF_LEN || length <= 0) {
        return -EMSGSIZE;
    }

    for(bytes_write = 0; bytes_write < length; bytes_write++) {
        if (get_user(msg_holder[bytes_write], &buffer[bytes_write]) != 0) {
            return -EIO;
        }
    }

    for(i = 0; i < bytes_write; i++) {
        open_channel->last_message[i] = msg_holder[i];
    }
    open_channel->last_message_size = bytes_write;
    return bytes_write;
}

static long device_ioctl( struct   file* file,
                          unsigned int   ioctl_command_id,
                          unsigned long  ioctl_param )
{
    int minor;
    channel *curr_channel, *prev_channel;
    if (ioctl_command_id != MSG_SLOT_CHANNEL || ioctl_param == 0) {
        return -EINVAL;
    }
    minor = iminor(file->f_inode) - 1;
    if (ms[minor].size == MAX_CHANNELS_PER_SLOT || ms[minor].is_open == 0) {
        return -EINVAL;
    }

    curr_channel = ms[minor].head_channel;
    while (curr_channel != NULL)
    {
        if (curr_channel->channel_id == ioctl_param) {
            break;
        }
        prev_channel = curr_channel;
        curr_channel = curr_channel->next;
    }

    // There are no channels at all / There is no channel with the given channel id
    if (curr_channel == NULL) {
        curr_channel = (channel*) kmalloc(sizeof(channel*), GFP_KERNEL);
        if (curr_channel == NULL) {
            return -ENOMEM;
        }
        if (ms[minor].head_channel == NULL) { // Case 1
            ms[minor].head_channel = curr_channel;
        } else { // Case 2
            prev_channel->next = curr_channel;
        }
        curr_channel->channel_id = ioctl_param;
        curr_channel->last_message_size = 0;
        curr_channel->next = NULL;
        ms[minor].size++;
    }

    file->private_data = curr_channel;
    return 0;
}

struct file_operations Fops = {
  .owner	  = THIS_MODULE, 
  .read           = device_read,
  .write          = device_write,
  .open           = device_open,
  .unlocked_ioctl = device_ioctl,
  .release        = device_release,
};

static int __init init(void)
{
  int rc = -1, i;
  rc = register_chrdev(MAJOR_NUM, DEVICE_FILE_NAME, &Fops );

  if(rc < 0) {
    printk(KERN_ERR "%s registraion failed for %d\n",
                       DEVICE_FILE_NAME, MAJOR_NUM);
    return rc;
  }

  for(i = 0; i< 256; i++) {
    ms[i].head_channel = NULL;
    ms[i].size = 0;
    ms[i].is_open = 0;
  }

  return 0;
}

static void __exit cleanup(void)
{
  int i;
  channel *curr_channel, *next_channel;
  for(i = 0; i < 256; i++) {
    curr_channel = ms[i].head_channel;
    while (curr_channel != NULL) {
        next_channel = curr_channel->next;
        kfree(curr_channel);
        curr_channel = next_channel;
    }
  }
  unregister_chrdev(MAJOR_NUM, DEVICE_FILE_NAME);
}

module_init(init);
module_exit(cleanup);

