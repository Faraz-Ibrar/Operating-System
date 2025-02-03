#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <algorithm>
#include <climits>
#include <ctime>
#include <cstdlib>
#include <fstream>
#include <random>
#include <atomic>
#include <pthread.h>
#include <filesystem>

using namespace std;
namespace fs = std::filesystem;

// Global variables
vector<vector<int>> graph;
int num_cities;
int num_threads;

// Synchronization variables
atomic<int> global_min_cost(INT_MAX);
atomic<int> winning_thread_id(-1);
mutex global_mutex;
atomic<bool> solution_found(false);
mt19937 gen;

// Method to find the absolute path of the test file
string find_test_file() {
    // List of possible locations to search
    vector<string> search_paths = {
        "testFile.txt"          // Current directory
    };

    for (const auto& path : search_paths) {
        if (fs::exists(path)) {
            return path;
        }
    }

    // If file not found, create a default file
    ofstream default_file("testFile.txt");
    default_file << "4\n";
    default_file << "1 - 2 10\n";
    default_file << "1 - 3 15\n";
    default_file << "1 - 4 20\n";
    default_file << "2 - 3 35\n";
    default_file << "2 - 4 25\n";
    default_file << "3 - 4 3\n";
    default_file.close();

    return "testFile.txt";
}

// Function to update minimum cost
void update_minimum_cost(int thread_id, int cost) {
    lock_guard<mutex> lock(global_mutex);

    // Print current thread's solution
    cout << "Thread " << thread_id << " found tour cost: " << cost << endl;

    // Update minimum if found a better solution
    if (cost < global_min_cost) {
        global_min_cost = cost;
        winning_thread_id = thread_id;
        solution_found = true;
    } 
    // Tiebreaker: In case of equal cost, prefer earlier thread
    else if (cost == global_min_cost && thread_id < winning_thread_id) {
        winning_thread_id = thread_id;
    }
}

// Thread solving function
void* solve_tsp(void* arg) {
    int thread_id = pthread_self();

    // Generate a random permutation
    vector<int> path(num_cities);
    for (int i = 0; i < num_cities; ++i) 
        path[i] = i;

    // Shuffle path
    shuffle(path.begin(), path.end(), gen);

    // Calculate tour cost
    int tour_cost = 0;
    for (int i = 0; i < num_cities - 1; ++i) {
        if (graph[path[i]][path[i+1]] == INT_MAX) {
            tour_cost = INT_MAX;
            break;
        }
        tour_cost += graph[path[i]][path[i+1]];
    }

    // Add cost of returning to start
    if (graph[path.back()][path.front()] != INT_MAX) {
        tour_cost += graph[path.back()][path.front()];
    } else {
        tour_cost = INT_MAX;
    }

    // Check and update minimum cost
    update_minimum_cost(thread_id, tour_cost);

    // If this thread didn't find the minimum, enter infinite loop
    while (!solution_found) {
        this_thread::yield();
    }

    return nullptr;
}

// Main solving function
void run_tsp() {
    vector<pthread_t> threads(num_threads);

    // Create threads
    for (int i = 0; i < num_threads; ++i) {
        pthread_create(&threads[i], nullptr, solve_tsp, nullptr);
    }

    // Wait for all threads
    for (int i = 0; i < num_threads; ++i) {
        pthread_join(threads[i], nullptr);
    }

    // Final result
    if (solution_found) {
        cout << "Minimum tour cost: " << global_min_cost 
             << " found by thread: " << winning_thread_id << endl;
    } else {
        cout << "No valid tour found!" << endl;
    }
}

int main() {
    // Find and open the test file
    string filename = find_test_file();
    ifstream input(filename);
    
    if (!input) {
        cerr << "Absolutely failed to open file: " << filename << endl;
        return 1;
    }

    input >> num_cities;
    graph.resize(num_cities, vector<int>(num_cities, INT_MAX));

    // Read edges
    int u, v, weight;
    char dash;
    while(input >> u >> dash >> v >> weight) {
        graph[u-1][v-1] = weight;
        graph[v-1][u-1] = weight;
    }

    // Seed random number generator
    random_device rd;
    gen.seed(rd());

    // Get number of threads
    cout << "Enter number of threads: ";
    cin >> num_threads;

    try {
        run_tsp();
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }

    return 0;
}