//Sequential type of execution/validation

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>

#define MAX_N 1000

int N, K;
int sudoku[MAX_N][MAX_N];
int invalid_found = 0;  // Overall Checker for Stopping
FILE *output_file;

void check_rows() {
    for (int i = 0; i < N; i++) {
        int check[MAX_N];
        for (int j = 0; j < N; j++) check[j] = 1;  // Initialize binary array

        for (int j = 0; j < N; j++) {
            int val = sudoku[i][j];
            if (val >= 1 && val <= N) check[val - 1] = 0;  // Mark found numbers
        }

        int sum = 0;
        for (int j = 0; j < N; j++) sum += check[j];

        if (sum > 0) {
            invalid_found = 1;
            fprintf(output_file, "Row %d is NOT valid.\n", i + 1);
        } else {
            fprintf(output_file, "Row %d is valid.\n", i + 1);
        }
    }
}

void check_columns() {
    for (int j = 0; j < N; j++) {
        int check[MAX_N];
        for (int i = 0; i < N; i++) check[i] = 1;

        for (int i = 0; i < N; i++) {
            int val = sudoku[i][j];
            if (val >= 1 && val <= N) check[val - 1] = 0;
        }

        int sum = 0;
        for (int i = 0; i < N; i++) sum += check[i];

        if (sum > 0) {
            invalid_found = 1;
            fprintf(output_file, "Column %d is NOT valid.\n", j + 1);
        } else {
            fprintf(output_file, "Column %d is valid.\n", j + 1);
        }
    }
}

void check_subgrids() {
    int n = (int)sqrt(N);
    if (n * n != N) {
        fprintf(output_file, "Invalid Sudoku size: N is not a perfect square.\n");
        return;
    }

    for (int grid = 0; grid < N; grid++) {
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
            invalid_found = 1;
            fprintf(output_file, "Subgrid %d is NOT valid.\n", grid + 1);
        } else {
            fprintf(output_file, "Subgrid %d is valid.\n", grid + 1);
        }
    }
}

int main() {
    struct timeval start, end;

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

    gettimeofday(&start, NULL);

    // Validating rows
    check_rows();

    // Validating columns
    check_columns();

    // Validating subgrids
    check_subgrids();

    fprintf(output_file, "Sudoku is %s.\n", invalid_found ? "invalid" : "valid");

    gettimeofday(&end, NULL);
    double elapsed_time = (end.tv_sec - start.tv_sec) * 1000000.0 + (end.tv_usec - start.tv_usec);
    fprintf(output_file, "Total time taken: %.2f microseconds.\n", elapsed_time);

    fclose(output_file);
    return 0;
}
