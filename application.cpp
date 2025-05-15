//JobScheduler.cpp
//                      break;      
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <map>
#include <ctime>
#include <windows.h>  

enum class SchedulingAlgorithm { FIFO, PRIORITY, DEADLINE };

struct Job {
    int id;
    int priority;
    int executionTime;
    int deadline;
    std::time_t arrivalTime;
    std::vector<int> dependencies;
    bool completed = false;
    int retryCount = 0;


	// A constructor to initialize a Job object

    Job(int i, int p, int t, int d, std::vector<int> deps = {})
        : id(i), priority(p), executionTime(t), deadline(d), dependencies(deps) {
        arrivalTime = std::time(nullptr);
    }
};
   
//This declares a vector of Job objects called jobList.
 // مهم جدا غدآآ
std::vector<Job> jobList;
std::map<int, std::vector<int>> jobHistory;
std::map<int, std::pair<int, int>> statsMap;

// This map stores the statistics for each job, where the key is the job ID and the value is a pair of total time and number of runs.
SchedulingAlgorithm currentAlgorithm = SchedulingAlgorithm::PRIORITY;


// Helper function to check if all dependencies are satisfied
bool AreDependenciesSatisfied(const Job& job, const std::vector<Job>& jobList) {
    for (int dep : job.dependencies) {
        auto it = std::find_if(jobList.begin(), jobList.end(),
            [dep](const Job& j) { return j.id == dep; });
        if (it == jobList.end() || !it->completed) {
            return false; // Dependency not found or not completed
        }
    }
    return true; // All dependencies are satisfied
}

std::vector<Job> GetJobsInScheduleOrder() {
    std::vector<Job> jobsToSchedule;//Initializes an empty list to hold jobs that are ready to be scheduled.



    for (auto& job : jobList) {
        if (!job.completed && AreDependenciesSatisfied(job, jobList)) {
            jobsToSchedule.push_back(job);
        }
    }

    switch (currentAlgorithm) {
    case SchedulingAlgorithm::FIFO:
        std::sort(jobsToSchedule.begin(), jobsToSchedule.end(),
            [](const Job& a, const Job& b) { return a.arrivalTime < b.arrivalTime; });//lambda
        break;
    case SchedulingAlgorithm::PRIORITY:
        std::sort(jobsToSchedule.begin(), jobsToSchedule.end(),
            [](const Job& a, const Job& b) { return a.priority > b.priority; });
        break;
    case SchedulingAlgorithm::DEADLINE:
        std::sort(jobsToSchedule.begin(), jobsToSchedule.end(),
            [](const Job& a, const Job& b) { return a.deadline < b.deadline; });
        break;
    }

    return jobsToSchedule;
}

bool ExecuteJob(Job& job) {
    const int MAX_RETRY = 3;

    // Ensure jobHistory and statsMap are initialized for the job
    if (jobHistory.find(job.id) == jobHistory.end()) {
        jobHistory[job.id] = {};
    }
    if (statsMap.find(job.id) == statsMap.end()) {
        statsMap[job.id] = {0, 0};
    }

    for (int attempt = 0; attempt <= MAX_RETRY; ++attempt) {
        Sleep(job.executionTime);

        // Ensure random number generation is seeded properly
        if ((rand() % 100) < 85) {
            job.completed = true;
            jobHistory[job.id].push_back(job.executionTime);
            statsMap[job.id].first += job.executionTime;
            statsMap[job.id].second++;
            return true;
        }
        job.retryCount++;
    }
    return false;
}

// Combined function to schedule and execute jobs
void ScheduleAndRunJobs() {
    std::vector<Job> jobsToRun;

    // f and sort jobs based on the current algorithm.

    for (auto& job : jobList) {
        if (!job.completed && AreDependenciesSatisfied(job, jobList)) {
            jobsToRun.push_back(job);
        }
    }

    switch (currentAlgorithm) {
    case SchedulingAlgorithm::FIFO:
        std::sort(jobsToRun.begin(), jobsToRun.end(),
            [](const Job& a, const Job& b) { return a.arrivalTime < b.arrivalTime; });
        break;
    case SchedulingAlgorithm::PRIORITY:
        std::sort(jobsToRun.begin(), jobsToRun.end(),
            [](const Job& a, const Job& b) { return a.priority > b.priority; });
        break;
    case SchedulingAlgorithm::DEADLINE:
        std::sort(jobsToRun.begin(), jobsToRun.end(),
            [](const Job& a, const Job& b) { return a.deadline < b.deadline; });
        break;
    }

             //  Execute Sorted Jobs 

    for (auto& jobRef : jobsToRun) {
        auto it = std::find_if(jobList.begin(), jobList.end(),
            [&jobRef](const Job& j) { return j.id == jobRef.id; });
        if (it != jobList.end()) {

            //Calling The function ExecuteJob On The Found Job If Ti Returns True The Job Completed Successfully.
            if (ExecuteJob(*it)) {
                std::cout << "[✓] Job " << it->id << " completed.\n";
            } else if (it->retryCount < 3) {
                std::cout << "[!] Job " << it->id << " failed, retrying...\n";
                ExecuteJob(*it);
            } else {
                std::cout << "[✗] Job " << it->id << " failed after retries.\n";
            }
        } else {
            std::cout << "[✗] Job " << jobRef.id << " not found in job list.\n";
        }
    }

    // Refresh job list
    std::cout << "\n                  JOB LIST\n";
    std::cout << "---------------------------------------------------\n"
              << " ID   | Priority | Exec Time | Deadline | Status \n"
              << "---------------------------------------------------\n";
    for (const auto& job : jobList) {
        std::cout << "Job " << job.id
                  << " |   P  : " << job.priority
                  << " |  T: " << job.executionTime << "ms"
                  << " | D: " << job.deadline << "ms"
                  << (job.completed ? " | Completed" : "") << "\n";
    }
    std::cout << "----------------------------------------------------\n\n\n";
}

//  function to print job history 
void PrintJobDetails() {
    std::cout << "\n                JOB HISTORY \n"
              << "--------------------------------------------------\n";

    if (jobHistory.empty()) {
        std::cout << "No job history available.\n";
    } else {
     //it loops through each entry in the container.
        for (const auto& entry : jobHistory) {
            std::cout << "  Job " << entry.first << " | ";
            for (int time : entry.second) {
                std::cout << time << "ms |";
            }
        }
        std::cout << "\n--------------------------------------------------\n";
    }

    std::cout << " \n                JOB STATISTICS \n"
              << "--------------------------------------------------\n";
    for (const auto& entry : statsMap) {
        std::cout << "Job " << entry.first
                  << " | Total Time: " << entry.second.first
                  << "ms | Runs: " << entry.second.second << "\n";
    }
    std::cout << "--------------------------------------------------\n\n\n";
}

// Add cleanup logic to release resources when the program exits
void CleanupResources() {
    jobList.clear(); // Clear the job list
    jobHistory.clear(); // Clear job history
    statsMap.clear(); // Clear statistics map
}

// Updated main function
int main() {
    srand((unsigned int)time(nullptr));

    // Add some jobs
    jobList.push_back(Job(1, 5, 100, 300));
    jobList.push_back(Job(2, 8, 200, 400));
    jobList.push_back(Job(3, 3, 150, 350, {1, 2}));
    jobList.push_back(Job(4, 9, 300, 500));

    ScheduleAndRunJobs();
    PrintJobDetails();

    // Call cleanup function before exiting
    CleanupResources();

    return 0;
}
