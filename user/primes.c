#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define MAX_PRIMES 100
#define DEFAULT_CHECKERS 3

void generator(int checkers, int gen_to_check);
void checker(int id, int gen_to_check, int check_to_print);
void printer(int check_to_print);
int is_prime(int n);

int main(int argc, char *argv[])
{
    int num_checkers = DEFAULT_CHECKERS;
    if (argc > 1)
    {
        num_checkers = atoi(argv[1]);
    }

    // Create channels
    int gen_to_check = channel_create();
    int check_to_print = channel_create();

    if (gen_to_check < 0 || check_to_print < 0)
    {
        printf("Failed to create channels\n");
        exit(1);
    }

    // checker processes
    for (int i = 0; i < num_checkers; i++)
    {
        if (fork() == 0)
        {
            checker(i, gen_to_check, check_to_print);
            exit(0);
        }
    }

    // printer process
    if (fork() == 0)
    {
        printer(check_to_print);
        exit(0);
    }
    else // generator process - the father
    {
        generator(num_checkers, gen_to_check);
    }

    // Wait for all child processes to exit
    for (int i = 0; i < num_checkers + 1; i++)
    {
        wait(0);
    }

    // Prompt user to restart the system
    printf("Do you want to start the system again? (y/n): ");
    char buf[2];
    gets(buf, sizeof(buf));
    if (buf[0] == 'y')
    {
        main(argc, argv); // Restart the system
    }

    exit(0);
}

void generator(int checkers, int gen_to_check)
{
    int num = 2;
    while (1)
    {
        if (channel_put(gen_to_check, num) < 0)
        {
            break;
        }
        num++;
    }
    printf("Channel 1 closed, Generator exiting\n");
}

void checker(int i, int gen_to_check, int check_to_print)
{
    int num;
    while (1)
    {
        if (channel_take(gen_to_check, &num) < 0)
        {
            printf("failed to take a new number to check\n");
            exit(1);
        }
        if (is_prime(num))
        {
            if (channel_put(check_to_print, num) < 0)
            {
                break;
            }
        }
    }
    printf("Channel 2 closed, Checker #%d exiting\n", i);
    channel_destroy(gen_to_check);
}

void printer(int check_to_print)
{
    int num;
    int count = 0;
    while (count < MAX_PRIMES)
    {
        if (channel_take(check_to_print, &num) < 0)
        {
            printf("failed to take a new number to print\n");
            exit(1);
        }
        count++;
        printf("Prime %d: %d\n", count, num);
    }
    printf("found 100 primes, shutting down...\n");

    // Destroy channels to signal other processes to exit
    channel_destroy(check_to_print);
}

int is_prime(int n)
{
    for (int i = 2; i * i <= n; i++)
    {
        if (n % i == 0)
            return 0;
    }
    return 1;
}
