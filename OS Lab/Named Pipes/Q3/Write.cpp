#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <cstring>
#include <cstdlib>

const char *FIFO_PATH = "/tmp/nonblock_fifo";

void *writerThread(void *arg) {
    int fd = open(FIFO_PATH, O_WRONLY | O_NONBLOCK);
    if (fd < 0) {
        perror("Error opening FIFO in writer");
        return nullptr;
    }

    int writerId = *(int *)arg;
    for (int i = 0; i < 5; ++i) {
        std::string message = "Writer " + std::to_string(writerId) + ": Message " + std::to_string(i) + "\n";
        ssize_t written = write(fd, message.c_str(), message.size());
        if (written < 0) {
            perror("Write failed");
        } else {
            std::cout << "Writer " << writerId << " wrote: " << message;
        }
        // Simulate busy work
        sleep(1);
    }

    close(fd);
    return nullptr;
}

int main() {
    pthread_t writerThreads[2];
    int writerIds[2] = {1, 2};

    // Start the writer threads
    for (int i = 0; i < 2; ++i) {
        if (pthread_create(&writerThreads[i], nullptr, writerThread, &writerIds[i]) != 0) {
            perror("Error creating writer thread");
            return EXIT_FAILURE;
        }
    }

    // Wait for writer threads to finish
    for (int i = 0; i < 2; ++i) {
        pthread_join(writerThreads[i], nullptr);
    }

    return 0;
}