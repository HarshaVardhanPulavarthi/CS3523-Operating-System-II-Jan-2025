import subprocess

# Number of runs
NUM_RUNS = 5

# Lists to store the timing metrics from each run
total_times = []
avg_enter_times = []
avg_exit_times = []
worst_enter_times = []
worst_exit_times = []

# Run the TAS-based C++ executable 5 times
for i in range(NUM_RUNS):
    print(f"Starting run {i+1}...")
    # Execute the compiled C++ program
    subprocess.run(["./q1"])
    
    # Read all timing metrics from output.txt
    with open("output.txt", "r") as file:
        lines = file.readlines()
        for line in lines:
            if "The total time taken is" in line:
                time_str = line.split("is")[1].split("microseconds")[0].strip()
                total_times.append(float(time_str))
            elif "Average time taken by a thread to enter the CS is" in line:
                time_str = line.split("is")[1].split("microseconds")[0].strip()
                avg_enter_times.append(float(time_str))
            elif "Average time taken by a thread to exit the CS is" in line:
                time_str = line.split("is")[1].split("microseconds")[0].strip()
                avg_exit_times.append(float(time_str))
            elif "Worst-case time taken by a thread to enter the CS is" in line:
                time_str = line.split("is")[1].split("microseconds")[0].strip()
                worst_enter_times.append(float(time_str))
            elif "Worst-case time taken by a thread to exit the CS is" in line:
                time_str = line.split("is")[1].split("microseconds")[0].strip()
                worst_exit_times.append(float(time_str))

# Calculate and display the averages for all metrics
if total_times and len(total_times) == NUM_RUNS:  # Ensure we got all runs
    avg_total_time = sum(total_times) / NUM_RUNS
    avg_avg_enter = sum(avg_enter_times) / NUM_RUNS
    avg_avg_exit = sum(avg_exit_times) / NUM_RUNS
    avg_worst_enter = sum(worst_enter_times) / NUM_RUNS
    avg_worst_exit = sum(worst_exit_times) / NUM_RUNS

    # Print results
    print("\nResults across 5 runs:")
    print(f"Total times (microseconds): {total_times}")
    print(f"Average total time: {avg_total_time:.2f} microseconds")
    print(f"Average enter CS times (microseconds): {avg_enter_times}")
    print(f"Average of average enter CS time: {avg_avg_enter:.2f} microseconds")
    print(f"Average exit CS times (microseconds): {avg_exit_times}")
    print(f"Average of average exit CS time: {avg_avg_exit:.2f} microseconds")
    print(f"Worst-case enter CS times (microseconds): {worst_enter_times}")
    print(f"Average worst-case enter CS time: {avg_worst_enter:.2f} microseconds")
    print(f"Worst-case exit CS times (microseconds): {worst_exit_times}")
    print(f"Average worst-case exit CS time: {avg_worst_exit:.2f} microseconds")
else:
    print(f"Error: Expected {NUM_RUNS} runs, but got {len(total_times)}. Check output.txt format.")