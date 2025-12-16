#include <iostream>
#include <iomanip>
#include <cmath>
#include <chrono>
#include "models/milp_solver.hpp"
#include "models/pseudo_polynomial.hpp"
#include "test_cases.hpp"

void TestCase::print() const {
    std::cout << "\nTest Case: " << name << std::endl;
    std::cout << "  n = " << times.size() << ", m = " << m << std::endl;
    std::cout << "  Times: ";
    for (size_t i = 0; i < times.size(); i++) {
        std::cout << times[i];
        if (i < times.size() - 1) std::cout << ", ";
    }
    std::cout << std::endl;
    std::cout << "  Expected makespan: " << expected_makespan << std::endl;
}

void print_solution(const std::string& method, 
                   double makespan, 
                   double solve_time,
                   const std::vector<int>& times,
                   const std::map<int, std::vector<int>>& assignments) {
    std::cout << "\n" << method << " Results:" << std::endl;
    std::cout << "Makespan: " << makespan << std::endl;
    std::cout << "Solve time: " << std::fixed << std::setprecision(6) 
              << solve_time << " seconds" << std::endl;
    
    std::cout << "Job Assignments:" << std::endl;
    for (const auto& [machine, jobs] : assignments) {
        std::cout << "  Machine " << machine << ": jobs [";
        
        // Print job indices
        for (size_t i = 0; i < jobs.size(); i++) {
            std::cout << jobs[i];
            if (i < jobs.size() - 1) std::cout << ", ";
        }
        std::cout << "], times [";
        
        // Print processing times and calculate total
        int total = 0;
        for (size_t i = 0; i < jobs.size(); i++) {
            int job_time = times[jobs[i]];
            std::cout << job_time;
            total += job_time;
            if (i < jobs.size() - 1) std::cout << ", ";
        }
        std::cout << "], total: " << total << std::endl;
    }
}

// Run a single test case
bool run_test_case(const TestCase& tc) {
    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << "RUNNING TEST: " << tc.name << std::endl;
    std::cout << std::string(70, '=') << std::endl;
    
    tc.print();
    
    bool test_passed = true;
    double milp_makespan = -1.0;
    double pseudo_makespan = -1.0;
    
    // ============================================
    // 1. PSEUDO-POLYNOMIAL ALGORITHM
    // ============================================
    std::cout << "\n1. PSEUDO-POLYNOMIAL ALGORITHM" << std::endl;
    try {
        PseudoPolynomialSolver pseudo_solver;
        PseudoPolySolution pseudo_sol = pseudo_solver.solve(tc.times, tc.m, true);
        
        if (pseudo_sol.isValid()) {
            // Convert partition to assignments
            std::map<int, std::vector<int>> pseudo_assignments;
            for (size_t i = 0; i < pseudo_sol.partition.size(); i++) {
                pseudo_assignments[i] = pseudo_sol.partition[i];
            }
            
            print_solution("Pseudo-polynomial", 
                          pseudo_sol.makespan,
                          pseudo_sol.solve_time,
                          tc.times,
                          pseudo_assignments);
            
            pseudo_makespan = pseudo_sol.makespan;
            
            // Check against expected value
            double diff = std::abs(pseudo_sol.makespan - tc.expected_makespan);
            if (diff == 0.0) {
                std::cout << "✓ PASS: Makespan matches expected value" << std::endl;
            } else {
                std::cout << "✗ FAIL: Makespan differs from expected value" << std::endl;
                std::cout << "  Expected: " << tc.expected_makespan 
                          << ", Got: " << pseudo_sol.makespan 
                          << ", Diff: " << diff << std::endl;
                test_passed = false;
            }
        } else {
            std::cout << "ERROR: " << pseudo_sol.status << std::endl;
            test_passed = false;
        }
    } catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << std::endl;
        test_passed = false;
    }
    
    // ============================================
    // 2. MILP SOLUTION (TEST MODE)
    // ============================================
    if (tc.times.size() <= 200) {
        std::cout << "\n2. MILP SOLUTION (CPLEX)" << std::endl;
        try {
            MILPSolver milp_solver;
            MILPSolution milp_sol = milp_solver.solve(tc.times, tc.m, 30.0, 0.0, true);
            
            if (milp_sol.isValid()) {
                print_solution("MILP",
                              milp_sol.makespan,
                              milp_sol.solve_time,
                              tc.times,
                              milp_sol.assignments);
                
                milp_makespan = milp_sol.makespan;
                
                // Compare with pseudo-polynomial solution
                if (pseudo_makespan >= 0) {
                    double diff = std::abs(milp_sol.makespan - pseudo_makespan);
                    std::cout << "\nComparison:" << std::endl;
                    std::cout << "  Pseudo makespan: " << pseudo_makespan << std::endl;
                    std::cout << "  MILP makespan:   " << milp_sol.makespan << std::endl;
                    std::cout << "  Difference:      " << diff << std::endl;
                    
                    if (diff < 0.001) {
                        std::cout << "✓ PASS: Both algorithms give same result" << std::endl;
                    } else {
                        std::cout << "✗ FAIL: Algorithms give different results" << std::endl;
                        test_passed = false;
                    }
                    
                    // Compare solve times
                    std::cout << "\nPerformance Comparison:" << std::endl;
                    std::cout << "  Pseudo time: " << pseudo_makespan << " seconds" << std::endl;
                    std::cout << "  MILP time:   " << milp_sol.solve_time << " seconds" << std::endl;
                    if (milp_sol.solve_time > 0 && pseudo_makespan > 0) {
                        double speedup = milp_sol.solve_time / pseudo_makespan;
                        std::cout << "  Speedup:     " << speedup << "x (pseudo is faster)" << std::endl;
                    }
                }
            } else {
                std::cout << "MILP Status: " << milp_sol.status << std::endl;
                test_passed = false;
            }
        } catch (const std::exception& e) {
            std::cout << "MILP Exception: " << e.what() << std::endl;
            test_passed = false;
        }
    } else {
        std::cout << "\n2. MILP SOLUTION: Skipped (n=" << tc.times.size() << " > 200)" << std::endl;
    }
    
    std::cout << std::string(70, '=') << std::endl;
    return test_passed;
}

// Main test runner
int main() {
    std::cout << "======================================================" << std::endl;
    std::cout << "  Huawei Job Scheduling - Test Runner                " << std::endl;
    std::cout << "======================================================" << std::endl;
    
    // Get test cases
    auto test_cases = get_validation_test_cases();
    
    std::cout << "\nFound " << test_cases.size() << " test cases." << std::endl;
    
    int passed = 0;
    int failed = 0;
    
    // Run all test cases
    for (const auto& tc : test_cases) {
        if (run_test_case(tc)) {
            passed++;
        } else {
            failed++;
        }
    }
    
    // Summary
    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << "TEST SUMMARY" << std::endl;
    std::cout << std::string(70, '=') << std::endl;
    
    std::cout << "Total tests: " << test_cases.size() << std::endl;
    std::cout << "Passed:      " << passed << std::endl;
    std::cout << "Failed:      " << failed << std::endl;
    
    if (failed == 0) {
        std::cout << "\n✓ All tests passed successfully!" << std::endl;
    } else {
        std::cout << "\n✗ Some tests failed." << std::endl;
    }
    
    return (failed == 0) ? 0 : 1;
}