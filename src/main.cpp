#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <ctime>
#include "models/milp_solver.hpp"
#include "models/pseudo_polynomial.hpp"
#include "io/input_data.hpp"
#include "io/output_writer_json.hpp"

int main() {
    std::cout << "==========================================" << std::endl;
    std::cout << "  Huawei Job Scheduling - Both Models     " << std::endl;
    std::cout << "==========================================" << std::endl;
    
    // ============================================
    // GENERATE TIMESTAMP
    // ============================================
    auto now = std::chrono::system_clock::now();
    auto now_time = std::chrono::system_clock::to_time_t(now);
    std::tm* local_time = std::localtime(&now_time);
    
    char timestamp[20];
    std::strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", local_time);
    
    // ============================================
    // READ INPUT DATA
    // ============================================
    int n = InputData::get_number_of_jobs();
    int m = InputData::get_number_of_machines();
    std::vector<int> times = InputData::get_processing_times(n);
    InputData::MILPParameters milp_params = InputData::get_milp_parameters();
    
    // ============================================
    // DISPLAY INPUT DATA
    // ============================================
    std::cout << "\n=== Input Data ===" << std::endl;
    std::cout << "Number of jobs (n): " << n << std::endl;
    std::cout << "Number of machines (m): " << m << std::endl;
    std::cout << "MILP Time Limit: " << milp_params.time_limit << "s" << std::endl;
    std::cout << "MILP MIP Gap: " << milp_params.mip_gap << std::endl;
    
    // ============================================
    // SOLVE WITH PSEUDO-POLYNOMIAL ALGORITHM
    // ============================================
    std::cout << "\n=== Solving with Both Algorithms ===" << std::endl;
    
    std::cout << "1. Running Pseudo-Polynomial Algorithm..." << std::endl;
    PseudoPolynomialSolver pseudo_solver;
    PseudoPolySolution pseudo_solution = pseudo_solver.solve(times, m);
    
    if (pseudo_solution.isValid()) {
        std::cout << "   ✓ Pseudo-polynomial solution found" << std::endl;
        std::cout << "   Makespan: " << pseudo_solution.makespan << std::endl;
        std::cout << "   Solve time: " << pseudo_solution.solve_time << " seconds" << std::endl;
    } else {
        std::cout << "   ✗ Pseudo-polynomial failed: " << pseudo_solution.status << std::endl;
    }
    
    // ============================================
    // SOLVE WITH MILP ALGORITHM
    // ============================================
    std::cout << "\n2. Running MILP Algorithm..." << std::endl;
    MILPSolver milp_solver;
    MILPSolution milp_solution = milp_solver.solve(times, m, 
                                                   milp_params.time_limit, 
                                                   milp_params.mip_gap);
    
    if (milp_solution.isValid()) {
        std::cout << "   ✓ MILP solution found" << std::endl;
        std::cout << "   Makespan: " << milp_solution.makespan << std::endl;
        std::cout << "   Solve time: " << milp_solution.solve_time << " seconds" << std::endl;
        std::cout << "   Status: " << milp_solution.status << std::endl;
        std::cout << "   Gap: " << (milp_solution.gap * 100) << "%" << std::endl;
    } else {
        std::cout << "   ✗ MILP failed: " << milp_solution.status << std::endl;
    }
    
    // ============================================
    // CREATE TEST NAME AND RESULT
    // ============================================
    std::string test_name = "Production_Run_n" + std::to_string(n) + 
                           "_m" + std::to_string(m) + 
                           "_" + timestamp;
    
    auto test_result = OutputWriterJson::JsonWriter::createTestResult(
        test_name, n, m, times, -1.0, pseudo_solution, milp_solution);
    
    // ============================================
    // CREATE RESULTS DIRECTORY
    // ============================================
    system("mkdir -p results");
    
    // ============================================
    // WRITE INDIVIDUAL RESULT TO JSON FILE
    // ============================================
    std::string filename = "results/result_" + test_name + ".json";
    bool write_success = OutputWriterJson::JsonWriter::writeSingleResult(
        test_result, filename);
    
    // ============================================
    // DISPLAY COMPARISON
    // ============================================
    if (pseudo_solution.isValid() && milp_solution.isValid()) {
        double diff = std::abs(pseudo_solution.makespan - milp_solution.makespan);
        bool match = (diff < 0.001);
        
        std::cout << "\n=== Algorithm Comparison ===" << std::endl;
        std::cout << "Solutions match: " << (match ? "✓ Yes" : "✗ No") << std::endl;
        std::cout << "Makespan difference: " << diff << std::endl;
        
        if (pseudo_solution.solve_time > 0 && milp_solution.solve_time > 0) {
            double speedup = milp_solution.solve_time / pseudo_solution.solve_time;
            std::cout << "Speedup (MILP/Pseudo): " << speedup << "x" << std::endl;
        }
    }
    
    // ============================================
    // PROGRAM COMPLETION
    // ============================================
    std::cout << "\n==========================================" << std::endl;
    std::cout << "  Program completed successfully          " << std::endl;
    if (write_success) {
        std::cout << "  JSON results saved to:                " << std::endl;
        std::cout << "    - " << filename << std::endl;
    }
    std::cout << "==========================================" << std::endl;
    
    return 0;
}
