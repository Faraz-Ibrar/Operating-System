#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <cstring>
#include <cstdlib>

const char *FIFO_PATH = "/tmp/nonblock_fifo";

void *readerThread(void *arg) {
    int fd = open(FIFO_PATH, O_RDONLY | O_NONBLOCK);
    if (fd < 0) {
        perror("Error opening FIFO in reader");
        return nullptr;
    }

    char buffer[256];
    while (true) {
        ssize_t bytesRead = read(fd, buffer, sizeof(buffer) - 1);
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            std::cout << "Reader received: " << buffer;
        } else if (bytesRead == 0) {
            // No more data; simulate busy work
            sleep(1);
        } else if (errno != EAGAIN) {
            perror("Read error");
            break;
        }
        // Simulate reader doing busy work
        usleep(500000); // 0.5 seconds
    }

    close(fd);
    return nullptr;
}

int main() {
    // Create the named pipe
    if (mkfifo(FIFO_PATH, 0666) < 0 && errno != EEXIST) {
        perror("Error creating FIFO");
        return EXIT_FAILURE;
    }

    pthread_t readerThreadHandle;

    // Start the reader thread
    if (pthread_create(&readerThreadHandle, nullptr, readerThread, nullptr) != 0) {
        perror("Error creating reader thread");
        return EXIT_FAILURE;
    }

    // Wait for the reader thread to finish
    pthread_join(readerThreadHandle, nullptr);

    return 0;
}