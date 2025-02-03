#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <iostream>
using namespace std;

int main() {
    char str[256];  // Ensure enough space for the message
    int fifo_read = open("pipe_one", O_RDONLY);
    if (fifo_read < 0) {
        cout << "Error while opening file " << endl;
    } else {
        while (true) {
            memset(str, 0, sizeof(str)); // Clear buffer before reading
            read(fifo_read, str, sizeof(str));
            cout << "Text: " << str << endl;
            if (strcmp(str, "abort") == 0) {
                break; // Exit the loop when "abort" is received
            }
        }
        close(fifo_read);
    }
    return 0;
}
