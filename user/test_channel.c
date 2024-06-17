#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
    int cd = channel_create();
    if (cd < 0)
    {
        printf("Failed to create channel\n");
        exit(1);
    }
    printf("the channel id is: %d\n", cd);

    if (fork() == 0)
    {
        if (channel_put(cd, 42) < 0)
        {
            printf("Failed to put data in channel\n");
            exit(1);
        }
    }
    else
    {
        int data = 1;
        printf("the data is: %d\n", data);
        channel_take(cd, &data);
        printf("the data is: %d\n", data);
    }

    exit(0);
}