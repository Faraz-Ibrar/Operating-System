#include <iostream>
#include <vector>
#include <cmath>
#include <pthread.h>

using namespace std;

const int IMAGE_SIZE = 16;
const int NUM_THREADS = 16;
const int BLOCK_SIZE = 4;
const int NUM_BLOCKS_PER_ROW = IMAGE_SIZE / BLOCK_SIZE;

vector<vector<int>> image = {
    {251, 255, 254, 252, 253, 255, 254, 253, 253, 253, 250, 251, 251, 252, 254, 253},
    {253, 251, 254, 254, 254, 252, 252, 252, 250, 252, 16, 15, 250, 253, 249, 252},
    {254, 255, 254, 255, 254, 253, 254, 253, 252, 247, 131, 171, 14, 249, 251, 255},
    {254, 251, 254, 251, 251, 251, 247, 250, 251, 245, 101, 121, 155, 10, 251, 254},
    {253, 252, 250, 247, 27, 37, 239, 65, 109, 200, 230, 126, 149, 179, 3, 254},
    {252, 252, 250, 26, 169, 118, 88, 156, 90, 239, 239, 10, 9, 7, 253, 250},
    {255, 252, 247, 52, 186, 92, 153, 113, 155, 96, 91, 245, 249, 251, 254, 253},
    {254, 251, 243, 61, 224, 239, 130, 113, 120, 142, 119, 246, 252, 251, 254, 248},
    {250, 251, 91, 123, 116, 227, 228, 238, 95, 152, 8, 251, 254, 253, 252, 254},
    {253, 248, 160, 117, 97, 141, 111, 242, 230, 95, 251, 253, 253, 253, 255, 251},
    {241, 74, 242, 244, 244, 63, 113, 9, 248, 5, 253, 253, 253, 252, 252, 254},
    {241, 148, 12, 97, 246, 9, 28, 210, 2, 253, 247, 254, 252, 254, 255, 251},
    {245, 111, 169, 126, 242, 248, 159, 5, 253, 254, 255, 253, 255, 252, 254, 254},
    {250, 9, 174, 142, 148, 6, 5, 252, 254, 254, 253, 253, 253, 254, 254, 252},
    {254, 250, 7, 7, 3, 252, 253, 252, 255, 252, 252, 254, 253, 254, 253, 254},
    {252, 254, 254, 254, 255, 254, 252, 254, 251, 253, 253, 253, 252, 253, 255, 254}
};

int histogram[256] = {0};
double mean_blocks[NUM_THREADS];
double overall_mean = 0;
pthread_mutex_t mutex_histogram = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_mean = PTHREAD_MUTEX_INITIALIZER;

// Thread function to calculate the histogram
void* calculate_histogram(void* arg) {
    int thread_id = *(int*)arg;
    int start_row = (thread_id / BLOCK_SIZE) * BLOCK_SIZE;
    int start_col = (thread_id % BLOCK_SIZE) * BLOCK_SIZE;
    
    for (int i = start_row; i < start_row + BLOCK_SIZE; ++i) {
        for (int j = start_col; j < start_col + BLOCK_SIZE; ++j) {
            pthread_mutex_lock(&mutex_histogram);
            histogram[image[i][j]]++;
            pthread_mutex_unlock(&mutex_histogram);
        }
    }
    pthread_exit(0);
}

// Thread function to calculate the mean of each block
void* calculate_mean(void* arg) {
    int thread_id = *(int*)arg;
    int start_row = (thread_id / BLOCK_SIZE) * BLOCK_SIZE;
    int start_col = (thread_id % BLOCK_SIZE) * BLOCK_SIZE;

    double sum = 0;
    for (int i = start_row; i < start_row + BLOCK_SIZE; ++i) {
        for (int j = start_col; j < start_col + BLOCK_SIZE; ++j) {
            sum += image[i][j];
        }
    }
    double mean = sum / (BLOCK_SIZE * BLOCK_SIZE);

    pthread_mutex_lock(&mutex_mean);
    mean_blocks[thread_id] = mean;
    pthread_mutex_unlock(&mutex_mean);

    pthread_exit(0);
}

// Thread function to replace pixel value with overall mean
void* replace_pixel(void* arg) {
    int block_num = *(int*)arg;

    // Calculate block's position in the grid
    int block_row = block_num / NUM_BLOCKS_PER_ROW;
    int block_col = block_num % NUM_BLOCKS_PER_ROW;

    // Block's top-left corner
    int start_row = block_row * BLOCK_SIZE;
    int start_col = block_col * BLOCK_SIZE;

    // Relative pixel within the block
    int replace_row = block_num / BLOCK_SIZE; // Adjust based on block size
    int replace_col = block_num % BLOCK_SIZE; // Adjust based on block size

    // Update pixel value using mutex for thread safety
    pthread_mutex_lock(&mutex_mean);
    image[start_row + replace_row][start_col + replace_col] = static_cast<int>(overall_mean);
    pthread_mutex_unlock(&mutex_mean);

    pthread_exit(0);
}


// Function to print the histogram
void print_histogram() {
    cout << "Grayscale\tFrequency" << endl;
    for (int i = 0; i < 256; ++i) {
        if (histogram[i] > 0) {
            cout << i << "\t\t" << histogram[i] << endl;
        }
    }
}

// Function to calculate the overall mean of the image
double calculate_overall_mean() {
    double sum = 0;
    for (const auto& row : image) {
        for (const auto& val : row) {
            sum += val;
        }
    }
    return sum / (IMAGE_SIZE * IMAGE_SIZE);
}

int main() {
    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];

    // Step 1: Calculate the histogram using 16 threads
    for (int i = 0; i < NUM_THREADS; ++i) {
        thread_ids[i] = i;
        pthread_create(&threads[i], nullptr, calculate_histogram, (void*)&thread_ids[i]);
    }
    for (int i = 0; i < NUM_THREADS; ++i) {
        pthread_join(threads[i], nullptr);
    }

    cout << "Original Image Histogram:" << endl;
    print_histogram();

    // Step 2: Calculate the mean of each block using 16 threads
    for (int i = 0; i < NUM_THREADS; ++i) {
        pthread_create(&threads[i], nullptr, calculate_mean, (void*)&thread_ids[i]);
    }
    for (int i = 0; i < NUM_THREADS; ++i) {
        pthread_join(threads[i], nullptr);
    }

    cout << "\nMean Values of Each 4x4 Block:" << endl;
    for (int i = 0; i < NUM_THREADS; ++i) {
        cout << "Block " << i << ": " << mean_blocks[i] << endl;
    }

    overall_mean = calculate_overall_mean();
    cout << "\nOverall Mean of the Image: " << overall_mean << endl;

    cout << "\nBlocks with Mean Difference Less Than 1:" << endl;
    for (int i = 0; i < NUM_THREADS; ++i) {
        if (fabs(mean_blocks[i] - overall_mean) < 1) {
            cout << "Block " << i << endl;
        }
    }

    // Step 3: Modify the image based on mean values using 16 threads
    for (int i = 0; i < NUM_THREADS; ++i) {
        pthread_create(&threads[i], nullptr, replace_pixel, (void*)&thread_ids[i]);
    }
    for (int i = 0; i < NUM_THREADS; ++i) {
        pthread_join(threads[i], nullptr);
    }

    fill(begin(histogram), end(histogram), 0);  // Reset histogram

        // Step 4: Recalculate the histogram for the modified image using 16 threads
    for (int i = 0; i < NUM_THREADS; ++i) {
        pthread_create(&threads[i], nullptr, calculate_histogram, (void*)&thread_ids[i]);
    }
    for (int i = 0; i < NUM_THREADS; ++i) {
        pthread_join(threads[i], nullptr);
    }

    cout << "\nModified Image Histogram:" << endl;
    print_histogram();

    return 0;
}
