#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <queue>
#include <fstream>
#include <random>
#include <chrono>
#include <iomanip>

using namespace std;

// Defining a class Buffer for the Buffer
class Buffer {
private:
    queue<int> buffer;
    int capacity;
    
public:
    Buffer(int cap) : capacity(cap) {}
    bool isFull() { return buffer.size() >= capacity; }
    bool isEmpty() { return buffer.empty(); }
    void insert(int item) { buffer.push(item); }
    int remove() {
        int item = buffer.front();
        buffer.pop();
        return item;
    }
    int getSize() { return buffer.size(); }
};

// Defining the variables
Buffer* sharedBuffer;
int np, nc, cntp, cntc;
double mu_p, mu_c;
ofstream outFile;
mutex bufferMutex;
condition_variable bufferNotFull, bufferNotEmpty;
chrono::duration<double> totalProducerTime(0), totalConsumerTime(0);

// Random number generator for exponential distribution - Took help from internet to implement 
double getExponentialDelay(double mean) {
    random_device rd;
    mt19937 gen(rd());
    exponential_distribution<> d(1.0/mean);
    return d(gen);
}

// Created a function to find current time 
string getCurrentTime() {
    auto now = chrono::system_clock::now();
    time_t tt = chrono::system_clock::to_time_t(now);
    stringstream ss;
    ss << put_time(localtime(&tt), "%H:%M:%S");
    return ss.str();
}

// Producer Function - Locks 
void producer(int id) {
    auto start = chrono::high_resolution_clock::now();
    for (int i = 0; i < cntp; i++) {
        int item = rand() % 1000;

        unique_lock<mutex> lock(bufferMutex);
        bufferNotFull.wait(lock, [] { return !sharedBuffer->isFull(); });

        sharedBuffer->insert(item);
        string timeStr = getCurrentTime();

        outFile << i+1 << "th item: " << item << " produced by thread " 
                << id << " at " << timeStr << " into buffer location " 
                << sharedBuffer->getSize() << endl;

        lock.unlock();
        bufferNotEmpty.notify_one();

        this_thread::sleep_for(chrono::milliseconds((int)(getExponentialDelay(mu_p) * 1000)));
    }
    auto end = chrono::high_resolution_clock::now();
    totalProducerTime += end - start;
}

// Consumer Function - Locks
void consumer(int id) {
    auto start = chrono::high_resolution_clock::now();
    for (int i = 0; i < cntc; i++) {
        unique_lock<mutex> lock(bufferMutex);
        bufferNotEmpty.wait(lock, [] { return !sharedBuffer->isEmpty(); });

        int item = sharedBuffer->remove();
        string timeStr = getCurrentTime();

        outFile << i+1 << "th item: " << item << " consumed by thread " 
                << id << " at " << timeStr << " from buffer location " 
                << sharedBuffer->getSize() << endl;

        lock.unlock();
        bufferNotFull.notify_one();

        this_thread::sleep_for(chrono::milliseconds((int)(getExponentialDelay(mu_c) * 1000)));
    }
    auto end = chrono::high_resolution_clock::now();
    totalConsumerTime += end - start;
}

int main() {
    // Reading the input parameters from the input file
    ifstream inFile("inp-params.txt");
    int capacity;
    inFile >> capacity >> np >> nc >> cntp >> cntc >> mu_p >> mu_c;
    inFile.close();
    
    // Initialize the buffer
    sharedBuffer = new Buffer(capacity);
    
    // Open output file
    outFile.open("output-locks.txt");
    
    // Create threads
    vector<thread> producers, consumers;
    for (int i = 0; i < np; i++)
        producers.emplace_back(producer, i);
    for (int i = 0; i < nc; i++)
        consumers.emplace_back(consumer, i);
    
    // Join threads
    for (auto& t : producers) t.join();
    for (auto& t : consumers) t.join();
    
    // Output performance metrics
    outFile << "Average Producer thread time: " << (totalProducerTime.count() / (np*cntp)) << " seconds" << endl;
    outFile << "Average Consumer thread time: " << (totalConsumerTime.count() / (nc*cntc)) << " seconds" << endl;
    
    // Cleanup
    outFile.close();
    delete sharedBuffer;
    
    return 0;
}