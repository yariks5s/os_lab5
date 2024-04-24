#include <cstring>
#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <csignal>
#include <ctime>

int f(int x)
{
    if (x == 512)
        while (1)
            sleep(1);
    else if (x % 2 == 0)
        return 0;
    else
        return 1;
}

int g(int x)
{
    if (x == 512)
        while (1)
            sleep(1);
    else if (x > 10)
        return x;
    else
        return 0;
}

void handle_timeout(int signum)
{
    std::cout << "Timeout occurred. No response from child processes.\n";
    std::cout << "You might want to check if the processes are stuck in an infinite loop.\n";
    std::cout << "Exiting...\n";
    exit(1);
}

int main() {
    int x;
    std::cout << "Enter a value for x: ";
    std::cin >> x;

    int fd_f[2], fd_g[2];
    pipe(fd_f);
    pipe(fd_g);

    pid_t pid_f, pid_g;
    int status_f, status_g;

    signal(SIGALRM, handle_timeout);
    alarm(3); // timeout alarm - 3 сек

    if ((pid_f = fork()) == 0)
    {
        close(fd_f[0]); // Close read end in child
        dup2(fd_f[1], STDOUT_FILENO); // Redirect stdout to write end of pipe
        int result_f = f(x);
        write(fd_f[1], &result_f, sizeof(result_f));
        close(fd_f[1]); // Close the write end
        exit(0);
    }

    if ((pid_g = fork()) == 0)
    {
        close(fd_g[0]); // Close read end in child
        dup2(fd_g[1], STDOUT_FILENO); // Redirect stdout to write end of pipe
        int result_g = g(x);
        write(fd_g[1], &result_g, sizeof(result_g));
        close(fd_g[1]); // Close the write end
        exit(0);
    }

    // Close write ends in the parent
    close(fd_f[1]);
    close(fd_g[1]);

    int result_f = 0, result_g = 0;
    if (read(fd_f[0], &result_f, sizeof(result_f)) < 0)
        std::cerr << "Error reading from pipe for f(x): " << strerror(errno) << std::endl;
    if (read(fd_g[0], &result_g, sizeof(result_g)) < 0)
        std::cerr << "Error reading from pipe for g(x): " << strerror(errno) << std::endl;

    // Wait for child processes to finish
    waitpid(pid_f, &status_f, 0);
    waitpid(pid_g, &status_g, 0);

    int final_result = result_f || result_g;
    std::cout << "Result of f(x) || g(x): " << final_result << ((final_result == 1) ? " (true)." : " (false).") << std::endl;

    close(fd_f[0]);
    close(fd_g[0]);

    return 0;
}