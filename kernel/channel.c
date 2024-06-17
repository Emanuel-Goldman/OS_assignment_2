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
    struct sleeplock put_lock;
    struct sleeplock take_lock;
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
        initsleeplock(&channel->put_lock, "channel_put");
        initsleeplock(&channel->take_lock, "channel_take");
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
            acquiresleep(&channel->take_lock);
            release(&channel->state_lock);
            return channel->id;
        }
        release(&channel->state_lock);
    }
    return -1;
}

int channel_put(int cd, int data)
{
    // Check if the channel id is valid
    if (cd < 0 || cd >= NPchannel)
    {
        return -1;
    }

    struct channel *channel = &channels[cd];

    acquiresleep(&channel->put_lock); // now we can put the data without worring that another data will be overwritten + we are thw only one who can put data now
    channel->data = data;
    releasesleep(&channel->take_lock);
    return 0;
}

int channel_take(int cd, int *data)
{
    // Check if the channel id is valid
    if (cd < 0 || cd >= NPchannel)
    {
        return -1;
    }

    struct channel *channel = &channels[cd];
    //acquire(&channel->state_lock);
    acquiresleep(&channel->take_lock);
    if()
    //release(&channel->state_lock);
    *data = channel->data;
    releasesleep(&channel->put_lock);
    return 0;
}

int channel_destroy(int cd)
{
    // Check if the channel id is valid
    if (cd < 0 || cd >= NPchannel)
    {
        return -1;
    }

    struct channel *channel = &channels[cd];

    // init
    acquire(&channel->state_lock);
    channel->state = FREE;

    releasesleep(&channel->put_lock); 

    acquiresleep(&channel->take_lock);
    release(&channel->state_lock);

     wakeup(&channels[cd]); // wake up all processes waiting on this channel

    return 0;
}
