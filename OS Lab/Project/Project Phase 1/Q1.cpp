#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>
#include <csignal>
#include <cerrno>

using namespace std;

// Function to split command string into tokens
vector<string> splitCommand(const string& cmd) {
    vector<string> args;
    stringstream ss(cmd);
    string arg;
    while (ss >> arg) {
        args.push_back(arg);
    }
    return args;
}

// Function to convert vector<string> to char* array for execvp
char** convertToCharArray(const vector<string>& args) {
    char** argv = new char*[args.size() + 1]; // +1 for NULL termination
    for (size_t i = 0; i < args.size(); i++) {
        argv[i] = new char[args[i].length() + 1];
        strcpy(argv[i], args[i].c_str());
    }
    argv[args.size()] = nullptr; // Null-terminate the array
    return argv;
}

// Function to free memory allocated for char* array
void cleanupCharArray(char** argv, int size) {
    for (int i = 0; i < size; i++) {
        delete[] argv[i];
    }
    delete[] argv;
}

// Function to split a command string into pipelined components
vector<string> splitPipeline(const string& command) {
    vector<string> pipeline;
    stringstream ss(command);
    string segment;
    while (getline(ss, segment, '|')) {
        pipeline.push_back(segment);
    }
    return pipeline;
}

// Function to handle pipelined execution
void executePipeline(const vector<string>& pipeline) {
    int numCommands = pipeline.size();
    int pipes[numCommands - 1][2]; // Pipes for communication between processes

    // Create necessary pipes
    for (int i = 0; i < numCommands - 1; i++) {
        if (pipe(pipes[i]) == -1) {
            cerr << "Error: Pipe creation failed: " << strerror(errno) << endl;
            return;
        }
    }

    // Loop through each command in the pipeline
    for (int i = 0; i < numCommands; i++) {
        pid_t pid = fork();
        if (pid == -1) {
            cerr << "Error: Fork failed: " << strerror(errno) << endl;
            return;
        }

        if (pid == 0) {
            // Child process
            vector<string> args = splitCommand(pipeline[i]);
            char** argv = convertToCharArray(args);

            // Set up input pipe
            if (i > 0) {
                dup2(pipes[i - 1][0], STDIN_FILENO); // Read from the previous pipe
            }

            // Set up output pipe
            if (i < numCommands - 1) {
                dup2(pipes[i][1], STDOUT_FILENO); // Write to the next pipe
            }

            // Close all pipes
            for (int j = 0; j < numCommands - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            // Execute the command
            if (execvp(argv[0], argv) == -1) {
                cerr << "Command execution failed: " << strerror(errno) << endl;
                cleanupCharArray(argv, args.size());
                exit(1);
            }
        }
    }

    // Close all pipes in the parent process
    for (int i = 0; i < numCommands - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    // Wait for all child processes to complete
    for (int i = 0; i < numCommands; i++) {
        int status;
        wait(&status);
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            cerr << "Command in pipeline exited with status " << WEXITSTATUS(status) << endl;
        }
    }
}

int main() {
    // Handle Ctrl+C gracefully
    signal(SIGINT, [](int) { cout << "\nmyshell 😁> "; cout.flush(); });

    string command;
    const string PROMPT = "myshell 😁> ";

    while (true) {
        // Display prompt
        cout << PROMPT;
        cout.flush();

        // Read command
        getline(cin, command);

        // Skip empty commands
        if (command.empty()) {
            continue;
        }

        // Check for exit command
        if (command == "exit" || command == "quit") {
            cout << "Goodbye!\n";
            break;
        }

        // Check for pipeline
        vector<string> pipeline = splitPipeline(command);
        if (pipeline.size() > 1) {
            // Execute pipelined commands
            executePipeline(pipeline);
        } else {
            // Execute single command
            pid_t pid = fork();
            if (pid == -1) {
                cerr << "Error: Fork failed: " << strerror(errno) << endl;
                continue;
            }

            if (pid == 0) {
                // Child process
                vector<string> args = splitCommand(command);
                char** argv = convertToCharArray(args);

                if (execvp(argv[0], argv) == -1) {
                    cerr << "Command execution failed: " << strerror(errno) << endl;
                    cleanupCharArray(argv, args.size());
                    exit(1);
                }
            } else {
                // Parent process waits for the child to finish
                int status;
                wait(&status);
                if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
                    cerr << "Command exited with status " << WEXITSTATUS(status) << endl;
                }
            }
        }
    }

    return 0;
}
