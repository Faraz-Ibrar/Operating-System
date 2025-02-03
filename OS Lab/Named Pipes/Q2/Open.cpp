#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <iostream>
#include <sys/stat.h>

using namespace std;

int main() {
    char str[256];  // Use char array for consistency
    int fifo_write;
    int f1 = mkfifo("pipe_one", 0666);
    if (f1 < 0) {
        cout << "Error while creating pipe " << endl;
    }

    fifo_write = open("pipe_one", O_WRONLY);
    if (fifo_write < 0) {
        cout << "Error opening file " << endl;
    } else {
        while (true) {
            cout << "Enter text: " << endl;
            cin.getline(str, sizeof(str)); // Use getline to handle spaces
            write(fifo_write, str, sizeof(str));
            if (strcmp(str, "abort") == 0) {
                break; // Exit the loop when "abort" is entered
            }
        }
        close(fifo_write);
    }
    return 0;
}
