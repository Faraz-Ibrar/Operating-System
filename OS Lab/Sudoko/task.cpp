#include <iostream>
#include <pthread.h>
#include <vector>
#include <set>

using namespace std;

// Size of the Sudoku
const int SIZE = 9;

// Shared Sudoku grid
int sudoku[SIZE][SIZE];

// Result variables
bool rowsValid = true;
bool colsValid = true;

// Function to validate rows
void* validateRows(void* arg) {
    for (int i = 0; i < SIZE; i++) {
        set<int> seen;
        for (int j = 0; j < SIZE; j++) {
            int val = sudoku[i][j];
            if (val < 1 || val > 9 || seen.count(val)) {
                rowsValid = false;
                return nullptr;
            }
            seen.insert(val);
        }
    }
    return nullptr;
}

// Function to validate columns
void* validateColumns(void* arg) {
    for (int j = 0; j < SIZE; j++) {
        set<int> seen;
        for (int i = 0; i < SIZE; i++) {
            int val = sudoku[i][j];
            if (val < 1 || val > 9 || seen.count(val)) {
                colsValid = false;
                return nullptr;
            }
            seen.insert(val);
        }
    }
    return nullptr;
}

int main() {
    // Example Sudoku (replace with input if needed)
    int example[SIZE][SIZE] = {
        {5, 3, 4, 6, 7, 8, 9, 1, 2},
        {6, 7, 2, 1, 9, 5, 3, 4, 8},
        {1, 9, 8, 3, 4, 2, 5, 6, 7},
        {8, 5, 9, 7, 6, 1, 4, 2, 3},
        {4, 2, 6, 8, 5, 3, 7, 9, 1},
        {7, 1, 3, 9, 2, 4, 8, 5, 6},
        {9, 6, 1, 5, 3, 7, 2, 8, 4},
        {2, 8, 7, 4, 1, 9, 6, 3, 5},
        {3, 4, 5, 2, 8, 6, 1, 7, 9}
    };

    // Copy example to shared Sudoku grid
    for (int i = 0; i < SIZE; i++)
        for (int j = 0; j < SIZE; j++)
            sudoku[i][j] = example[i][j];

    // Create threads
    pthread_t rowThread, colThread;

    pthread_create(&rowThread, nullptr, validateRows, nullptr);
    pthread_create(&colThread, nullptr, validateColumns, nullptr);

    // Wait for threads to finish
    pthread_join(rowThread, nullptr);
    pthread_join(colThread, nullptr);

    // Check results
    if (rowsValid && colsValid) {
        cout << "The Sudoku is valid." << endl;
    } else {
        cout << "The Sudoku is invalid." << endl;
    }

    return 0;
}
