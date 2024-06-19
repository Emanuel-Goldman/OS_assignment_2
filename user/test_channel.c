#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
    // int cd = channel_create();
    // printf("channel id is : %d\n", cd);

    // channel_destroy(cd);

    // cd = channel_create();
    // printf("channel id is : %d\n", cd);
    // אמורים לקבל 0 בשניהם
    //--------------------------------------------

    // int cd = channel_create();
    // printf("channel id is : %d\n", cd);
    // channel_put(cd, 42);

    // channel_destroy(cd);

    // cd = channel_create();
    // printf("channel id is : %d\n", cd);
    // int data;
    // channel_take(cd, &data);
    // printf("data42 : %s\n", data); // אמו7רים להיתקע

    //--------------------------------------------

    // int cd = channel_create();
    // printf("channel id is : %d\n", cd);
    // channel_put(cd, 42);

    // channel_destroy(cd);

    // cd = channel_create();
    // printf("channel id is : %d\n", cd);
    // channel_put(cd, 78);
    // int data;
    // channel_take(cd, &data);
    // printf("data78 : %d\n", data);
    // אמרוים לקבל רק 78 בסוף

    //--------------------------------------------
    // בודקים idים

    // int cd = channel_create();
    // cd = channel_create();
    // cd = channel_create();
    // cd = channel_create();
    // cd = channel_create();

    // cd = channel_create();
    // printf("channel id is (-1): %d\n", cd);

    // channel_destroy(4);

    // cd = channel_create();
    // printf("channel id is (4): %d\n", cd);

    // channel_destroy(2);
    // cd = channel_create();
    // printf("channel id is (2): %d\n", cd);

    //--------------------------------------------

    int cd = channel_create();
    printf("channel id is (0): %d\n", cd);

    if (fork() == 0) // 1
    {
        if (fork() == 0) // 2
        {
            if (fork() == 0) // 3
            {
                if (fork() == 0) // 4
                {
                    if (fork() == 0) // 5
                    {
                        printf("return value: %d\n", channel_put(cd, 42));
                    }
                }
            }
        }
    }

    else
    {
        channel_destroy(cd);
    }

    //--------------------------------------------

    // בודק רק פוט וטייק
    // int cd = channel_create();
    // printf("channel id is : %d\n", cd);

    // if (fork() == 0)
    // {
    //     if (channel_put(cd, 42) < 0)
    //     {
    //         printf("Failed to put data in channel\n");
    //         exit(1);
    //     }
    //     if (channel_put(cd, 78) < 0)
    //     {
    //         printf("Failed to put data in channel\n");
    //         exit(1);
    //     }
    // }

    // else
    // {
    //     int data;
    //     if (channel_take(cd, &data) < 0)
    //     { // 42
    //         printf("Failed to take data from channel\n");
    //         exit(1);
    //     }
    //     printf("data42 : %d\n", data);
    //     if (channel_take(cd, &data) < 0)
    //     { // 43
    //         printf("Failed to take data from channel\n");
    //         exit(1);
    //     }
    //     printf("data78 : %d\n", data);
    // }

    exit(0);
}