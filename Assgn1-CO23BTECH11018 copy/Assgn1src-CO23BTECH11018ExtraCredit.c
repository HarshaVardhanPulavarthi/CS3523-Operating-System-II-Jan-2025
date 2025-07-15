//Applied early termination to Chunk Method 
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include <signal.h>

#define MAX_N 1000

// Here we declare the variables
int N, K; // Size of Grid and Number of Threads
int sudoku[MAX_N][MAX_N]; // Sudoku Grid
int invalid_found = 0;  // Shared flag for early stopping (will be set via signal)
FILE *output_file;

typedef struct {
    int index;
    int chunk_size;
    int extra;
    int thread_num;
} thread_info;

// Signal handler to set invalid_found flag
void handle_invalid_signal(int sig) {
    invalid_found = 1;
}

// Function to Check Validity of Rows
void *check_rows(void *arg) {
    thread_info *data = (thread_info *)arg;
    for (int i = data->index; i < data->index + data->chunk_size + data->extra && i < N; i++) {
        if (invalid_found) return NULL;  // Early exit if invalid found

        int check[MAX_N];
        for (int j = 0; j < N; j++) check[j] = 1;  // Initialize binary array

        for (int j = 0; j < N; j++) {
            int val = sudoku[i][j];
            if (val >= 1 && val <= N) check[val - 1] = 0;  // Mark found numbers
        }

        int sum = 0;
        for (int j = 0; j < N; j++) sum += check[j];

        if (sum > 0) {
            fprintf(output_file, "Thread %d checks row %d and is NOT valid.\n", data->thread_num, i + 1);
            // Send signal to other threads
            pthread_kill(pthread_self(), SIGUSR1);
            pthread_exit(NULL);
        } else {
            fprintf(output_file, "Thread %d checks row %d and is valid.\n", data->thread_num, i + 1);
        }
    }
    return NULL;
}

// Function to Check Columns Validity
void *check_columns(void *arg) {
    thread_info *data = (thread_info *)arg;
    for (int j = data->index; j < data->index + data->chunk_size + data->extra && j < N; j++) {
        if (invalid_found) return NULL;  // Early exit if invalid found

        int check[MAX_N];
        for (int i = 0; i < N; i++) check[i] = 1;

        for (int i = 0; i < N; i++) {
            int val = sudoku[i][j];
            if (val >= 1 && val <= N) check[val - 1] = 0;
        }

        int sum = 0;
        for (int i = 0; i < N; i++) sum += check[i];

        if (sum > 0) {
            fprintf(output_file, "Thread %d checks column %d and is NOT valid.\n", data->thread_num, j + 1);
            // Send signal to other threads
            pthread_kill(pthread_self(), SIGUSR1);
            pthread_exit(NULL);
        } else {
            fprintf(output_file, "Thread %d checks column %d and is valid.\n", data->thread_num, j + 1);
        }
    }
    return NULL;
}

// Function to Check Subgrid Validity
void *check_subgrids(void *arg) {
    thread_info *data = (thread_info *)arg;
    int n = (int)sqrt(N);
    if (n * n != N) {
        fprintf(output_file, "Invalid Sudoku size: N is not a perfect square.\n");
        return NULL;
    }

    for (int grid = data->index; grid < data->index + data->chunk_size + data->extra && grid < N; grid++) {
        if (invalid_found) return NULL;  // Early exit if invalid found

        int check[MAX_N];
        for (int i = 0; i < N; i++) check[i] = 1;

        int row_start = (grid / n) * n, col_start = (grid % n) * n;
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                int val = sudoku[row_start + i][col_start + j];
                if (val >= 1 && val <= N) check[val - 1] = 0;
            }
        }

        int sum = 0;
        for (int i = 0; i < N; i++) sum += check[i];

        if (sum > 0) {
            fprintf(output_file, "Thread %d checks subgrid %d and is NOT valid.\n", data->thread_num, grid + 1);
            // This will send signal to other threads
            pthread_kill(pthread_self(), SIGUSR1);
            pthread_exit(NULL);
        } else {
            fprintf(output_file, "Thread %d checks subgrid %d and is valid.\n", data->thread_num, grid + 1);
        }
    }
    return NULL;
}

// Main Function
int main() {
    struct timeval start, end;

    // File Operations
    FILE *input_file = fopen("inp.txt", "r");
    output_file = fopen("output.txt", "w");
    if (!input_file || !output_file) {
        printf("Error opening files.\n");
        return 1;
    }

    fscanf(input_file, "%d %d", &K, &N);
    if (N > MAX_N) {
        fprintf(output_file, "Error: Sudoku size exceeds MAX_N.\n");
        return 1;
    }

    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            fscanf(input_file, "%d", &sudoku[i][j]);

    fclose(input_file);

    // We set up signal handler
    signal(SIGUSR1, handle_invalid_signal);

    gettimeofday(&start, NULL);

    pthread_t threads[K];
    thread_info thread_data[K];

    int k1 = K / 3;
    int k2 = K / 3 + (K % 3) / 2; // So that if there are 2 threads after dividing by 3, they will be equally distributed among k2 and k3
    int k3 = K - k1 - k2;
    int index = 0;

    // Creating threads for Rows
    int chunk_size = N / k1, extra = N % k1;
    for (int i = 0; i < k1; i++, index++) {
        thread_data[index] = (thread_info){.index = i * chunk_size, .chunk_size = chunk_size, .extra = (i == k1 - 1 ? extra : 0), .thread_num = index + 1};
        pthread_create(&threads[index], NULL, check_rows, &thread_data[index]);
    }

    // Creating threads for Columns
    chunk_size = N / k2, extra = N % k2;
    for (int i = 0; i < k2; i++, index++) {
        thread_data[index] = (thread_info){.index = i * chunk_size, .chunk_size = chunk_size, .extra = (i == k2 - 1 ? extra : 0), .thread_num = index + 1};
        pthread_create(&threads[index], NULL, check_columns, &thread_data[index]);
    }

    // Creating threads for subgrids
    chunk_size = N / k3, extra = N % k3;
    for (int i = 0; i < k3; i++, index++) {
        thread_data[index] = (thread_info){.index = i * chunk_size, .chunk_size = chunk_size, .extra = (i == k3 - 1 ? extra : 0), .thread_num = index + 1};
        pthread_create(&threads[index], NULL, check_subgrids, &thread_data[index]);
    }

    for (int i = 0; i < K; i++) pthread_join(threads[i], NULL);

    fprintf(output_file, "Sudoku is %s.\n", invalid_found ? "invalid" : "valid");

    gettimeofday(&end, NULL);
    double elapsed_time = (end.tv_sec - start.tv_sec) * 1000000.0 + (end.tv_usec - start.tv_usec);
    fprintf(output_file, "Total time taken: %.2f microseconds.\n", elapsed_time);

    // Closing
    fclose(output_file);
    return 0;
}
