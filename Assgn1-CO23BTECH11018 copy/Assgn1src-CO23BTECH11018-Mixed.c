//Used Mixed type for thread allocation
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>

#define MAX_N 1000

//Here we declare the variables
int N, K; // Size of Grid and Number of Threads 
int sudoku[MAX_N][MAX_N]; //Sudoku Grid
int row_valid[MAX_N], col_valid[MAX_N], grid_valid[MAX_N]; //Binary Grids 
int invalid_found = 0;  // Common flag for Checking Validity
pthread_mutex_t lock; //Mutex - Learnt from internet to prevent race condition
FILE *output_file;

typedef struct {
    int index;
    int total;
    int thread_num;
} thread_info;

//Function to Check Validity of Rows
void *check_rows(void *arg) {
    thread_info *data = (thread_info *)arg;
    for (int i = data->index; i < N; i += data->total) {
        int check[MAX_N];
        for (int j = 0; j < N; j++) check[j] = 1; // Initialize binary array with 1's

        for (int j = 0; j < N; j++) {
            int val = sudoku[i][j];
            if (val >= 1 && val <= N) check[val - 1] = 0;  //We mark the found numbers
        }

        int sum = 0;
        for (int j = 0; j < N; j++) sum += check[j];

        if (sum > 0) {
            pthread_mutex_lock(&lock);
            invalid_found = 1;
            pthread_mutex_unlock(&lock);
            fprintf(output_file, "Thread %d checks row %d and is NOT valid.\n", data->thread_num, i + 1);
        } else {
            fprintf(output_file, "Thread %d checks row %d and is valid.\n", data->thread_num, i + 1);
        }
    }
    return NULL;
}

//Function to Check Columns Validity
//Implemented same as above
void *check_columns(void *arg) {
    thread_info *data = (thread_info *)arg;
    for (int j = data->index; j < N; j += data->total) {
        int check[MAX_N];
        for (int i = 0; i < N; i++) check[i] = 1;

        for (int i = 0; i < N; i++) {
            int val = sudoku[i][j];
            if (val >= 1 && val <= N) check[val - 1] = 0;
        }

        int sum = 0;
        for (int i = 0; i < N; i++) sum += check[i];

        if (sum > 0) {
            pthread_mutex_lock(&lock);
            invalid_found = 1;
            pthread_mutex_unlock(&lock);
            fprintf(output_file, "Thread %d checks column %d and is NOT valid.\n", data->thread_num, j + 1);
        } else {
            fprintf(output_file, "Thread %d checks column %d and is valid.\n", data->thread_num, j + 1);
        }
    }
    return NULL;
}

//Function to Check Columns Validity
//This has also been implemented same as above
void *check_subgrids(void *arg) {
    thread_info *data = (thread_info *)arg;
    int n = (int)sqrt(N);
    if (n * n != N) {
        fprintf(output_file, "Invalid Sudoku size: N is not a perfect square.\n");
        return NULL;
    }

    for (int grid = data->index; grid < N; grid += data->total) {
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
            pthread_mutex_lock(&lock);
            invalid_found = 1;
            pthread_mutex_unlock(&lock);
            fprintf(output_file, "Thread %d checks subgrid %d and is NOT valid.\n", data->thread_num, grid + 1);
        } else {
            fprintf(output_file, "Thread %d checks subgrid %d and is valid.\n", data->thread_num, grid + 1);
        }
    }
    return NULL;
}

//Main Function
int main() {
    struct timeval start, end;
    //File Operations
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
    pthread_mutex_init(&lock, NULL);

    gettimeofday(&start, NULL);

    pthread_t threads[K];
    thread_info thread_data[K];

    // Cyclic manner
    int k1 = K / 3;  // For rows
    int k2 = K / 3 + (K % 3) / 2; //So that if there are 2 threads after dividing by 3, they will be equally distributed among k2 and k3
    int k3 = K - k1 - k2;  // For subgrids

    // Create threads for checking rows
    for (int i = 0; i < k1; i++) {
        thread_data[i] = (thread_info){.index = i, .total = k1, .thread_num = i + 1};
        pthread_create(&threads[i], NULL, check_rows, &thread_data[i]);
    }

    // Create threads for checking columns
    for (int i = 0; i < k2; i++) {
        thread_data[k1 + i] = (thread_info){.index = i, .total = k2, .thread_num = k1 + i + 1};
        pthread_create(&threads[k1 + i], NULL, check_columns, &thread_data[k1 + i]);
    }

    // Create threads for checking subgrids
    for (int i = 0; i < k3; i++) {
        thread_data[k1 + k2 + i] = (thread_info){.index = i, .total = k3, .thread_num = k1 + k2 + i + 1};
        pthread_create(&threads[k1 + k2 + i], NULL, check_subgrids, &thread_data[k1 + k2 + i]);
    }

    for (int i = 0; i < K; i++) pthread_join(threads[i], NULL);

    fprintf(output_file, "Sudoku is %s.\n", invalid_found ? "invalid" : "valid");

    gettimeofday(&end, NULL);
    double elapsed_time = (end.tv_sec - start.tv_sec) * 1000000.0 + (end.tv_usec - start.tv_usec);
    fprintf(output_file, "Total time taken: %.2f microseconds.\n", elapsed_time);

    fclose(output_file);
    pthread_mutex_destroy(&lock);
    return 0;
}
