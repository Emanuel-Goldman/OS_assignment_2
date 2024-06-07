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
#define READ 1
#define NREAD 0

struct channel
{
    int data;
    struct sleeplock data_lock;
    int state;
    int id;
    struct spinlock id_lock;
    int read_flage;

    uint nread;    // number of bytes read
    uint nwrite;   // number of bytes written
    int readopen;  // read fd is still open
    int writeopen; // write fd is still open

    char buffer[1024]; // Example buffer size
    int read_pos;      // Read position
    int write_pos;     // Write position
    int closed;        // Status to indicate if the channel is closed
};

struct channel channels[NPchannel];

// initialize the channels
void channelinit(void)
{
    struct channel *channel;
    int counter = 0;
    for (channel = channels; channel < &channels[NPchannel]; channel++)
    {
        channel->state = FREE;
        channel->id = counter;
        initlock(&channel->id_lock, "channel_id");
        initsleeplock(&channel->id_lock, "channel_data");
        channel->read_flage = 0;

        counter++;
    }
}

int channel_create(void)
{
    struct channel *channel;
    for (channel = channels; channel < &channels[NPchannel]; channel++)
    {
        acquire(&channel->id_lock);
        if (channel->state == FREE)
        {
            channel->state = NFREE;
            release(&channel->id_lock);
            return channel->id;
        }
    }
    return -1;
}

int channel_put(int cd, int data)
{
    struct channel *channel;

    // Check if the channel id is valid
    if (cd < 0 || cd >= NPchannel)
    {
        return -1; // Invalid channel id
    }

    channel = &channels[cd];
    acquiresleep(&channel->data_lock);
    channel->data = data;
    channel->read_flage = 0;
    sleep(channel->read_flage, );
    releasesleep(&channel->data_lock);
    return 0;
}

// int channelalloc(struct file **f0, struct file **f1)
// {
//     struct channel *pi;

//     pi = 0;
//     *f0 = *f1 = 0;
//     if ((*f0 = filealloc()) == 0 || (*f1 = filealloc()) == 0)
//         goto bad;
//     if ((pi = (struct channel *)kalloc()) == 0)
//         goto bad;
//     pi->readopen = 1;
//     pi->writeopen = 1;
//     pi->nwrite = 0;
//     pi->nread = 0;
//     initlock(&pi->lock, "channel");
//     (*f0)->type = FD_channel;
//     (*f0)->readable = 1;
//     (*f0)->writable = 0;
//     (*f0)->channel = pi;
//     (*f1)->type = FD_channel;
//     (*f1)->readable = 0;
//     (*f1)->writable = 1;
//     (*f1)->channel = pi;
//     return 0;

// bad:
//     if (pi)
//         kfree((char *)pi);
//     if (*f0)
//         fileclose(*f0);
//     if (*f1)
//         fileclose(*f1);
//     return -1;
// }

void channelclose(struct channel *pi, int writable)
{
    acquire(&pi->lock);
    if (writable)
    {
        pi->writeopen = 0;
        wakeup(&pi->nread);
    }
    else
    {
        pi->readopen = 0;
        wakeup(&pi->nwrite);
    }
    if (pi->readopen == 0 && pi->writeopen == 0)
    {
        release(&pi->lock);
        kfree((char *)pi);
    }
    else
        release(&pi->lock);
}

int channelwrite(struct channel *pi, uint64 addr, int n)
{
    int i = 0;
    struct proc *pr = myproc();

    acquire(&pi->lock);
    while (i < n)
    {
        if (pi->readopen == 0 || killed(pr))
        {
            release(&pi->lock);
            return -1;
        }
        if (pi->nwrite == pi->nread + channelSIZE)
        { // DOC: channelwrite-full
            wakeup(&pi->nread);
            sleep(&pi->nwrite, &pi->lock);
        }
        else
        {
            char ch;
            if (copyin(pr->pagetable, &ch, addr + i, 1) == -1)
                break;
            pi->data[pi->nwrite++ % channelSIZE] = ch;
            i++;
        }
    }
    wakeup(&pi->nread);
    release(&pi->lock);

    return i;
}

int channelread(struct channel *pi, uint64 addr, int n)
{
    int i;
    struct proc *pr = myproc();
    char ch;

    acquire(&pi->lock);
    while (pi->nread == pi->nwrite && pi->writeopen)
    { // DOC: channel-empty
        if (killed(pr))
        {
            release(&pi->lock);
            return -1;
        }
        sleep(&pi->nread, &pi->lock); // DOC: channelread-sleep
    }
    for (i = 0; i < n; i++)
    { // DOC: channelread-copy
        if (pi->nread == pi->nwrite)
            break;
        ch = pi->data[pi->nread++ % channelSIZE];
        if (copyout(pr->pagetable, addr + i, &ch, 1) == -1)
            break;
    }
    wakeup(&pi->nwrite); // DOC: channelread-wakeup
    release(&pi->lock);
    return i;
}
