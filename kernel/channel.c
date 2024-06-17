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
    struct spinlock id_lock;
    struct sleeplock put_lock;
    struct sleeplock take_lock;
};

struct channel channels[NPchannel];

// initialize the channels
void channelinit(void)
{
    // struct channel *channel;
    //  int counter = 0;
    //  for (channel = channels; channel < &channels[NPchannel]; channel++)
    for (int i = 0; i < NPchannel; i++)
    {
        struct channel *channel = &channels[i];
        channel->state = FREE;
        channel->id = i;
        initlock(&channel->id_lock, "channel_id");
        initsleeplock(&channel->put_lock, "channel_put");
        initsleeplock(&channel->take_lock, "channel_take");
        acquiresleep(&channel->take_lock);

        // counter++;
    }
}

int channel_create(void)
{
    int ans = -1;
    struct channel *channel;
    for (channel = channels; channel < &channels[NPchannel]; channel++)
    {
        acquire(&channel->id_lock);
        if (channel->state == FREE)
        {
            channel->state = NFREE;
            ans = channel->id;
        }
        release(&channel->id_lock); // must be outside the if
        if (ans != -1)              // found one
        {
            break;
        }
    }
    return ans;

    // struct proc *p = myproc();
    // for (int i = 0; i < NCHANNEL; i++) {
    //     acquire(&channels[i].lock);
    //     if (channels[i].owner == 0) {
    //         channels[i].owner = p;
    //         release(&channels[i].lock);
    //         return i; // return the channel descriptor
    //     }
    //     release(&channels[i].lock);
    // }
    // return -1; // no available channel
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
    memmove(&channel->data, &data, sizeof(data));
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
    // struct proc *p = myproc();

    acquiresleep(&channel->take_lock);
    // copyout(p->pagetable, data, &channel->data, sizeof(channel->data));
    //  memmove(&data, &channel->data, sizeof(channel->data));
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

    // acquire(&channels[cd].lock);
    // if (channels[cd].owner != myproc()) {
    //     release(&channels[cd].lock);
    //     return -1;
    // }

    // init
    channel->state = FREE;
    channel->id = cd;
    initlock(&channel->id_lock, "channel_id");
    initsleeplock(&channel->put_lock, "channel_put");
    initsleeplock(&channel->take_lock, "channel_take");
    acquiresleep(&channel->take_lock);

    // wakeup(&channels[cd]); // wake up all processes waiting on this channel
    // release(&channels[cd].lock);

    return 0;
}
