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
    printf("channel id is: %d\n", cd);

    if (fork() == 0)
    {
        if (channel_put(cd, 42) < 0)
        {
            printf("Failed to put data in channel\n");
            exit(1);
        }
        if (channel_put(cd, 43) < 0)
        {
            printf("Failed to put data in channel\n");
            exit(1);
        }
        // channel_destroy(cd);
        //  Handle error
    }
    else
    {
        int data;
        if (channel_take(cd, &data) < 0)
        { // 42
            printf("Failed to take data from channel\n");
            exit(1);
        }
        printf("take1:%d\n", data);
        if (channel_take(cd, &data) < 0)
        { // 43
            printf("Failed to take data from channel\n");
            exit(1);
        }
        printf("take2:%d\n", data);
        printf("take3:%d\n", data); // shouldnt be reached
    }

    exit(0);
}