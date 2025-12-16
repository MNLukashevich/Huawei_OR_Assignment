#include "models/pseudo_polynomial.hpp"
#include <chrono>
#include <algorithm>
#include <numeric>
#include <iostream>

using namespace std;
using namespace std::chrono;

bool validateInput(const std::vector<int>& times, int m, 
                   std::string& error_msg, bool is_test_mode = false) {
    int n = times.size();
    
    if (!is_test_mode) {
        // Production mode: n must be in [50, 5000]
        if (n < 50 || n > 5000) {
            error_msg = "n must be in range [50, 5000] (n=" + std::to_string(n) + ")";
            return false;
        }
    } 
    
    // Check n > m condition
    if (n <= m) {
        error_msg = "n must be greater than m (n=" + std::to_string(n) + 
                   ", m=" + std::to_string(m) + ")";
        return false;
    }
    
    // Check processing times are in range [1, 24]
    for (size_t i = 0; i < times.size(); i++) {
        if (times[i] < 1 || times[i] > 24) {
            error_msg = "Processing time at index " + std::to_string(i) + 
                       " is " + std::to_string(times[i]) + 
                       ", must be in range [1, 24]";
            return false;
        }
    }
    
    return true;
}

PseudoPolySolution PseudoPolynomialSolver::solve(const std::vector<int>& times, int m, bool is_test_mode) {
    PseudoPolySolution result;
    auto start_time = high_resolution_clock::now();
    
    // Validate input with test mode flag
    std::string error_msg;
    if (!validateInput(times, m, error_msg, is_test_mode)) {
        result.status = "invalid_input";
        result.solve_time = 0.0;
        std::cerr << "Pseudo-polynomial input error: " << error_msg << std::endl;
        return result;
    }
    
    // Initialize counters
    result.feasibility_checks = 0;
    
    try {
        // Find optimal makespan using binary search
        result.makespan = findOptimalMakespan(times, m, result.feasibility_checks);
        
        // Reconstruct partition
        result.partition = reconstructPartition(times, m, result.makespan);
        
        // Calculate machine loads
        result.machine_loads.clear();
        for (const auto& block : result.partition) {
            int load = 0;
            for (int job_idx : block) {
                load += times[job_idx];
            }
            result.machine_loads.push_back(load);
        }
        
        // Calculate solve time
        auto end_time = high_resolution_clock::now();
        result.solve_time = duration<double>(end_time - start_time).count();
        
        // Set status
        result.status = "optimal";
        
    } catch (const exception& e) {
        result.status = string("error: ") + e.what();
        auto end_time = high_resolution_clock::now();
        result.solve_time = duration<double>(end_time - start_time).count();
    }
    
    return result;
}

bool PseudoPolynomialSolver::isFeasible(int T, const std::vector<int>& times, int m) {
    int blocks_needed = 1;
    int current_sum = 0;
    
    for (int t : times) {
        if (t > T) return false;
        
        if (current_sum + t <= T) {
            current_sum += t;
        } else {
            blocks_needed++;
            current_sum = t;
            if (blocks_needed > m) return false;
        }
    }
    return true;
}

int PseudoPolynomialSolver::findOptimalMakespan(const std::vector<int>& times, int m, int& feasibility_checks) {
    // Lower bound: maximum processing time
    int left = *max_element(times.begin(), times.end());
    
    // Upper bound: sum of all processing times
    int right = accumulate(times.begin(), times.end(), 0);
    
    feasibility_checks = 0;
    
    // Binary search
    while (left < right) {
        int mid = left + (right - left) / 2;
        feasibility_checks++;
        
        if (isFeasible(mid, times, m)) {
            right = mid;  // Can try smaller makespan
        } else {
            left = mid + 1;  // Need larger makespan
        }
    }
    
    return left;
}

std::vector<std::vector<int>> PseudoPolynomialSolver::reconstructPartition(
    const std::vector<int>& times, int m, int T_opt) {
    
    vector<vector<int>> partition;
    vector<int> current_block;
    int current_sum = 0;
    
    for (int i = 0; i < times.size(); i++) {
        if (current_sum + times[i] <= T_opt) {
            current_block.push_back(i);
            current_sum += times[i];
        } else {
            partition.push_back(current_block);
            current_block.clear();
            current_block.push_back(i);
            current_sum = times[i];
        }
    }
    
    // Add the last block
    if (!current_block.empty()) {
        partition.push_back(current_block);
    }
    
    return partition;
}