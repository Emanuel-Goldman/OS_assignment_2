#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "spinlock.h"
#include "proc.h"
#include "fs.h"
#include "sleeplock.h"
#include "file.h"

#define channelSIZE 1
#define NPchannel 5
#define FREE 0
#define NFREE 1

struct channel
{
    int data;
    int state;
    int id;
    struct spinlock state_lock;
    struct spinlock data_lock;
    int valid_put;
    int valid_take;
};

struct channel channels[NPchannel];

// initialize the channels
void channelinit(void)
{
    for (int i = 0; i < NPchannel; i++)
    {
        struct channel *channel = &channels[i];
        channel->state = FREE;
        channel->id = i;
        initlock(&channel->state_lock, "channel_state");
        initlock(&channel->data_lock, "channel_data");
        channel->valid_put = 1;
        channel->valid_take = 0;
    }
}

int channel_create(void)
{
    struct channel *channel;
    for (channel = channels; channel < &channels[NPchannel]; channel++)
    {
        acquire(&channel->state_lock);
        if (channel->state == FREE)
        {
            channel->state = NFREE;
            channel->valid_put = 1;
            channel->valid_take = 0;
            release(&channel->state_lock);
            return channel->id;
        }
        release(&channel->state_lock);
    }
    return -1;
}

int channel_put(int cd, int data)
{
    if (cd < 0 || cd >= NPchannel)
    {
        return -1;
    }

    struct channel *channel = &channels[cd];

    acquire(&channel->data_lock);
    while (channel->valid_put == 0)
    {

        sleep(&channel->valid_put, &channel->data_lock); // wakes up when valid_put changes and realse data_lock
        if (channel->state == FREE)                     // desroy has been called
        {
            return -1;
        }
    }

    //  now we have the key and we are allowed to write
    acquire(&channel->state_lock);
    if (&channel->state == FREE) // desroy has been called
    {
        release(&channel->state_lock);
        release(&channel->data_lock);
        return -1;
    }
    else
    {
        channel->data = data;
        channel->valid_take = 1;
        channel->valid_put = 0;
        release(&channel->state_lock);
        release(&channel->data_lock);
        wakeup(&channel->valid_take);
        return 0;
    }
}

int channel_take(int cd, int *data)
{

    if (cd < 0 || cd >= NPchannel)
    {
        return -1;
    }

    struct channel *channel = &channels[cd];
    acquire(&channel->data_lock);
    while (channel->valid_take == 0)
    {
        printf("we went to sleep\n");
        sleep(&channel->valid_take, &channel->data_lock); // wakes up when valid_put changes and realse data_lock
        printf("we woke up\n");
        if (channel->state == FREE)                      // desroy has been called
        {
            printf("the channel is free\n");
            return -1;
        }
    }

    // now we have the key and we are allowed to read
    acquire(&channel->state_lock);
    if (&channel->state == FREE) // desroy has been called
    {
        release(&channel->state_lock);
        release(&channel->data_lock);
        return -1;
    }
    else
    {

        *data = channel->data;
        channel->valid_take = 0;
        channel->valid_put = 1;
        release(&channel->state_lock);
        release(&channel->data_lock);
        wakeup(&channel->valid_put);
        return 0;
    }
}

int channel_destroy(int cd)
{
    printf("we are in the destroy function\n");
    // Check if the channel id is valid
    if (cd < 0 || cd >= NPchannel)
    {
        return -1;
    }

    struct channel *channel = &channels[cd];

    // init
    acquire(&channel->state_lock);

    channel->state = FREE;
    wakeup(&channels->valid_put);
    wakeup(&channels->valid_take);

    release(&channel->state_lock);
    printf("we are out of the destroy function\n");
    return 0;
}
