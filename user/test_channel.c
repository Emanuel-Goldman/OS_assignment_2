#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
    //destroying the channel in the middle of the process
    int channel_id = channel_create();

    int pid = fork();
    if(pid == 0){
        printf("child process\n");
        int taken_data;
        printf("the data taken is %d\n", channel_take(channel_id, &taken_data));
    }
    else{
        sleep(20);
        printf("parent process\n");
        channel_destroy(channel_id);
        printf("parent procces had the channel destroyed\n");
    }




    exit(0);
}