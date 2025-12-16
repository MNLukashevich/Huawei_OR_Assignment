#ifndef MILP_SOLVER_HPP
#define MILP_SOLVER_HPP

#include <vector>
#include <map>
#include <string>

// Solution structure for MILP model
struct MILPSolution {
    std::string status;                       // "optimal", "infeasible", "error", "time_limit"
    double makespan;                          // Optimal C_max value
    std::map<int, std::vector<int>> assignments;  // Job assignments per machine
    std::vector<int> machine_loads;           // Load of each machine
    double solve_time;                        // Solution time in seconds
    double gap;                               // Final optimality gap
    int iterations;                           // Number of iterations
    int nodes;                                // Number of explored nodes
    
    bool isValid() const { 
        return status == "optimal" || status == "feasible"; 
    }
    void printSummary() const;
};

class MILPSolver {
public:
    MILPSolver() = default;
    
    MILPSolution solve(const std::vector<int>& times, int m,
                   double time_limit, double mip_gap,
                   bool is_test_mode = false);
    
private:
    bool validateInput(const std::vector<int>& times, int m, 
                   std::string& error_msg, bool is_test_mode = false) const;
};

#endif