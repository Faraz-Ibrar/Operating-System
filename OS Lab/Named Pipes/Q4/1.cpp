#include <iostream>
#include <fstream>
#include <unistd.h>
#include <cstring>
#include <sys/wait.h>

using namespace std;

const char* pipeName = "/tmp/log_pipe";

void writerProcess() {
    // Opening the pipe for writing
    int pipeFd = open(pipeName, O_WRONLY);
    if (pipeFd == -1) {
        cerr << "Error opening pipe for writing" << endl;
        exit(1);
    }

    string logEntry;
    cout << "Writer Process: Enter log entries. Type 'exit' to stop." << endl;

    while (true) {
        cout << "Log: ";
        getline(cin, logEntry);

        if (logEntry == "exit") {
            break;
        }

        write(pipeFd, logEntry.c_str(), logEntry.size());
        write(pipeFd, "\n", 1); 
    }

    close(pipeFd);
    exit(0);
}

void loggerProcess() {
    // Opening the pipe for reading
    int pipeFd = open(pipeName, O_RDONLY);
    if (pipeFd == -1) {
        cerr << "Error opening pipe for reading" << endl;
        exit(1);
    }

    ofstream logFile("log.txt");
    if (!logFile.is_open()) {
        cerr << "Error opening log file" << endl;
        exit(1);
    }

    char buffer[256];
    cout << "Logger Process: Logging started..." << endl;

    while (true) {
        ssize_t bytesRead = read(pipeFd, buffer, sizeof(buffer) - 1);
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            logFile << buffer;
            cout << "Logged: " << buffer;
        } else if (bytesRead == 0) {
            break;
        }
    }

    logFile.close();
    close(pipeFd);
    unlink(pipeName); // Removing the named pipe
    cout << "Logger Process: Logging stopped and pipe removed." << endl;
    exit(0);
}

int main() {
    // Creating the named pipe using a shell command
    system(("mkfifo " + string(pipeName)).c_str());

    pid_t pid = fork();

    if (pid < 0) {
        cerr << "Fork failed" << endl;
        return 1;
    } else if (pid == 0) {
        // Child process acts as the writer
        writerProcess();
    } else {
        // Parent process acts as the logger
        loggerProcess();

        // Waiting for the child process to finish
        wait(nullptr);
    }

    return 0;
}