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
#define NPchan 5

struct channel
{
    char data[channelSIZE];

    struct spinlock lock;
    uint nread;    // number of bytes read
    uint nwrite;   // number of bytes written
    int readopen;  // read fd is still open
    int writeopen; // write fd is still open

    char buffer[1024];     // Example buffer size
    int read_pos;          // Read position
    int write_pos;         // Write position
    struct spinlock lock2; // Lock for synchronizing access
    int closed;            // Status to indicate if the channel is closed
};

struct channel channels[NPchan];

// initialize the channels
void channelinit(void)
{
    struct channel *chan;

    for (chan = channels; chan < &channels[NPchan]; chan++)
    {
        // chan->state =
        // chan->kstack =
    }
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
