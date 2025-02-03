#include <iostream>
#include <unistd.h>   // For fork(), pipe(), and read()/write()
#include <sys/wait.h> // For wait()
#include <pthread.h>
#include <vector>
#include <cmath> // For exp function

using namespace std;

vector<vector<long double>> w = {
    {1.2, 2.2, 3.2},
    {4.2, 5.2, 6.2},
    {7.2, 8.2, 9.2}}; // Weight matrix

vector<vector<long double>> x = {
    {1.2, 4.2},
    {2.2, 5.2},
    {3.2, 6.2}}; // Feature matrix

vector<vector<long double>> b = {
    {1.2, 1.2},
    {1.2, 1.2},
    {1.2, 1.2}};  // Bias matrix

vector<vector<long double>> result(w.size(), vector<long double>(x[0].size(), 0));           // Intermediate result matrix
vector<vector<long double>> z(w.size(), vector<long double>(b[0].size(), 0));                // Z matrix (final result)
vector<vector<long double>> sigmoid_result(w.size(), vector<long double>(b[0].size(), 0.0)); // Sigmoid matrix

int numFeatures = w[0].size(); // Columns in W
int numCols = x[0].size();     // Columns in X
int numRowsB = b.size();       // Rows in B

// Function executed by threads in P1 to compute the result matrix (W * X)
void *multiplyRow(void *arg)
{
    int rowIndex = *(int *)arg;

    for (int col = 0; col < numCols; ++col)
    {
        long double sum = 0;
        for (int feature = 0; feature < numFeatures; ++feature)
        {
            sum += w[rowIndex][feature] * x[feature][col];
        }
        result[rowIndex][col] = sum;
    }

    pthread_exit(nullptr);
}

// Function executed by threads in P2 to compute the Z matrix (result + b)
void *addBiasRow(void *arg)
{
    int rowIndex = *(int *)arg;

    for (int col = 0; col < numCols; ++col)
    {
        z[rowIndex][col] = result[rowIndex][col] + b[rowIndex][col];
    }

    pthread_exit(nullptr);
}

// Function executed by threads in P3 to apply the sigmoid function to the Z matrix
void *applySigmoid(void *arg)
{
    int rowIndex = *(int *)arg;

    for (int col = 0; col < numCols; ++col)
    {
        long double exp_neg_z = exp(-z[rowIndex][col]);
        cout << "exp(-z[" << rowIndex << "][" << col << "]) = " << exp_neg_z << endl;
        sigmoid_result[rowIndex][col] = 1.0 / (1.0 + exp_neg_z);
        }

    pthread_exit(nullptr);
}

int main()
{
    cout << "Parent process started (PID: " << getpid() << ").\n";

    int pipe_fd1[2], pipe_fd2[2];
    if (pipe(pipe_fd1) == -1 || pipe(pipe_fd2) == -1)
    {
        cerr << "Pipe creation failed.\n";
        return 1;
    }

    for (int i = 1; i <= 3; ++i) // Three child processes needed
    {
        pid_t pid = fork(); // Create a new process
        if (pid == 0)
        {
            // Child process
            if (i == 1)
            {
                // Close read end of the pipe in P1
                close(pipe_fd1[0]);
                
                // Step 1: Compute W * X
                int rowsW = w.size();
                pthread_t threads[rowsW];
                int rowIndices[rowsW];

                // Create threads
                for (int i = 0; i < rowsW; ++i)
                {
                    rowIndices[i] = i; // Pass row index to thread
                    pthread_create(&threads[i], nullptr, multiplyRow, (void *)&rowIndices[i]);
                }

                // Wait for all threads to finish
                for (int i = 0; i < rowsW; ++i)
                {
                    pthread_join(threads[i], nullptr);
                }

                // Print the result matrix
                cout << "Resultant Matrix (W * X):" << endl;
                for (const auto &row : result)
                {
                    for (const auto &val : row)
                    {
                        cout << val << " ";
                    }
                    cout << endl;
                }

                // Write result matrix to the pipe
                for (auto &row : result)
                {
                    write(pipe_fd1[1], row.data(), row.size() * sizeof(long double));
                }
                close(pipe_fd1[1]); // Close write end after writing

                _exit(0); // Exit child process P1
            }
            else if (i == 2)
            {
                // Close write end of the pipe in P2
                close(pipe_fd1[1]);
                close(pipe_fd2[0]);

                // Read result matrix from the pipe
                for (auto &row : result)
                {
                    read(pipe_fd1[0], row.data(), row.size() * sizeof(long double));
                }
                close(pipe_fd1[0]); // Close read end after reading

                // Step 2: Compute Z = result + b
                pthread_t threadsP2[numRowsB];
                int rowIndicesP2[numRowsB];

                // Create threads
                for (int i = 0; i < numRowsB; ++i)
                {
                    rowIndicesP2[i] = i; // Pass row index to thread
                    pthread_create(&threadsP2[i], nullptr, addBiasRow, (void *)&rowIndicesP2[i]);
                }

                // Wait for all threads to finish
                for (int i = 0; i < numRowsB; ++i)
                {
                    pthread_join(threadsP2[i], nullptr);
                }

                // Print the Z matrix
                cout << "Z Matrix (result + b):" << endl;
                for (const auto &row : z)
                {
                    for (const auto &val : row)
                    {
                        cout << val << " ";
                    }
                    cout << endl;
                }

                // Write Z matrix to the pipe
                for (auto &row : z)
                {
                    write(pipe_fd2[1], row.data(), row.size() * sizeof(long double));
                }
                close(pipe_fd2[1]); // Close write end after writing

                _exit(0); // Exit child process P2
            }
            else if (i == 3)
            {
                // Close write end of the pipe in P3
                close(pipe_fd2[1]);

                // Read Z matrix from the pipe
                for (auto &row : z)
                {
                    read(pipe_fd2[0], row.data(), row.size() * sizeof(long double));
                }
                close(pipe_fd2[0]); // Close read end after reading

                // Step 3: Apply Sigmoid function
                pthread_t threadsP3[numRowsB];
                int rowIndicesP3[numRowsB];

                // Create threads
                for (int i = 0; i < numRowsB; ++i)
                {
                    rowIndicesP3[i] = i; // Pass row index to thread
                    pthread_create(&threadsP3[i], nullptr, applySigmoid, (void *)&rowIndicesP3[i]);
                }

                // Wait for all threads to finish
                for (int i = 0; i < numRowsB; ++i)
                {
                    pthread_join(threadsP3[i], nullptr);
                }

                // Print the Sigmoid matrix
                cout << "Sigmoid Matrix:" << endl;
                for (const auto &row : sigmoid_result)
                {
                    for (const auto &val : row)
                    {
                        cout << val << " ";
                    }
                    cout << endl;
                }

                _exit(0); // Exit child process P3
            }

            return 0; // Exit child process
        }
        else if (pid > 0)
        {
            // Parent process continues
            cout << "Parent created process P" << i << " with PID: " << pid << ".\n";
            wait(NULL); // Wait for the child process to finish
        }
        else
        {
            // Fork failed
            cerr << "Failed to create process P" << i << ".\n";
            return 1;
        }
    }

    cout << "Parent process completed. All child processes terminated.\n";

    return 0;
}
