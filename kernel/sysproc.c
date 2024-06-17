#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0; // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if (growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  acquire(&tickslock);
  ticks0 = ticks;
  while (ticks - ticks0 < n)
  {
    if (killed(myproc()))
    {
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

uint64
sys_channel_create(void)
{
  return channel_create();
}

uint64
sys_channel_put(void)
{
  int cd;
  int data;
  argint(0, &cd);
  argint(1, &data);
  return channel_put(cd,data);
}


uint64
sys_channel_take(void)
{
    int cd;
    uint64 data_addr;
    int data;

    // Get the arguments from the user
    argint(0, &cd);
    argaddr(1, &data_addr);


    // Call the channel_take function and check for errors
    if (channel_take(cd, &data) < 0) {
        return -1; // Error in taking data from the channel
    }

    // Copy the data from kernel space to user space
    if (copyout(myproc()->pagetable, data_addr, (char *)&data, sizeof(data)) < 0) {
        printf("Error in copying data to user space\n");
        return -1; // Error in copying data to user space
    }

    return 0;
}

uint64
sys_channel_destroy(void)
{
  int n;
  argint(0, &n);
  return channel_destroy(n);
}
