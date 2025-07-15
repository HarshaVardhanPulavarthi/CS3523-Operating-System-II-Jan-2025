//We are just checking the validity of sudoku sequentially
#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <iomanip>
#include <sstream>

using namespace std;
using namespace std::chrono;

// Global variables
int K, N, taskInc;          // From input: #threads (unused), size, tasks per increment (unused)
vector<vector<int>> sudoku; // Sudoku grid
bool sudokuValid = true;    // Final result

//Defining the functions:
// Function prototypes - We are using function prototypes as a lot of functions are defined and we will face an issue with the order of definition
bool validateRow(int row);
bool validateCol(int col);
bool validateGrid(int startRow, int startCol);
string getCurrentTime();

// Validation Function
bool validateSection(const vector<int>& section) {
    vector<int> finder(N, 1); // Binary array initialized to 1s
    for (int num : section) {
        finder[num-1] = 0;    // Mark number as found
    }
    int sum = 0;
    for (int val : finder) sum += val;
    return sum == 0; //If final sum is 0 then valid else invalid
}

int main() {
    // Read input

    //If theres an issue with the file
    ifstream inFile("inp.txt");
    if (!inFile) {
        ofstream outFile("output.txt", ios::app); // Append mode
        outFile << "Error opening input file!" << endl;
        outFile.close();
        return 1;
    }
    
    //Reading the input
    inFile >> K >> N >> taskInc; // K and taskInc are unused in sequential version
    sudoku.resize(N, vector<int>(N));
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            inFile >> sudoku[i][j];
    inFile.close();

    // Start timing
    auto startTime = high_resolution_clock::now();

    // Sequential validation of all tasks (3*N tasks: N rows, N columns, N grids)
    int n = sqrt(N); // Size of subgrids
    bool valid;

    // Validating rows
    for (int i = 0; i < N && sudokuValid; i++) { // Added early termination with sudokuValid
        valid = validateRow(i);
        ofstream outFile1("output.txt", ios::app); // Append mode
        outFile1 << "Completed checking of row " << i << " at " << getCurrentTime() 
                 << " hrs and found it as " << (valid ? "valid" : "invalid") << ".\n";
        outFile1.close();
        sudokuValid = sudokuValid && valid; // Update validity
        if (!valid) break; // Early termination if invalid
    }

    // Validating columns (only if still valid)
    if (sudokuValid) {
        for (int j = 0; j < N && sudokuValid; j++) { // Added early termination
            valid = validateCol(j);
            ofstream outFile2("output.txt", ios::app); // Append mode
            outFile2 << "Completed checking of column " << j << " at " << getCurrentTime() 
                     << " hrs and found it as " << (valid ? "valid" : "invalid") << ".\n";
            outFile2.close();
            sudokuValid = sudokuValid && valid; // Update validity
            if (!valid) break; // Early termination if invalid
        }
    }

    // Validating grids (only if still valid)
    if (sudokuValid) {
        for (int gridNum = 0; gridNum < N && sudokuValid; gridNum++) { // Added early termination
            int startRow = (gridNum / n) * n;
            int startCol = (gridNum % n) * n;
            valid = validateGrid(startRow, startCol);
            ofstream outFile3("output.txt", ios::app); // Append mode
            outFile3 << "Completed checking of grid " << gridNum << " at " << getCurrentTime() 
                     << " hrs and found it as " << (valid ? "valid" : "invalid") << ".\n";
            outFile3.close();
            sudokuValid = sudokuValid && valid; // Update validity
            if (!valid) break; // Early termination if invalid
        }
    }

    // End timing
    auto endTime = high_resolution_clock::now();
    double totalTime = duration_cast<microseconds>(endTime - startTime).count();

    // Write the output
    ofstream outFile("output.txt", ios::app); // Append mode
    outFile << "Sudoku is " << (sudokuValid ? "valid" : "invalid") << ".\n";
    outFile << "The total time taken is " << fixed << setprecision(2) 
            << totalTime << " microseconds.\n";
    outFile.close();

    return 0;
}

//Getting the current time ( with microseconds in brackets )
string getCurrentTime() {
    auto now = system_clock::now();
    time_t tt = system_clock::to_time_t(now);
    tm local_tm = *localtime(&tt);

    // Get microseconds
    auto us = duration_cast<microseconds>(now.time_since_epoch()) % 1000000;

    stringstream ss;
    ss << setfill('0') << setw(2) << local_tm.tm_hour << ":"
       << setfill('0') << setw(2) << local_tm.tm_min << ":"
       << setfill('0') << setw(2) << local_tm.tm_sec << " ("
       << setfill('0') << setw(6) << us.count() << ")";
    return ss.str();
}

//Function to validate a single row
bool validateRow(int row) {
    vector<int> section(N);
    for (int j = 0; j < N; j++)
        section[j] = sudoku[row][j];
    return validateSection(section);
}

//Function to validate a single column
bool validateCol(int col) {
    vector<int> section(N);
    for (int i = 0; i < N; i++)
        section[i] = sudoku[i][col];
    return validateSection(section);
}

//Function to validate a single subgrid
bool validateGrid(int startRow, int startCol) {
    int n = sqrt(N);
    vector<int> section(N);
    int idx = 0;
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            section[idx++] = sudoku[startRow + i][startCol + j];
    return validateSection(section);
}