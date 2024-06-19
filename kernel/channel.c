#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "spinlock.h"
#include "proc.h"
#include "fs.h"
#include "sleeplock.h"
#include "file.h"

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
    int proc;
};

struct channel channels[NPchannel];

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
            channel->proc = myproc()->pid;
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
        if (channel->state == FREE)                      // desroy has been called
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
        sleep(&channel->valid_take, &channel->data_lock); // realse data_lock + wakes up when valid_put changes and take the key
        if (channel->state == FREE)                       // desroy has been called
        {
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
    return 0;
}

int destroy_my_channels(int pid)
{
    for (int i = 0; i < NPchannel; i++)
    {
        struct channel *channel = &channels[i];
        if (pid == channel->proc)
        {
            channel_destroy(pid);
        }
    }
    return 0;
}
