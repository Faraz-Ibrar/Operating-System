#include <iostream>
#include <pthread.h>
#include <vector>
#include <algorithm>

using namespace std;

// Shared variables
vector<pair<string, int>> items = { {"eggs", 45}, {"bread", 60}, {"chocolate", 250} };
int sum = 0;
float taxed_sum = 0;
float discounted_sum = 0;

// Thread 1: Calculate the sum of the prices
void* calculateSum(void* arg) {
    cout << "Thread 1:\nItems Purchased: eggs, bread, and chocolate.\n";
    int quantities[] = {3, 1, 5}; // Quantities for eggs, bread, chocolate
    int prices[] = {15, 60, 50};
    
    sum = 0;
    for (size_t i = 0; i < items.size(); ++i) {
        int total = quantities[i] * prices[i];
        cout << items[i].first << " = " << quantities[i] << " x " << prices[i] << " = " << total << "\n";
        sum += total;
    }
    cout << "Sum = " << sum << "\n\n";
    pthread_exit(nullptr);
}

// Thread 2: Calculate the taxed sum
void* calculateTaxedSum(void* arg) {
    float tax_rate = 0.08f;
    taxed_sum = sum + (sum * tax_rate);
    cout << "Thread 2:\nTaxed sum = " << taxed_sum << "\n\n";
    pthread_exit(nullptr);
}

// Thread 3: Calculate the discounted sum if applicable
void* calculateDiscountedSum(void* arg) {
    if (sum > 250) {
        float discount_rate = 0.1f;
        float discount = sum * discount_rate;
        discounted_sum = sum - discount;
        cout << "Thread 3:\nPrice = " << sum << ", Sale = " << discount_rate * 100 << "%, Discount = " << discount 
             << ", Discounted Price = " << discounted_sum << "\n\n";
    } else {
        discounted_sum = sum;
        cout << "Thread 3:\nNo discount applied. Price = " << discounted_sum << "\n\n";
    }
    pthread_exit(nullptr);
}

// Thread 4: Sort items by price
void* sortItemsByPrice(void* arg) {
    sort(items.begin(), items.end(), [](const pair<string, int>& a, const pair<string, int>& b) {
        return a.second > b.second; // Sort in descending order of price
    });

    cout << "Thread 4:\nItems sorted by price:\n";
    for (const auto& item : items) {
        cout << item.first << " " << item.second << "\n";
    }
    cout << "\n";
    pthread_exit(nullptr);
}

int main() {
    pthread_t t1, t2, t3, t4;

    // Create threads
    pthread_create(&t1, nullptr, calculateSum, nullptr);
    pthread_join(t1, nullptr); // Ensure Thread 1 completes before starting Thread 2

    pthread_create(&t2, nullptr, calculateTaxedSum, nullptr);
    pthread_join(t2, nullptr); // Ensure Thread 2 completes before starting Thread 3

    pthread_create(&t3, nullptr, calculateDiscountedSum, nullptr);
    pthread_join(t3, nullptr); // Ensure Thread 3 completes before starting Thread 4

    pthread_create(&t4, nullptr, sortItemsByPrice, nullptr);
    pthread_join(t4, nullptr); // Ensure Thread 4 completes before exiting

    return 0;
}
