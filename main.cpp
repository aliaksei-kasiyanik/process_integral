#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>


double calculate_trapezoid(double step, double a, double b);

double f(double x) {
    return -x * x;
}

int main(int argc, char *argv[]) {

    int A = 0;
    int B = 10;

    int PROCESS_COUNT = atoi(argv[1]);
    int STEP_COUNT = atoi(argv[2]);

    if (PROCESS_COUNT > STEP_COUNT) {
        PROCESS_COUNT = STEP_COUNT;
    }

    double STEP = (B - A) / (double) STEP_COUNT;

    int steps_per_process = STEP_COUNT / PROCESS_COUNT;
    int processes_with_additional_step = STEP_COUNT % PROCESS_COUNT;


    int pipefds[2];
    if (pipe(pipefds) == -1) {
        perror("pipe");
        return 1;
    }

    sem_t mutex;
    if (sem_init(&mutex, 1, 1) < 0) {
        perror("sem_init");
        exit(0);
    }

    pid_t child_pid[PROCESS_COUNT];

    double proc_a;
    double proc_b = A;

    for (int process_i = 0; process_i < PROCESS_COUNT; process_i++) {

        proc_a = proc_b;

        if (process_i == PROCESS_COUNT - 1) {
            proc_b = B;
        } else {
            proc_b = proc_a + STEP * steps_per_process;
            if (processes_with_additional_step != 0) {
                proc_b += STEP;
                processes_with_additional_step--;
            }
        }

        pid_t pid = fork();

        if (pid > 0) { // if parent

            child_pid[process_i] = pid;

        } else if (pid == 0) { // if child

            close(pipefds[0]);

            double sum = calculate_trapezoid(proc_a, proc_b, STEP);
            printf("PART_SUM: %f\n", sum);

            sem_wait(&mutex);
            write(pipefds[1], &sum, sizeof(double));
            sem_post(&mutex);

            return 0;
        } else { // if error

            perror("fork");
            return 1;

        }
    }

    close(pipefds[1]);

    double part_result;
    double result = 0.0;

    for (int i = 0; i < PROCESS_COUNT; i++) {
        read(pipefds[0], &part_result, sizeof(double));
        result += part_result;
    }

    printf("RESULT: %f\n", result);

    int exitCode = 0;
    int status;
    for (int i = 0; i < PROCESS_COUNT; ++i) {
        waitpid(child_pid[i], &status, 0);
        if (WIFEXITED(status)) {
            if (WEXITSTATUS(status) != 0) {
                exitCode = 1;
            }
        }
    }

    sem_close(&mutex);
    return exitCode;
}

double calculate_trapezoid(double a, double b, double step) {
    double result = 0.0;

    double f_a = f(a);

    double f_b;
    double b_i = a + step;

    while (b_i <= b) {
        f_b = f(b_i);
        result += (f_a + f_b);

        f_a = f_b;

        b_i += step;
    }

    result = result * step / 2;

    b_i -= step;
    if (b - b_i > 0) {
        result += (f_a + f(b)) * (b - b_i) / 2;
    }

    return result;
}

