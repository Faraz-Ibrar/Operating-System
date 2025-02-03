#include <iostream>
#include <queue>
#include <vector>
#include <algorithm>
#include <chrono>
#include <thread>
#include <iomanip>

using namespace std;

// Structure to represent a process
struct Process {
    int id;
    int priority;
    int burstTime;
    int arrivalTime;
    int remainingTime;
    int waitingTime;
    int completionTime;
    
    Process(int i, int p, int bt, int at) : 
        id(i), priority(p), burstTime(bt), arrivalTime(at), 
        remainingTime(bt), waitingTime(0), completionTime(0) {}
};

// Compare function for priority scheduling
struct ComparePriority {
    bool operator()(const Process& p1, const Process& p2) {
        return p1.priority < p2.priority;  // Higher priority first
    }
};

// Compare function for SRTF
struct CompareSRTF {
    bool operator()(const Process& p1, const Process& p2) {
        return p1.remainingTime > p2.remainingTime;  // Shorter remaining time first
    }
};

class Scheduler {
private:
    vector<Process> processes;
    int currentTime;

    void simulateExecution(Process& p, int timeQuantum) {
        cout << "Executing Process " << p.id << " for " << timeQuantum << " units\n";
        this_thread::sleep_for(chrono::milliseconds(timeQuantum * 100));  // Simulate execution
        p.remainingTime -= timeQuantum;
    }

public:
    Scheduler() : currentTime(0) {}

    void inputProcesses() {
        int n;
        cout << "Enter number of processes: ";
        cin >> n;

        for (int i = 0; i < n; i++) {
            int priority, burstTime, arrivalTime;
            cout << "\nProcess " << i + 1 << ":\n";
            cout << "Priority (1-10): ";
            cin >> priority;
            cout << "Burst Time: ";
            cin >> burstTime;
            cout << "Arrival Time: ";
            cin >> arrivalTime;

            processes.emplace_back(i + 1, priority, burstTime, arrivalTime);
        }
    }

    double priorityScheduling() {
        vector<Process> tempProcesses = processes;
        priority_queue<Process, vector<Process>, ComparePriority> pq;
        currentTime = 0;
        double totalWaitingTime = 0;

        while (!tempProcesses.empty() || !pq.empty()) {
            // Add newly arrived processes to queue
            while (!tempProcesses.empty() && tempProcesses.front().arrivalTime <= currentTime) {
                pq.push(tempProcesses.front());
                tempProcesses.erase(tempProcesses.begin());
            }

            if (!pq.empty()) {
                Process current = pq.top();
                pq.pop();
                
                current.waitingTime = currentTime - current.arrivalTime;
                totalWaitingTime += current.waitingTime;
                
                simulateExecution(current, current.burstTime);
                currentTime += current.burstTime;
            } else {
                currentTime++;
            }
        }

        return totalWaitingTime / processes.size();
    }

    double srtfScheduling() {
        vector<Process> tempProcesses = processes;
        priority_queue<Process, vector<Process>, CompareSRTF> pq;
        currentTime = 0;
        double totalWaitingTime = 0;
        vector<int> waitingTimes(processes.size(), 0);

        while (!tempProcesses.empty() || !pq.empty()) {
            while (!tempProcesses.empty() && tempProcesses.front().arrivalTime <= currentTime) {
                pq.push(tempProcesses.front());
                tempProcesses.erase(tempProcesses.begin());
            }

            if (!pq.empty()) {
                Process current = pq.top();
                pq.pop();

                simulateExecution(current, 1);  // Execute for 1 time unit
                currentTime++;

                if (current.remainingTime > 0) {
                    pq.push(current);
                } else {
                    waitingTimes[current.id - 1] = currentTime - current.arrivalTime - current.burstTime;
                }
            } else {
                currentTime++;
            }
        }

        for (int wt : waitingTimes) {
            totalWaitingTime += wt;
        }
        return totalWaitingTime / processes.size();
    }

    double multilevelFeedbackQueue() {
        vector<Process> tempProcesses = processes;
        queue<Process> foregroundQueue;
        priority_queue<Process, vector<Process>, CompareSRTF> backgroundQueue;
        currentTime = 0;
        double totalWaitingTime = 0;
        const int FOREGROUND_TIME = 10;
        const int BACKGROUND_TIME = 3;

        // Separate processes into queues
        for (const Process& p : tempProcesses) {
            if (p.priority >= 5) {
                foregroundQueue.push(p);
            } else {
                backgroundQueue.push(p);
            }
        }

        while (!foregroundQueue.empty() || !backgroundQueue.empty()) {
            // Process foreground queue (Priority based)
            for (int i = 0; i < FOREGROUND_TIME && !foregroundQueue.empty(); i++) {
                Process current = foregroundQueue.front();
                foregroundQueue.pop();

                simulateExecution(current, 1);
                currentTime++;

                if (current.remainingTime > 0) {
                    foregroundQueue.push(current);
                } else {
                    totalWaitingTime += currentTime - current.arrivalTime - current.burstTime;
                }
            }

            // Process background queue (SRTF)
            for (int i = 0; i < BACKGROUND_TIME && !backgroundQueue.empty(); i++) {
                Process current = backgroundQueue.top();
                backgroundQueue.pop();

                simulateExecution(current, 1);
                currentTime++;

                if (current.remainingTime > 0) {
                    backgroundQueue.push(current);
                } else {
                    totalWaitingTime += currentTime - current.arrivalTime - current.burstTime;
                }
            }
        }

        return totalWaitingTime / processes.size();
    }
};

int main() {
    Scheduler scheduler;
    scheduler.inputProcesses();

    cout << "\nRunning Priority Based Scheduling...\n";
    double avgWaitingTimePriority = scheduler.priorityScheduling();

    cout << "\nRunning Shortest Remaining Time First...\n";
    double avgWaitingTimeSRTF = scheduler.srtfScheduling();

    cout << "\nRunning Multilevel Feedback Queue...\n";
    double avgWaitingTimeMFQ = scheduler.multilevelFeedbackQueue();

    cout << "\nResults:\n";
    cout << fixed << setprecision(2);
    cout << "Average Waiting Time (Priority): " << avgWaitingTimePriority << endl;
    cout << "Average Waiting Time (SRTF): " << avgWaitingTimeSRTF << endl;
    cout << "Average Waiting Time (MFQ): " << avgWaitingTimeMFQ << endl;

    return 0;
}