#include <unistd.h>
#include <iostream>
#include <sys/wait.h>
#include <csignal>
#include <vector>

int f(int x) {
    // Тут можна додати код функції f.
    return x % 2; // Приклад: повертає 1, якщо x непарне, інакше 0.
}

int g(int x) {
    // Тут можна додати код функції g.
    return (x > 5) ? 1 : 0; // Приклад: повертає 1, якщо x більше 5, інакше 0.
}

void execute_function(int (*func)(int), int x) {
    std::cout << func(x) << std::endl;
    exit(0); // Завершуємо процес після виводу.
}

int main() {
    int x;
    std::cout << "Введіть значення x: ";
    std::cin >> x;

    int fd_f[2], fd_g[2];
    pipe(fd_f);
    pipe(fd_g);

    if (fork() == 0) {
        dup2(fd_f[1], STDOUT_FILENO);
        close(fd_f[0]);
        close(fd_f[1]);
        execute_function(f, x);
    }
    close(fd_f[1]);

    if (fork() == 0) {
        dup2(fd_g[1], STDOUT_FILENO);
        close(fd_g[0]);
        close(fd_g[1]);
        execute_function(g, x);
    }
    close(fd_g[1]);

    int status, result_f = 0, result_g = 0;
    read(fd_f[0], &result_f, sizeof(result_f));
    read(fd_g[0], &result_g, sizeof(result_g));

    // Логіка Кліні для "||"
    int result = result_f || result_g;
    std::cout << "Результат f(x) || g(x): " << result << std::endl;

    wait(&status);
    wait(&status);
    return 0;
}
