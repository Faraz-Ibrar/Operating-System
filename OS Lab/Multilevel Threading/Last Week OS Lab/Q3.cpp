#include <iostream>
#include <vector>
#include <pthread.h>
#include <cmath>

using namespace std;

// Global Variables
vector<vector<double>> W = {{0.2, -0.5, 0.1, 2.0}, {1.5, 1.3, 2.1, 0.0}, {0, 0.25, 0.2, -0.3}};
vector<double> xi = {56, 231, 24, 2};
vector<double> b = {1.1, 3.2, -1.2};
vector<double> Z(3, 0.0);
vector<double> A(3, 0.0);

// Function prototypes
void* matrixMultiplication(void* arg);
void* addBiases(void* arg);
void* applySigmoid(void* arg);

// Structure for thread arguments
struct ThreadArg {
    int row;
};

int main() {
    pthread_t threads[3];
    ThreadArg args[3];

    // Step 1: Matrix Multiplication
    for (int i = 0; i < 3; i++) {
        args[i].row = i;
        pthread_create(&threads[i], nullptr, matrixMultiplication, &args[i]);
    }
    for (int i = 0; i < 3; i++) {
        pthread_join(threads[i], nullptr);
    }

    // Step 2: Add Biases
    for (int i = 0; i < 3; i++) {
        args[i].row = i;
        pthread_create(&threads[i], nullptr, addBiases, &args[i]);
    }
    for (int i = 0; i < 3; i++) {
        pthread_join(threads[i], nullptr);
    }

    // Step 3: Apply Sigmoid
    for (int i = 0; i < 3; i++) {
        args[i].row = i;
        pthread_create(&threads[i], nullptr, applySigmoid, &args[i]);
    }
    for (int i = 0; i < 3; i++) {
        pthread_join(threads[i], nullptr);
    }

    // Output the results
    cout << "Output after sigmoid function (A):" << endl;
    for (const auto& a : A) {
        cout << a << " ";
    }
    cout << endl;

    return 0;
}

// Step 1: Matrix Multiplication
void* matrixMultiplication(void* arg) {
    ThreadArg* tArg = (ThreadArg*)arg;
    int row = tArg->row;
    Z[row] = 0.0;
    for (size_t i = 0; i < W[row].size(); i++) {
        Z[row] += W[row][i] * xi[i];
    }
    pthread_exit(nullptr);
}

// Step 2: Add Biases
void* addBiases(void* arg) {
    ThreadArg* tArg = (ThreadArg*)arg;
    int row = tArg->row;
    Z[row] += b[row];
    pthread_exit(nullptr);
}

// Step 3: Apply Sigmoid
void* applySigmoid(void* arg) {
    ThreadArg* tArg = (ThreadArg*)arg;
    int row = tArg->row;
    A[row] = 1 / (1 + exp(-Z[row]));
    pthread_exit(nullptr);
}
