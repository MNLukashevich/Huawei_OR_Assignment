#include "models/milp_solver.hpp"
#include <ilcplex/ilocplex.h>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <numeric>

ILOSTLBEGIN

// Print summary of MILP solution
void MILPSolution::printSummary() const {
    std::cout << "\n=== MILP Solution Summary ===" << std::endl;
    std::cout << "Status: " << status << std::endl;
    
    if (isValid()) {
        std::cout << "Makespan (C_max): " << makespan << std::endl;
        std::cout << "Solve Time: " << solve_time << " seconds" << std::endl;
        std::cout << "Optimality Gap: " << (gap * 100) << "%" << std::endl;
        std::cout << "Iterations: " << iterations << std::endl;
        std::cout << "Nodes Explored: " << nodes << std::endl;
        
        std::cout << "\nJob Assignments:" << std::endl;
        // Исправляем для C++11
        for (const auto& assignment_pair : assignments) {
            int machine = assignment_pair.first;
            const std::vector<int>& jobs = assignment_pair.second;
            std::cout << "  Machine " << machine << ": Jobs [";
            for (size_t i = 0; i < jobs.size(); i++) {
                std::cout << jobs[i];
                if (i < jobs.size() - 1) std::cout << ", ";
            }
            std::cout << "]" << std::endl;
        }
        
        std::cout << "\nMachine Loads:" << std::endl;
        for (size_t i = 0; i < machine_loads.size(); i++) {
            std::cout << "  Machine " << i << ": " << machine_loads[i] << std::endl;
        }
    }
}

bool MILPSolver::validateInput(const std::vector<int>& times, int m, 
                               std::string& error_msg, bool is_test_mode) const {
    int n = times.size();
    
    if (!is_test_mode) {
        if (n < 50 || n > 5000) {
            error_msg = "n must be in range [50, 5000] (n=" + std::to_string(n) + ")";
            return false;
        }
    }
    
    if (n <= m) {
        error_msg = "n must be greater than m (n=" + std::to_string(n) + 
                   ", m=" + std::to_string(m) + ")";
        return false;
    }
    
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

// Main solve method
MILPSolution MILPSolver::solve(const std::vector<int>& times, int m,
                               double time_limit, double mip_gap, bool is_test_mode) {
    MILPSolution result;
    
    // Start timing
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Validate input with test mode flag
    std::string error_msg;
    if (!validateInput(times, m, error_msg, is_test_mode)) {
        result.status = "invalid_input";
        result.solve_time = 0.0;
        std::cerr << "Input validation error: " << error_msg << std::endl;
        return result;
    }
    
    int n = times.size();
    
    try {
        // ============================================
        // CREATE CPLEX ENVIRONMENT AND MODEL
        // ============================================
        IloEnv env;
        IloModel model(env);
        
        // ============================================
        // DECISION VARIABLES
        // ============================================
        
        // x[i][j] = 1 if job i is assigned to machine j
        IloArray<IloNumVarArray> x(env, n);
        for (int i = 0; i < n; i++) {
            x[i] = IloNumVarArray(env, m, 0, 1, ILOINT);
            for (int j = 0; j < m; j++) {
                std::ostringstream oss;
                oss << "x_" << i << "_" << j;
                x[i][j].setName(oss.str().c_str());
            }
        }
        
        // y[i] = machine index for job i (1..m)
        IloNumVarArray y(env, n, 1, m, ILOINT);
        for (int i = 0; i < n; i++) {
            std::ostringstream oss;
            oss << "y_" << i;
            y[i].setName(oss.str().c_str());
        }
        
        // C[j] = load of machine j
        IloNumVarArray C(env, m, 0, IloInfinity, ILOFLOAT);
        for (int j = 0; j < m; j++) {
            std::ostringstream oss;
            oss << "C_" << j;
            C[j].setName(oss.str().c_str());
        }
        
        // C_max = makespan (maximum machine load)
        IloNumVar C_max(env, 0, IloInfinity, ILOFLOAT, "C_max");
        
        // ============================================
        // CONSTRAINTS
        // ============================================
        
        // (1) Job assignment
        for (int i = 0; i < n; i++) {
            IloExpr sum_x(env);
            for (int j = 0; j < m; j++) {
                sum_x += x[i][j];
            }
            model.add(sum_x == 1);
            sum_x.end();
        }

        // (2) Load definition
        for (int j = 0; j < m; j++) {
            IloExpr load_expr(env);
            for (int i = 0; i < n; i++) {
                load_expr += times[i] * x[i][j];
            }
            model.add(C[j] == load_expr);
            load_expr.end();
        }

        // (3) Makespan definition
        for (int j = 0; j < m; j++) {
            model.add(C_max >= C[j]);
        }
        
        // (4) Consistency between x and y
        for (int i = 0; i < n; i++) {
            IloExpr y_expr(env);
            for (int j = 0; j < m; j++) {
                y_expr += (j + 1) * x[i][j];  // +1 because machines are numbered from 1
            }
            model.add(y[i] == y_expr);
            y_expr.end();
        }
        
        // (5) Contiguity (monotonicity)
        for (int i = 0; i < n - 1; i++) {
            model.add(y[i] <= y[i + 1]);
        }
        
        // (6) Anchoring
        // First job must be on first machine
        model.add(x[0][0] == 1);
        // Last job must be on last machine
        model.add(x[n - 1][m - 1] == 1);
        
        // (8) Lower Bound on Makespan
        int max_time = *std::max_element(times.begin(), times.end());
        model.add(C_max >= max_time);
        // (9) Lower Bound Based on Average Load
        int total_time = std::accumulate(times.begin(), times.end(), 0);
        model.add(C_max >= static_cast<double>(total_time) / m);
        
        // ============================================
        // OBJECTIVE FUNCTION
        // ============================================
        model.add(IloMinimize(env, C_max));
        
        // ============================================
        // SOLVE WITH CPLEX
        // ============================================
        IloCplex cplex(model);
        
        // Set CPLEX parameters
        cplex.setParam(IloCplex::TiLim, time_limit);
        cplex.setParam(IloCplex::EpGap, mip_gap);
        // Maximum number of threads to use
        cplex.setParam(IloCplex::Threads, 4);

        cplex.solve();
        
        // Calculate solve time
        auto end_time = std::chrono::high_resolution_clock::now();
        result.solve_time = std::chrono::duration<double>(end_time - start_time).count();
        
        // ============================================
        // DETERMINE SOLUTION STATUS
        // ============================================
        IloAlgorithm::Status cplex_status = cplex.getStatus();

        if (cplex_status == IloAlgorithm::Optimal) {
            result.gap = cplex.getMIPRelativeGap();
            result.status = "optimal";
        }
        else if (cplex_status == IloAlgorithm::Feasible) {
            result.gap = cplex.getMIPRelativeGap();
            result.status = "feasible";
        }
        else if (cplex_status == IloAlgorithm::InfeasibleOrUnbounded) {
            result.gap = -1.0;
            result.status = "infeasible_or_unbounded";
            env.end();
            return result;
        }
        else {
            result.status = "unknown";
            result.gap = -1.0;
            env.end();
            return result;
        }

        // Get the makespan value
        double makespan_value = cplex.getValue(C_max);
        
        // Round to integer if close (epsilon = 1e-6)
        const double epsilon = 1e-6;
        if (std::abs(makespan_value - std::round(makespan_value)) < epsilon) {
            makespan_value = std::round(makespan_value);
        }
        result.makespan = makespan_value;
        
        result.iterations = cplex.getNiterations();
        result.nodes = cplex.getNnodes();
        
        // Extract job assignments
        for (int j = 0; j < m; j++) {
            std::vector<int> machine_jobs;
            int machine_load = 0;
            
            for (int i = 0; i < n; i++) {
                if (cplex.getValue(x[i][j]) > 0.5) {  // Threshold for binary variable
                    machine_jobs.push_back(i);
                    machine_load += times[i];
                }
            }
            
            if (!machine_jobs.empty()) {
                result.assignments[j] = machine_jobs;
                result.machine_loads.push_back(machine_load);
            }
        }
        
        // Clean up
        env.end();
        
    } catch (IloException& e) {
        auto end_time = std::chrono::high_resolution_clock::now();
        result.solve_time = std::chrono::duration<double>(end_time - start_time).count();
        result.status = "cplex_error";
        std::cerr << "CPLEX Exception: " << e.getMessage() << std::endl;
    } catch (const std::exception& e) {
        auto end_time = std::chrono::high_resolution_clock::now();
        result.solve_time = std::chrono::duration<double>(end_time - start_time).count();
        result.status = "std_error";
        std::cerr << "Standard Exception: " << e.what() << std::endl;
    } catch (...) {
        auto end_time = std::chrono::high_resolution_clock::now();
        result.solve_time = std::chrono::duration<double>(end_time - start_time).count();
        result.status = "unknown_error";
        std::cerr << "Unknown Error" << std::endl;
    }
    
    return result;
}