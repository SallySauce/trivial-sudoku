#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <assert.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <queue>
#include <vector>
#include <algorithm>

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

    Task(size_t ts, Slice<char *> ps) : timestamp(ts), puzzles(ps) {}
};

struct TaskResult {
    size_t        tid;
    size_t        timestamp;
    Slice<char *> puzzles;

    TaskResult(size_t tid, size_t ts, Slice<char *> ps)
        : tid(tid), timestamp(ts), puzzles(ps) {}

    bool operator<(const TaskResult &rhs) const {
        return timestamp > rhs.timestamp;
    }
};

// @FIXME: Lock these.
std::vector<Task>               submission_queue;
std::priority_queue<TaskResult> completion_queue;

sem_t *submit   = nullptr;
sem_t *complete = nullptr;
sem_t *done     = nullptr;
pthread_mutex_t submit_lock;
pthread_mutex_t complete_lock;

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
        pthread_mutex_lock(&submit_lock);
        assert(submission_queue.size() > 0);
        auto task = submission_queue.back();
        submission_queue.pop_back();
        pthread_mutex_unlock(&submit_lock);

        // Work on the puzzles
        for (size_t i = 0; i < task.puzzles.length; i++) {
            char *p = task.puzzles[i];
            dance_solve(p);
        }

        // Notify the scheduler that we're done, and ask it to
        // 1) print the answer,
        // 2) assign new task to me if there's any.
        pthread_mutex_lock(&complete_lock);
        completion_queue.emplace(tid, task.timestamp, task.puzzles);
        pthread_mutex_unlock(&complete_lock);

        sem_post(complete);
    }

    return nullptr;
}

void *printer_thread(void *arg) {
    size_t last_timestamp = 0;
    while (true) {
        sem_wait(complete);

        // Pull a task from completion queue.
        // @FIXME: Lock these queues.
        pthread_mutex_lock(&complete_lock);
        while (completion_queue.size() > 0) {
            auto res = completion_queue.top();

            if (res.timestamp != last_timestamp + 1) break;

            last_timestamp++;
            completion_queue.pop();

            // Print the answers
            for (size_t i = 0; i < res.puzzles.length; i++) {
                char *p = res.puzzles[i];
                print_char_rep(p); printf("\n");
            }

            solved_tasks++;

            sem_post(done);
        }
        pthread_mutex_unlock(&complete_lock);

    }
}

void init_threads() {

    int cores = get_number_of_cores();
    //int cores = 3;
    pthread_t solver_threads[cores];

    // @FIXME: Clean up semaphores
    int ret;
    ret = pthread_mutex_init(&submit_lock, nullptr);
    assert(ret == 0);
    ret = pthread_mutex_init(&complete_lock, nullptr);
    assert(ret == 0);
    submit   = sem_open("/sem_submit",   O_CREAT | O_EXCL, 0644, 0);
    assert(submit != SEM_FAILED);
    complete = sem_open("/sem_complete", O_CREAT | O_EXCL, 0644, 0);
    assert(complete != SEM_FAILED);
    done     = sem_open("/sem_done", O_CREAT | O_EXCL, 0644, 0);
    assert(done != SEM_FAILED);

    for (size_t i = 0; i < cores; i++) {
        ret = pthread_create(&solver_threads[i], /* TODO: attr? */ nullptr,
                                 solver_thread, (void *)i);
        assert(ret == 0);
    }

    pthread_t printer_tid;
    ret = pthread_create(&printer_tid, /* TODO: attr? */ nullptr,
                             printer_thread, nullptr);
    assert(ret == 0);

}

void start_scheduling() {
    char path[4096]; // Hope that TA won't overrun my buffer.
    size_t curr_timestamp = 1;
    while (scanf("%s", path) != EOF) {
        // @FIXME: Don't blow up the memory
        auto test_file = read_entire_file(path);
        auto puzzles = load_puzzles(test_file);

        // If less than a batch, schedule everything to one thread.
        pthread_mutex_lock(&submit_lock);
        constexpr size_t batch = 1024;
        if (puzzles.length < batch) {
            submission_queue.emplace_back(curr_timestamp++, puzzles);
            total_tasks++;
            sem_post(submit);
        } else {
            for (size_t i = 0; i < puzzles.length; i += batch) {
                size_t width = std::min(batch, puzzles.length - i);
                submission_queue.emplace_back(
                    curr_timestamp++, Slice<char *>(puzzles.data, i, width));
                total_tasks++;
            }

            for (size_t i = 0; i < puzzles.length; i += batch) {
                sem_post(submit);
            }
        }
        pthread_mutex_unlock(&submit_lock);

    }

    while (total_tasks != solved_tasks) {
        sem_wait(done);
    }

    assert(sem_unlink("/sem_submit") == 0);
    assert(sem_unlink("/sem_complete") == 0);
    assert(sem_unlink("/sem_done") == 0);
    pthread_mutex_destroy(&submit_lock);
    pthread_mutex_destroy(&complete_lock);
}

void start_threading_version() {
    sem_unlink("/sem_submit");
    sem_unlink("/sem_complete");
    sem_unlink("/sem_done");
    init_threads();
    start_scheduling();
}
