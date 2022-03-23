#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <assert.h>

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

template <typename T>
struct Slice {
    T *data;
    size_t length;

    Slice(T *v, size_t offset, size_t width) {
        data = v + offset;
        length = width;
    }

    Slice() {
        data = 0;
        length = 0;
    }

    T& operator[](size_t index) {
        return data[index];
    }
};

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

void *solver_thread(void *arg) {
    auto tid = (size_t)arg;
    printf("started thread #%lu\n", tid);
    
    // @TODO: termination when?
    while (true) {
        sem_wait(submit);
        
        // Pull a task from submission queue.
        // @FIXME: Lock the queue
        assert(submission_count > 0);
        Task task = submission_queue[--submission_count];

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

        sem_post(complete);
    }

    return nullptr;
}

void *printer_thread(void *arg) {
    return 0;
}

void init_threads() {
    const char *data[] = {
        "0000000104000000000200000000000504070080003000010900003004002000501000"
        "00000806000",
        "0000000104000000000200000000000506040080003000010900003004002000501000"
        "00000807000",
        "0000000120000350000006000707000003000004008001000000000001200000800000"
        "40050000600",
        "0000000120036000000000070004100200000005003007000006002800000400003005"
        "00000000000",
        "0000000120080300000000000401205000000000047000600000005070003000006200"
        "00000100000",
        "0000000120400500000000090000706004000001000000000000500000875006010003"
        "00200000000",
        "0000000120504000000000000307006004000010000000000800009200008000005107"
        "00000003000",
        "0000000123000000600000400009000005000000010700200000000003504000014008"
        "00060000000"
    };

    auto sudokus = (char **)malloc(8 * sizeof(char *));
    for (int i = 0; i < 8; i++) {
        auto p = (char *)malloc(82);
        sudokus[i] = p;
        strcpy(sudokus[i], data[i]);
    }

    //int cores = get_number_of_cores();
    int cores = 2;
    pthread_t solver_threads[cores];

    submission_queue[0] = {1, Slice<char *>(sudokus, 0, 4)};
    submission_queue[1] = {2, Slice<char *>(sudokus, 4, 4)};
    submission_count = 2;

    // @FIXME: Clean up semaphores
    submit   = sem_open("/sem_submit",   O_CREAT | O_EXCL, 0644, 0);
    assert(submit != SEM_FAILED);
    complete = sem_open("/sem_complete", O_CREAT | O_EXCL, 0644, 0);
    assert(complete != SEM_FAILED);

    for (size_t i = 0; i < cores; i++) {
        int ret = pthread_create(&solver_threads[i], /* TODO: attr? */ NULL,
                                 solver_thread, (void *)i);
        assert(ret == 0);
    }

    sem_post(submit);
    sem_post(submit);

    int total_solved = 0;

    while (true) {
        sem_wait(complete);

        // Pull a task from completion queue.
        assert(completion_count > 0);
        TaskResult res = completion_queue[--completion_count];

        // Print the answers
        for (size_t i = 0; i < res.puzzles.length; i++) {
            char *p = res.puzzles[i];
            print_char_rep(p); printf("\n");
        }

        if (++total_solved == cores) {
            break;
        }

        //sem_post()
    }

    for (size_t i = 0; i < cores; i++) {
        int ret = pthread_cancel(solver_threads[i]);
        assert(ret == 0);
    }

    assert(sem_unlink("/sem_submit") == 0);
    assert(sem_unlink("/sem_complete") == 0);

}
