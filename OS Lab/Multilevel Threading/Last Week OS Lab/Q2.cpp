#include <iostream>
#include <vector>
#include <iomanip>
#include <pthread.h>
#include <chrono>

using namespace std;

// Structure to pass arguments to threads
struct ThreadArg {
    const vector<vector<int>>* A;
    const vector<vector<int>>* B;
    vector<vector<int>>* C;
    int row;
    int col;
};

// Function to compute one element of the result matrix
void* multiplyElement(void* arg) {
    ThreadArg* tArg = (ThreadArg*)arg;

    const vector<vector<int>>& A = *(tArg->A);
    const vector<vector<int>>& B = *(tArg->B);
    vector<vector<int>>& C = *(tArg->C);

    int sum = 0;
    for (size_t k = 0; k < A[0].size(); ++k) {
        sum += A[tArg->row][k] * B[k][tArg->col];
    }
    C[tArg->row][tArg->col] = sum;

    pthread_exit(nullptr);
}

// Function to print a matrix
void printMatrix(const vector<vector<int>>& matrix, const string& name) {
    cout << "\nMatrix " << name << ":\n";
    for (const auto& row : matrix) {
        for (int val : row) {
            cout << setw(5) << val;
        }
        cout << "\n";
    }
}

int main() {
    // Hardcoded matrices
    vector<vector<int>> A = {
        {1, 2, 3},
        {4, 5, 6},
        {7, 8, 9},
        {10, 11, 12}
    }; // Matrix A: 4x3

    vector<vector<int>> B = {
        {1, 2, 3, 4},
        {5, 6, 7, 8},
        {9, 10, 11, 12}
    }; // Matrix B: 3x4

    int rowsA = A.size();
    int colsA = A[0].size();
    int rowsB = B.size();
    int colsB = B[0].size();

    // Resulting matrix C will be 4x4
    vector<vector<int>> C(rowsA, vector<int>(colsB, 0));

    // Print input matrices
    printMatrix(A, "A");
    printMatrix(B, "B");

    // Start timing
    auto start = chrono::high_resolution_clock::now();

    // Create threads for matrix multiplication
    pthread_t threads[rowsA * colsB];
    ThreadArg args[rowsA * colsB];
    int threadCount = 0;

    for (int i = 0; i < rowsA; ++i) {
        for (int j = 0; j < colsB; ++j) {
            args[threadCount] = {&A, &B, &C, i, j};
            pthread_create(&threads[threadCount], nullptr, multiplyElement, &args[threadCount]);
            threadCount++;
        }
    }

    // Join all threads
    for (int i = 0; i < threadCount; ++i) {
        pthread_join(threads[i], nullptr);
    }

    // End timing
    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::microseconds>(end - start);

    // Print result matrix
    printMatrix(C, "C (Result)");

    // Print execution time
    cout << "\nExecution time: " << duration.count() << " microseconds\n";

    // Print thread count
    cout << "\nTotal number of threads used: " << threadCount << endl;

    return 0;
}
