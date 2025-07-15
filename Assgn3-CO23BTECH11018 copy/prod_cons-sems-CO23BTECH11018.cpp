#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <queue>
#include <fstream>
#include <random>
#include <chrono>
#include <iomanip>
#include <semaphore> // For semaphores

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
chrono::duration<double> totalProducerTime(0), totalConsumerTime(0);
mutex timeMutex; // Protects total time accumulations

// Random number generator for exponential distribution - Took help from internet to implement 
double getExponentialDelay(double mean) {
    random_device rd;
    mt19937 gen(rd());
    exponential_distribution<> d(1.0 / mean);
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

// Producer Function - Semaphores 
void producer(int id, std::counting_semaphore<10000>& empty, std::counting_semaphore<10000>& full, std::binary_semaphore& buffer_mutex) {
    auto start = chrono::high_resolution_clock::now();
    for (int i = 0; i < cntp; i++) {
        int item = rand() % 1000;

        empty.acquire();        
        buffer_mutex.acquire(); 

        sharedBuffer->insert(item);
        string timeStr = getCurrentTime();
        outFile << i + 1 << "th item: " << item << " produced by thread "
                << id << " at " << timeStr << " into buffer location "
                << sharedBuffer->getSize() << endl;

        buffer_mutex.release(); 
        full.release();         

        this_thread::sleep_for(chrono::milliseconds((int)(getExponentialDelay(mu_p) * 1000)));
    }
    auto end = chrono::high_resolution_clock::now();
    {
        lock_guard<mutex> lock(timeMutex); 
        totalProducerTime += end - start;
    }
}

// Consumer Function - Semaphores
void consumer(int id, std::counting_semaphore<10000>& empty, std::counting_semaphore<10000>& full, std::binary_semaphore& buffer_mutex) {
    auto start = chrono::high_resolution_clock::now();
    for (int i = 0; i < cntc; i++) {
        full.acquire();        
        buffer_mutex.acquire();

        int item = sharedBuffer->remove();
        string timeStr = getCurrentTime();
        outFile << i + 1 << "th item: " << item << " consumed by thread "
                << id << " at " << timeStr << " from buffer location "
                << sharedBuffer->getSize() << endl;

        buffer_mutex.release(); 
        empty.release();        

        this_thread::sleep_for(chrono::milliseconds((int)(getExponentialDelay(mu_c) * 1000)));
    }
    auto end = chrono::high_resolution_clock::now();
    {
        lock_guard<mutex> lock(timeMutex); 
        totalConsumerTime += end - start;
    }
}

int main() {
    // Reading the input parameters from the input file
    ifstream inFile("inp-params.txt");
    int capacity;
    inFile >> capacity >> np >> nc >> cntp >> cntc >> mu_p >> mu_c;
    inFile.close();

    // Initialize the shared buffer
    sharedBuffer = new Buffer(capacity);

    // Open output file
    outFile.open("output-sems.txt");

    // Defining the semaphores 
    const int MAX_SEM_VALUE = 10000; 
    if (capacity > MAX_SEM_VALUE) {
        cerr << "Capacity exceeds maximum semaphore value." << endl;
        return 1;
    }
    std::counting_semaphore<MAX_SEM_VALUE> empty(capacity); // Available slots
    std::counting_semaphore<MAX_SEM_VALUE> full(0);         // Available items
    std::binary_semaphore buffer_mutex(1);                  // Mutual exclusion

    // Create producer and consumer threads
    vector<thread> producers, consumers;
    for (int i = 0; i < np; i++)
        producers.emplace_back(producer, i, ref(empty), ref(full), ref(buffer_mutex));
    for (int i = 0; i < nc; i++)
        consumers.emplace_back(consumer, i, ref(empty), ref(full), ref(buffer_mutex));

    // Wait for all threads to finish
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