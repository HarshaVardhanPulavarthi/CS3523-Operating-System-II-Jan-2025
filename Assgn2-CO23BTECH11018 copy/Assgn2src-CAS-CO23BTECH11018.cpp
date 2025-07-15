//We are validating a sudoku by dynamic allocation of threads. We are using CAS method to prevent simultaneous accession of common variable

//Header files
#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
#include <mutex>
#include <iomanip>
#include <sstream> // Added for stringstream

using namespace std;
using namespace std::chrono;

// Global variables
int K, N, taskInc;          // From input: #threads, size, tasks per increment
vector<vector<int>> sudoku; // Sudoku grid
atomic<int> C(0);           // Shared counter
atomic<bool> cas_lock(false); // In this code -> Cas Lock
bool sudokuValid = true;    // Final result
mutex outputMutex;          // For synchronized output

// Timing structure
struct ThreadTiming {
    vector<double> csEnterTimes;
    vector<double> csExitTimes;
};
vector<ThreadTiming> threadTimings;

//Defining the functions:
// Function prototypes - We are using function prototypes as a lot of functions are defined and we will face an issue with the order of definition
void validateTask(int threadId);
bool validateRow(int row);
bool validateCol(int col);
bool validateGrid(int startRow, int startCol);
chrono::high_resolution_clock::time_point casLock(int threadId); // Renamed to casLock
void casUnlock(int threadId, chrono::high_resolution_clock::time_point enterTime); // Renamed to casUnlock
string getCurrentTime();

// Main validation function using binary array
bool validateSection(const vector<int>& section) {
    vector<int> finder(N, 1); // Binary array initialized to 1s
    for (int num : section) {
        finder[num-1] = 0;    // Mark number as found
    }
    int sum = 0;
    for (int val : finder) sum += val;
    return sum == 0;
}

int main() {
    // Read input

    //if theres an error opening the file
    ifstream inFile("inp.txt");
    if (!inFile) {
        outputMutex.lock();
        ofstream outFile("output.txt", ios::app); // Append mode
        outFile << "Error opening input file!" << endl;
        outFile.close();
        outputMutex.unlock();
        return 1;
    }
    
    //Reading the input from the file
    inFile >> K >> N >> taskInc;
    sudoku.resize(N, vector<int>(N));
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            inFile >> sudoku[i][j];
    inFile.close();

    // Initialize timing data
    threadTimings.resize(K);

    // Creating the threads
    vector<thread> threads;
    auto startTime = high_resolution_clock::now();
    
    //Giving the threads IDs
    for (int i = 0; i < K; i++)
        threads.emplace_back(validateTask, i + 1);  

    // Join threads
    for (auto& t : threads)
        t.join();

    auto endTime = high_resolution_clock::now();
    double totalTime = duration_cast<microseconds>(endTime - startTime).count();

    // Calculate timing statistics
    double avgEnter = 0, avgExit = 0, worstEnter = 0, worstExit = 0;
    int totalEntries = 0;
    
    for (const auto& timing : threadTimings) {
        for (double t : timing.csEnterTimes) {
            avgEnter += t;
            worstEnter = max(worstEnter, t);
            totalEntries++;
        }
        for (double t : timing.csExitTimes) {
            avgExit += t;
            worstExit = max(worstExit, t);
        }
    }
    if(totalEntries>0){
        avgEnter = avgEnter / totalEntries;
        avgExit = avgExit / totalEntries;
    }

    // Writing the output (after all thread messages)
    ofstream outFile("output.txt", ios::app); // Append mode
    outFile << "Sudoku is " << (sudokuValid ? "valid" : "invalid") << ".\n";
    outFile << "The total time taken is " << fixed << setprecision(2) 
            << totalTime << " microseconds.\n";
    outFile << "Average time taken by a thread to enter the CS is " 
            << avgEnter << " microseconds\n";
    outFile << "Average time taken by a thread to exit the CS is " 
            << avgExit << " microseconds\n";
    outFile << "Worst-case time taken by a thread to enter the CS is " 
            << worstEnter << " microseconds\n";
    outFile << "Worst-case time taken by a thread to exit the CS is " 
            << worstExit << " microseconds\n";
    outFile.close();

    return 0;
}

// Critical Section - CAS Lock this time
chrono::high_resolution_clock::time_point casLock(int threadId) {
    auto requestTime = high_resolution_clock::now();
    outputMutex.lock();
    ofstream outFile1("output.txt", ios::app); // Append mode
    outFile1 << "Thread " << threadId << " requests to enter CS at " 
             << getCurrentTime() << " hrs\n";
    outFile1.close();
    outputMutex.unlock();

    bool expected = false;
    while (!cas_lock.compare_exchange_strong(expected, true)) { // CAS spinlock
        expected = false; // Reset expected value for next attempt
    }
    
    auto enterTime = high_resolution_clock::now();
    double elapsed = duration_cast<microseconds>(enterTime - requestTime).count();
    threadTimings[threadId-1].csEnterTimes.push_back(elapsed);
    
    outputMutex.lock();
    ofstream outFile2("output.txt", ios::app); // Append mode
    outFile2 << "Thread " << threadId << " enters CS at " 
             << getCurrentTime() << " hrs\n";
    outFile2.close();
    outputMutex.unlock();
    
    return enterTime; // Return enter time for CS duration calculation
}

//Leaving the lock
void casUnlock(int threadId, chrono::high_resolution_clock::time_point enterTime) {
    cas_lock.store(false); // Release the lock
    auto exitTime = high_resolution_clock::now();
    double elapsed = duration_cast<microseconds>(exitTime - enterTime).count();
    threadTimings[threadId-1].csExitTimes.push_back(elapsed);
    
    outputMutex.lock();
    ofstream outFile("output.txt", ios::app); // Append mode
    outFile << "Thread " << threadId << " leaves CS at " 
            << getCurrentTime() << " hrs\n";
    outFile.close();
    outputMutex.unlock();
}

//To get the current time (with microseconds in brackets)
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

void validateTask(int threadId) {
    int n = sqrt(N); // Size of subgrids
    bool localValid = true; // Local validity flag for this thread's tasks

    while (C < 3*N && sudokuValid) { // Added early termination with sudokuValid
        // Critical Section
        auto enterTime = casLock(threadId); // Updated to casLock
        int startTask = C;
        C += taskInc;
        int endTask = min(startTask + taskInc, 3*N);
        
        outputMutex.lock();
        ofstream outFile1("output.txt", ios::app); // Append mode
        outFile1 << "Thread " << threadId << " grabs tasks " << startTask 
                 << " to " << endTask-1 << " at " << getCurrentTime() << " hrs\n";
        outFile1.close();
        outputMutex.unlock();
        
        casUnlock(threadId, enterTime); 

        // Validation Section
        for (int task = startTask; task < endTask && sudokuValid; task++) { // Added early termination
            bool valid = false;
            //0 to N-1 are Rows, N to 2N-1 are columns, 2N to 3N-1 are Subgrids
            if (task < N) { // Rows
                valid = validateRow(task);
                outputMutex.lock();
                ofstream outFile2("output.txt", ios::app); // Append mode
                outFile2 << "Thread " << threadId << " completes checking of row " 
                         << task << " at " << getCurrentTime() << " hrs and finds it as " 
                         << (valid ? "valid" : "invalid") << ".\n";
                outFile2.close();
                outputMutex.unlock();
            }
            else if (task < 2*N) { // Columns
                valid = validateCol(task - N);
                outputMutex.lock();
                ofstream outFile2("output.txt", ios::app); // Append mode
                outFile2 << "Thread " << threadId << " completes checking of column " 
                         << (task-N) << " at " << getCurrentTime() << " hrs and finds it as " 
                         << (valid ? "valid" : "invalid") << ".\n";
                outFile2.close();
                outputMutex.unlock();
            }
            else { // Grids
                int gridNum = task - 2*N;
                int startRow = (gridNum / n) * n;
                int startCol = (gridNum % n) * n;
                valid = validateGrid(startRow, startCol);
                outputMutex.lock();
                ofstream outFile2("output.txt", ios::app); // Append mode
                outFile2 << "Thread " << threadId << " completes checking of grid " 
                         << gridNum << " at " << getCurrentTime() << " hrs and finds it as " 
                         << (valid ? "valid" : "invalid") << ".\n";
                outFile2.close();
                outputMutex.unlock();
            }
            localValid = localValid && valid; // Update local validity
            if (!valid) {
                outputMutex.lock();
                sudokuValid = false; // Set global invalid flag if any task fails
                outputMutex.unlock();
            }
        }
    }

    // Update global sudokuValid after all tasks are complete (if not already invalidated)
    outputMutex.lock();
    sudokuValid = sudokuValid && localValid;
    outputMutex.unlock();
}

//Validating a single row
bool validateRow(int row) {
    vector<int> section(N);
    for (int j = 0; j < N; j++)
        section[j] = sudoku[row][j];
    return validateSection(section);
}

//Validating a single column
bool validateCol(int col) {
    vector<int> section(N);
    for (int i = 0; i < N; i++)
        section[i] = sudoku[i][col];
    return validateSection(section);
}

//Validating a single subgrid
bool validateGrid(int startRow, int startCol) {
    int n = sqrt(N);
    vector<int> section(N);
    int idx = 0;
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            section[idx++] = sudoku[startRow + i][startCol + j];
    return validateSection(section);
}