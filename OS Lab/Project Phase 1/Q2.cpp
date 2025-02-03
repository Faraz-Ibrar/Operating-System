#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <string>
#include <queue>
#include <map>
#include <ctime>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include <list>

using namespace std;

// Struct for shared memory to manage resources and worker states
struct SharedMemory {
    int bricks;
    int cement;
    int tools;
};

// Struct for worker details
struct Worker {
    string name;
    vector<string> skills;
    int proficiency;
    bool onBreak;
    time_t lastActive; // For LRU
};

// Struct for task details
struct Task {
    string name;
    vector<string> requiredSkills;
    int requiredProficiency;
    vector<string> dependencies;
};

// Maximum resource limits
const int MAX_BRICKS = 10;
const int MAX_CEMENT = 10;
const int MAX_TOOLS = 5;

// Weather conditions
enum WeatherCondition { Sunny, Rainy, Stormy };
WeatherCondition currentWeather;

// Mutex and condition variable for synchronization
pthread_mutex_t resourceMutex;
pthread_cond_t resourceCondition;

// Shared memory instance
SharedMemory* sharedMemory = new SharedMemory{0, 0, 0};

// Priority queues for high, medium, and low priority tasks
priority_queue<pair<string, int>, vector<pair<string, int>>, greater<pair<string, int>>> highPriorityQueue;
priority_queue<pair<string, int>, vector<pair<string, int>>, greater<pair<string, int>>> mediumPriorityQueue;
priority_queue<pair<string, int>, vector<pair<string, int>>, greater<pair<string, int>>> lowPriorityQueue;

// Historical data for resource usage patterns
map<string, int> resourceUsageHistory = {
    {"bricks", 0},
    {"cement", 0},
    {"tools", 0}
};

// Workers and tasks lists
list<Worker> workers;
vector<Task> tasks;

// Error handling mechanism
void handleError(const string& error) {
    cerr << "Error: " << error << endl;
    // Additional recovery steps can be added here
}

// Function to replenish resources periodically
void* replenishResources(void* arg) {
    while (true) {
        pthread_mutex_lock(&resourceMutex);

        // Replenish resources up to their maximum limits
        bool resourcesAdded = false;
        if (sharedMemory->bricks < MAX_BRICKS) {
            sharedMemory->bricks++;
            resourcesAdded = true;
            cout << "Added 1 brick. Current: " << sharedMemory->bricks << " bricks." << endl;
        }
        if (sharedMemory->cement < MAX_CEMENT) {
            sharedMemory->cement++;
            resourcesAdded = true;
            cout << "Added 1 cement. Current: " << sharedMemory->cement << " cement units." << endl;
        }
        if (sharedMemory->tools < MAX_TOOLS) {
            sharedMemory->tools++;
            resourcesAdded = true;
            cout << "Added 1 tool. Current: " << sharedMemory->tools << " tools." << endl;
        }

        // Notify all waiting threads if resources were added
        if (resourcesAdded) {
            pthread_cond_broadcast(&resourceCondition);
        }
        pthread_mutex_unlock(&resourceMutex);

        sleep(2); // Simulate delay in replenishment
    }
    return nullptr;
}

// Function to simulate dynamic resource generation and degradation
void* dynamicResourceManagement(void* arg) {
    while (true) {
        pthread_mutex_lock(&resourceMutex);

        // Simulate resource generation or degradation based on historical usage
        if (resourceUsageHistory["bricks"] > 5) {
            if (sharedMemory->bricks < MAX_BRICKS) {
                sharedMemory->bricks++;
                cout << "Added 1 brick dynamically. Current: " << sharedMemory->bricks << " bricks." << endl;
            }
        } else {
            if (sharedMemory->bricks > 0) {
                sharedMemory->bricks--;
                cout << "Degraded 1 brick dynamically. Current: " << sharedMemory->bricks << " bricks." << endl;
            }
        }

        if (resourceUsageHistory["cement"] > 5) {
            if (sharedMemory->cement < MAX_CEMENT) {
                sharedMemory->cement++;
                cout << "Added 1 cement dynamically. Current: " << sharedMemory->cement << " cement units." << endl;
            }
        } else {
            if (sharedMemory->cement > 0) {
                sharedMemory->cement--;
                cout << "Degraded 1 cement dynamically. Current: " << sharedMemory->cement << " cement units." << endl;
            }
        }

        if (resourceUsageHistory["tools"] > 2) {
            if (sharedMemory->tools < MAX_TOOLS) {
                sharedMemory->tools++;
                cout << "Added 1 tool dynamically. Current: " << sharedMemory->tools << " tools." << endl;
            }
        } else {
            if (sharedMemory->tools > 0) {
                sharedMemory->tools--;
                cout << "Degraded 1 tool dynamically. Current: " << sharedMemory->tools << " tools." << endl;
            }
        }

        // Notify all waiting threads if resources were added
        pthread_cond_broadcast(&resourceCondition);
        pthread_mutex_unlock(&resourceMutex);

        sleep(3); // Simulate delay in dynamic resource management
    }
    return nullptr;
}

// Function to assign priorities to tasks based on task name
int assignPriority(const string& taskName) {
    if (taskName == "Urgent Repair" || taskName == "Foundation Laying" || taskName == "Critical Structural Work") {
        return 1; // High priority
    } else if (taskName == "General Construction" || taskName == "Bricklaying" || taskName == "Cement Mixing") {
        return 2; // Medium priority
    } else {
        return 3; // Low priority
    }
}

// Simulate weather conditions
void* simulateWeather(void* arg) {
    while (true) {
        pthread_mutex_lock(&resourceMutex);

        // Randomly change weather condition
        int randWeather = rand() % 3;
        currentWeather = static_cast<WeatherCondition>(randWeather);

        string weatherStr;
        switch (currentWeather) {
            case Sunny: weatherStr = "Sunny"; break;
            case Rainy: weatherStr = "Rainy"; break;
            case Stormy: weatherStr = "Stormy"; break;
        }
        cout << "Current weather: " << weatherStr << endl;

        // Delay construction activities if weather is Stormy
        if (currentWeather == Stormy) {
            cout << "Stormy weather detected. Construction activities delayed." << endl;
            pthread_mutex_unlock(&resourceMutex);
            sleep(5); // Simulate delay
            continue;
        }

        pthread_mutex_unlock(&resourceMutex);
        sleep(10); // Simulate weather change interval
    }
    return nullptr;
}

// Function to dynamically assign tasks to workers
void assignTasks() {
    for (Task& task : tasks) {
        for (Worker& worker : workers) {
            bool hasRequiredSkills = true;
            for (const string& skill : task.requiredSkills) {
                if (std::find(worker.skills.begin(), worker.skills.end(), skill) == worker.skills.end()) {
                    hasRequiredSkills = false;
                    break;
                }
            }
            if (hasRequiredSkills && worker.proficiency >= task.requiredProficiency) {
                cout << "Assigned task '" << task.name << "' to worker '" << worker.name << "'." << endl;
                break;
            }
        }
    }
}

// Function to simulate worker behavior
void* simulateWorkerBehavior(void* arg) {
    while (true) {
        pthread_mutex_lock(&resourceMutex);

        for (Worker& worker : workers) {
            if (rand() % 10 < 2) { // 20% chance the worker goes on break
                worker.onBreak = true;
                worker.lastActive = time(0);
                cout << worker.name << " is on break." << endl;
            } else if (worker.onBreak && rand() % 10 < 3) { // 30% chance the worker returns from break
                worker.onBreak = false;
                cout << worker.name << " returned from break." << endl;
            }
        }

        pthread_mutex_unlock(&resourceMutex);

        sleep(5); // Simulate behavior change interval
    }
    return nullptr;
}

// Task function for general worker tasks
void* workerTask(void* arg) {
    string workerName = *(string*)arg;
    int priority = assignPriority(workerName);

    pthread_mutex_lock(&resourceMutex);
    if (priority == 1) {
        highPriorityQueue.push({workerName, priority});
    } else if (priority == 2) {
        mediumPriorityQueue.push({workerName, priority});
    } else {
        lowPriorityQueue.push({workerName, priority});
    }

    // Wait until the worker's task is the highest priority and resources are available
    while (true) {
        if (currentWeather == Stormy) {
            pthread_cond_wait(&resourceCondition, &resourceMutex);
        }

        try {
                        if (!highPriorityQueue.empty() && highPriorityQueue.top().first == workerName) {
                if (sharedMemory->bricks >= 2 && sharedMemory->tools >= 1) {
                    // Consume resources
                    sharedMemory->bricks -= 2;
                    sharedMemory->tools--;
                    cout << workerName << " acquired resources for laying bricks. Remaining: "
                         << sharedMemory->bricks << " bricks, " << sharedMemory->tools << " tools." << endl;

                    resourceUsageHistory["bricks"] += 2;
                    resourceUsageHistory["tools"]++;

                    highPriorityQueue.pop();
                    break;
                }
            } else if (highPriorityQueue.empty() && !mediumPriorityQueue.empty() && mediumPriorityQueue.top().first == workerName) {
                if (sharedMemory->bricks >= 2 && sharedMemory->tools >= 1) {
                    // Consume resources
                    sharedMemory->bricks -= 2;
                    sharedMemory->tools--;
                    cout << workerName << " acquired resources for laying bricks. Remaining: "
                         << sharedMemory->bricks << " bricks, " << sharedMemory->tools << " tools." << endl;

                    resourceUsageHistory["bricks"] += 2;
                    resourceUsageHistory["tools"]++;

                    mediumPriorityQueue.pop();
                    break;
                }
            } else if (highPriorityQueue.empty() && mediumPriorityQueue.empty() && !lowPriorityQueue.empty() && lowPriorityQueue.top().first == workerName) {
                if (sharedMemory->bricks >= 2 && sharedMemory->tools >= 1) {
                    // Consume resources
                    sharedMemory->bricks -= 2;
                    sharedMemory->tools--;
                    cout << workerName << " acquired resources for laying bricks. Remaining: "
                         << sharedMemory->bricks << " bricks, " << sharedMemory->tools << " tools." << endl;

                    resourceUsageHistory["bricks"] += 2;
                    resourceUsageHistory["tools"]++;

                    lowPriorityQueue.pop();
                    break;
                }
            }
        } catch (const exception& e) {
            handleError("Resource allocation failed: " + string(e.what()));
        }

        cout << workerName << " is waiting with priority " << priority << "." << endl;
        pthread_cond_wait(&resourceCondition, &resourceMutex);
    }

    pthread_mutex_unlock(&resourceMutex);

    // Simulate work time
    sleep(3);

    cout << workerName << " finished laying bricks." << endl;
    return nullptr;
}

// Function to handle worker breaks using LRU technique
void* handleWorkerBreaks(void* arg) {
    while (true) {
        pthread_mutex_lock(&resourceMutex);

        // Check for workers on break and swap them out using LRU technique
        for (auto it = workers.begin(); it != workers.end();) {
            if (it->onBreak) {
                time_t currentTime = time(0);
                // If worker has been on break for too long, swap out using LRU
                if (difftime(currentTime, it->lastActive) > 10) { // example duration
                    cout << "Swapping out " << it->name << " due to extended break." << endl;
                    it = workers.erase(it);
                    // Add new worker as a replacement (example new worker)
                    workers.push_back({"New Worker", {"Bricklaying", "General Construction"}, 4, false, time(0)});
                } else {
                    ++it;
                }
            } else {
                ++it;
            }
        }

        pthread_mutex_unlock(&resourceMutex);
        sleep(5); // Check interval
    }
    return nullptr;
}

int main() {
    srand(time(0));

    // Initialize synchronization primitives
    pthread_mutex_init(&resourceMutex, nullptr);
    pthread_cond_init(&resourceCondition, nullptr);

    // Create workers
    workers.push_back({"Alice", {"Bricklaying", "Cement Mixing"}, 5, false, time(0)});
    workers.push_back({"Bob", {"Foundation Laying", "Critical Structural Work"}, 7, false, time(0)});
    workers.push_back({"Charlie", {"General Construction", "Non-Critical Task"}, 4, false, time(0)});

    // Create tasks
    tasks.push_back({"Urgent Repair", {"Critical Structural Work"}, 6, {}});
    tasks.push_back({"General Construction", {"Bricklaying", "Cement Mixing"}, 3, {}});
    tasks.push_back({"Foundation Laying", {"Foundation Laying"}, 5, {}});

    // Assign tasks dynamically
    assignTasks();

    // Thread for resource replenishment
    pthread_t replenishmentThread;
    pthread_create(&replenishmentThread, nullptr, replenishResources, nullptr);

    // Thread for dynamic resource management
    pthread_t resourceManagementThread;
    pthread_create(&resourceManagementThread, nullptr, dynamicResourceManagement, nullptr);

    // Thread for weather simulation
    pthread_t weatherThread;
    pthread_create(&weatherThread, nullptr, simulateWeather, nullptr);

    // Thread for simulating worker behavior
    pthread_t workerBehaviorThread;
    pthread_create(&workerBehaviorThread, nullptr, simulateWorkerBehavior, nullptr);

    // Thread for handling worker breaks using LRU
    pthread_t workerBreaksThread;
    pthread_create(&workerBreaksThread, nullptr, handleWorkerBreaks, nullptr);

    // Worker threads for specific tasks
    pthread_t workerThreads[3];
    for (int i = 0; i < 3; i++) {
        pthread_create(&workerThreads[i], nullptr, workerTask, &workers.begin()->name);
    }

    // Wait for worker threads to complete
    for (int i = 0; i < 3; i++) {
        pthread_join(workerThreads[i], nullptr);
    }

    // Clean up threads and synchronization primitives
    pthread_cancel(replenishmentThread);
    pthread_cancel(resourceManagementThread);
    pthread_cancel(weatherThread);
    pthread_cancel(workerBehaviorThread);
    pthread_cancel(workerBreaksThread);
    pthread_join(replenishmentThread, nullptr);
    pthread_join(resourceManagementThread, nullptr);
    pthread_join(weatherThread, nullptr);
    pthread_join(workerBehaviorThread, nullptr);
    pthread_join(workerBreaksThread, nullptr);
    pthread_mutex_destroy(&resourceMutex);
    pthread_cond_destroy(&resourceCondition);

    // Clean up shared memory
    delete sharedMemory;

    return 0;
}
