#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define MAX_PRIMES 100
#define DEFAULT_CHECKERS 3

// Channel indices
#define CHANNEL_GEN_TO_CHECK 0
#define CHANNEL_CHECK_TO_PRINT 1

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

    // Fork generator process
    if (fork() == 0)
    {
        generator(num_checkers, gen_to_check);
        exit(0);
    }

    // Fork checker processes
    for (int i = 0; i < num_checkers; i++)
    {
        if (fork() == 0)
        {
            checker(i, gen_to_check, check_to_print);
            exit(0);
        }
    }

    // Fork printer process
    if (fork() == 0)
    {
        printer(check_to_print);
        exit(0);
    }

    // Wait for all child processes to exit
    for (int i = 0; i < num_checkers + 2; i++)
    {
        wait(0);
    }

    // Prompt user to restart the system
    while (1)
    {
        printf("Do you want to restart the system? (y/n): ");
        char buf[2];
        gets(buf, sizeof(buf));
        if (buf[0] == 'y')
        {
            main(argc, argv); // Restart the system
        }
        else
        {
            break;
        }
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
    printf("Generator (PID %d): Channel closed, exiting\n", getpid());
}

void checker(int id, int gen_to_check, int check_to_print)
{
    int num;
    while (1)
    {
        if (channel_take(gen_to_check, &num) < 0)
        {
            break;
        }
        if (is_prime(num))
        {
            if (channel_put(check_to_print, num) < 0)
            {
                break;
            }
        }
    }
    printf("Checker %d (PID %d): Channel closed, exiting\n", id, getpid());
}

void printer(int check_to_print)
{
    int num;
    int count = 0;
    while (count < MAX_PRIMES)
    {
        if (channel_take(check_to_print, &num) < 0)
        {
            break;
        }
        printf("Prime %d: %d\n", count + 1, num);
        count++;
    }
    printf("Printer (PID %d): Found 100 primes, shutting down\n", getpid());

    // Destroy channels to signal other processes to exit
    channel_destroy(check_to_print);
    channel_destroy(CHANNEL_GEN_TO_CHECK);
}

int is_prime(int n)
{
    if (n <= 1)
        return 0;
    for (int i = 2; i * i <= n; i++)
    {
        if (n % i == 0)
            return 0;
    }
    return 1;
}
