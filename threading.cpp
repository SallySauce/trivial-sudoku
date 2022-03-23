#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <assert.h>

#include "common.h"

/* Returns non-zero value on failure */
int run_command(const char *cmd, char *buf, int len) {
    FILE *fp;
    char path[1035];

    fp = popen(cmd, "r");
    if (fp == NULL) {
        return -1;
    }

    int bytes_read = (int)fread(buf, 1, len - 1, fp);
    buf[len - 1] = '\0';

    pclose(fp);

    return bytes_read;
}

// @TODO: What happens when atoi failed?
int get_number_of_cores() {
    char buf[24];
#if __APPLE__
    #include <TargetConditionals.h>
    #if TARGET_OS_MAC
    run_command("sysctl -n hw.ncpu", buf, sizeof(buf));
    #else
        #error "Unsupported target platform. Only Linux and MacOS is supported."
    #endif
#elif __linux__
    run_command("grep processor /proc/cpuinfo | wc -l", buf, sizeof(buf));
#else
    #error "Unsupported target platform. Only Linux and MacOS is supported."
#endif
    return atoi(buf);
}


// @TODO: minimize the info passing around threads
struct Task {
    size_t timestamp;       // A strictly increasing integer that indicates the order of job.
    Slice<char *> puzzles;  // The task.
};

struct TaskResult {
    size_t        tid;
    size_t        timestamp;
    Slice<char *> puzzles;
};

// @FIXME: Hope that guolab has no more than 512 cores.
#define MAX_THREADS 512

Task       submission_queue[MAX_THREADS] = {};
TaskResult completion_queue[MAX_THREADS] = {};

// @FIXME: Lock these.
size_t submission_count = 0;
size_t completion_count = 0;

sem_t *submit   = nullptr;
sem_t *complete = nullptr;
sem_t *done     = nullptr;

// @FIXME: Lock thses
size_t total_tasks  = 0;
size_t solved_tasks = 0;

void *solver_thread(void *arg) {
    auto tid = (size_t)arg;
    
    // @TODO: termination when?
    while (true) {
        sem_wait(submit);
        
        // Pull a task from submission queue.
        // @FIXME: Lock the queue
        assert(submission_count > 0);
        Task task = submission_queue[submission_count - 1];

        // Work on the puzzles
        for (size_t i = 0; i < task.puzzles.length; i++) {
            char *p = task.puzzles[i];
            dance_solve(p);
        }

        // Notify the scheduler that we're done, and ask it to
        // 1) print the answer,
        // 2) assign new task to me if there's any.
        completion_queue[completion_count++] = {tid, task.timestamp,
                                                task.puzzles};

        submission_count--;
        solved_tasks++;

        sem_post(complete);
    }

    return nullptr;
}

void *printer_thread(void *arg) {
    while (true) {
        sem_wait(complete);

        // Pull a task from completion queue.
        // @FIXME: Lock these threads.
        assert(completion_count > 0);
        TaskResult res = completion_queue[completion_count - 1];

        // Print the answers
        for (size_t i = 0; i < res.puzzles.length; i++) {
            char *p = res.puzzles[i];
            print_char_rep(p); printf("\n");
        }

        completion_count--;
        sem_post(done);

    }
}

void init_threads() {

    //int cores = get_number_of_cores();
    int cores = 2;
    pthread_t solver_threads[cores];

    // @FIXME: Clean up semaphores
    submit   = sem_open("/sem_submit",   O_CREAT | O_EXCL, 0644, 0);
    assert(submit != SEM_FAILED);
    complete = sem_open("/sem_complete", O_CREAT | O_EXCL, 0644, 0);
    assert(complete != SEM_FAILED);
    done     = sem_open("/sem_done", O_CREAT | O_EXCL, 0644, 0);
    assert(done != SEM_FAILED);

    for (size_t i = 0; i < cores; i++) {
        int ret = pthread_create(&solver_threads[i], /* TODO: attr? */ nullptr,
                                 solver_thread, (void *)i);
        assert(ret == 0);
    }

    pthread_t printer_tid;
    int ret = pthread_create(&printer_tid, /* TODO: attr? */ nullptr,
                             printer_thread, nullptr);
    assert(ret == 0);

    /*
    for (size_t i = 0; i < cores; i++) {
        int ret = pthread_cancel(solver_threads[i]);
        assert(ret == 0);
    }

    */
}

void start_scheduling() {
    char path[4096]; // Hope that TA won't overrun my buffer.
    while (scanf("%s", path) != EOF) {
        // @FIXME: Don't blow up the memory
        auto test_file = read_entire_file(path);
        auto puzzles = load_puzzles(test_file);

        submission_queue[submission_count++] = {1, puzzles};
        total_tasks++;

        sem_post(submit);
    }

    while (total_tasks != solved_tasks) {
        sem_wait(done);
    }

    assert(sem_unlink("/sem_submit") == 0);
    assert(sem_unlink("/sem_complete") == 0);
    assert(sem_unlink("/sem_done") == 0);
}
