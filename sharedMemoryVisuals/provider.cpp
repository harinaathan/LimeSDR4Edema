#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <semaphore.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#define SHM_NAME "/live_data"
#define SEM_NAME "/semCtrl"
#define QUEUE_SIZE 1024
#define TEXT_SIZE 64

// Structure for each queue entry
struct QueueEntry {
    int64_t timestamp;
    char text[TEXT_SIZE];
};

// Structure for the FIFO queue
struct SharedQueue {
    size_t head;
    size_t tail;
    size_t count;
    QueueEntry entries[QUEUE_SIZE];
};

int error(const char *msg) {
    std::cerr << msg << ": " << strerror(errno) << std::endl;
    return -1;
}

int main() {

    // open shared memory
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) return error("shm_open failed");

    // set size of shared memory
    if (ftruncate(shm_fd, sizeof(SharedQueue)) == -1) error("ftruncate failed");

    // Map shared memory
    SharedQueue *queue = (SharedQueue *)mmap(nullptr, sizeof(SharedQueue), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (queue == MAP_FAILED) return error("mmap failed");

    // Create a named semaphore
    sem_t *sem = sem_open(SEM_NAME, O_CREAT, 0666, 1);
    if (sem == SEM_FAILED) error("sem_open failed");

    // initialize shared queue
    queue->head = 0;
    queue->tail = 0;
    queue->count = 0;

    // produce some data
    int i = 0;
    while (true) {
        // wait for semaphore
        if (sem_wait(sem) == -1) return error("sem_wait failed");
        // check if queue is full
        if (queue->count == QUEUE_SIZE) {
            std::cerr << "Queue is full, waiting for consumer..." << std::endl;
            sem_post(sem); // release semaphore lock
            usleep(100000); // Sleep for 100 milliseconds
            continue;
        }

        // produce data
        QueueEntry &entry = queue->entries[queue->tail];
        entry.timestamp = time(nullptr);
        snprintf(entry.text, TEXT_SIZE, "Data id : %06d", i++);
        // increment tail index
        queue->tail = (queue->tail + 1) % QUEUE_SIZE;
        queue->count++;
        std::cout << "Produced: " << entry.text << " at " << entry.timestamp << std::endl;
        // signal that data is available
        sem_post(sem); // release semaphore lock
        // sleep for a while to simulate data production
        usleep(100000); // Sleep for 100 milliseconds
    }

    //Cleanup
    munmap(queue, sizeof(SharedQueue));
    shm_unlink(SHM_NAME);
    sem_close(sem);
    sem_unlink(SEM_NAME);
    close(shm_fd);
    return 0;
}