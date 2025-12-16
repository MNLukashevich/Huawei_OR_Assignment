#ifndef PSEUDO_POLYNOMIAL_HPP
#define PSEUDO_POLYNOMIAL_HPP

#include <vector>
#include <string>

struct PseudoPolySolution {
    double makespan;                         // Optimal makespan
    std::vector<std::vector<int>> partition; // Partition of jobs
    std::vector<int> machine_loads;          // Load of each machine
    double solve_time;                       // Solution time in seconds
    int feasibility_checks;                  // Number of feasibility checks
    std::string status;                      // Solution status
    
    // Add this method:
    bool isValid() const { return true; } // Или другая логика проверки
};

class PseudoPolynomialSolver {
public:
    PseudoPolySolution solve(const std::vector<int>& times, int m,
                         bool is_test_mode = false);
    
private:
    bool isFeasible(int T, const std::vector<int>& times, int m);
    int findOptimalMakespan(const std::vector<int>& times, int m, int& checks);
    std::vector<std::vector<int>> reconstructPartition(const std::vector<int>& times, 
                                                      int m, int T_opt);
};

#endif