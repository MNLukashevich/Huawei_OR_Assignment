#include "io/output_writer_json.hpp"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <cmath>

namespace OutputWriterJson {

    // ============================================
    // PRIVATE HELPER METHODS
    // ============================================
    
    bool JsonWriter::checkSolutionsMatch(const AlgorithmResult& pseudo_result,
                                        const AlgorithmResult& milp_result,
                                        double tolerance) {
        if (pseudo_result.makespan < 0 || milp_result.makespan < 0) {
            return false;
        }
        return std::abs(pseudo_result.makespan - milp_result.makespan) < tolerance;
    }
    
    std::string JsonWriter::escapeJsonString(const std::string& str) {
        std::string result;
        for (char c : str) {
            switch (c) {
                case '"': result += "\\\""; break;
                case '\\': result += "\\\\"; break;
                case '\b': result += "\\b"; break;
                case '\f': result += "\\f"; break;
                case '\n': result += "\\n"; break;
                case '\r': result += "\\r"; break;
                case '\t': result += "\\t"; break;
                default: result += c;
            }
        }
        return result;
    }

    // ============================================
    // ALGORITHM RESULT CREATION METHODS (PUBLIC)
    // ============================================
    AlgorithmResult JsonWriter::createPseudoPolyResult(
        const PseudoPolySolution& solution,
        const std::vector<int>& times) {
        
        AlgorithmResult result;
        result.algorithm_name = "pseudo_polynomial";
        
        if (solution.isValid()) {
            result.makespan = solution.makespan;
            result.solution_time = solution.solve_time;
            result.status = solution.status;
            result.feasibility_checks = solution.feasibility_checks;
            result.gap = 0.0;
            
            // Convert partition to assignments
            for (size_t i = 0; i < solution.partition.size(); i++) {
                result.assignments[static_cast<int>(i)] = solution.partition[i];
                
                // Calculate machine load
                int load = 0;
                for (int job_idx : solution.partition[i]) {
                    if (job_idx >= 0 && job_idx < static_cast<int>(times.size())) {
                        load += times[job_idx];
                    }
                }
                result.machine_loads.push_back(load);
            }
        } else {
            result.makespan = -1.0;
            result.solution_time = -1.0;
            result.status = solution.status;
            result.feasibility_checks = 0;
            result.gap = -1.0;
        }
        
        return result;
    }

    AlgorithmResult JsonWriter::createMILPResult(
    const MILPSolution& solution,
    const std::vector<int>& times) {
    
    AlgorithmResult result;
    result.algorithm_name = "milp";
    
    if (solution.status == "optimal" || solution.status == "feasible") { 
    
        result.makespan = solution.makespan;
        result.solution_time = solution.solve_time;
        result.status = solution.status;
        result.gap = solution.gap;
        result.feasibility_checks = 0;
        result.assignments = solution.assignments;
        result.machine_loads = solution.machine_loads;
    } else {
        result.makespan = -1.0;
        result.solution_time = -1.0;
        result.status = solution.status;
        result.gap = -1.0;
        result.feasibility_checks = 0;
    }
    
    return result;
}

    // ============================================
    // TEST RESULT CREATION
    // ============================================
    TestResult JsonWriter::createTestResult(
        const std::string& name,
        int n,
        int m,
        const std::vector<int>& times,
        double expected_makespan,
        const PseudoPolySolution& pseudo_solution,
        const MILPSolution& milp_solution) {
        
        TestResult result;
        result.test_name = name;
        result.number_of_jobs = n;
        result.number_of_machines = m;
        result.processing_times = times;
        result.expected_makespan = expected_makespan;
        
        // Create algorithm results
        result.pseudo_polynomial_result = JsonWriter::createPseudoPolyResult(pseudo_solution, times);
        result.milp_result = JsonWriter::createMILPResult(milp_solution, times);
        
        // Check if solutions match
        result.solutions_match = JsonWriter::checkSolutionsMatch(result.pseudo_polynomial_result, 
                                                                result.milp_result);
        
        // Calculate makespan difference
        if (result.pseudo_polynomial_result.makespan >= 0 && 
            result.milp_result.makespan >= 0) {
            result.makespan_difference = std::abs(
                result.pseudo_polynomial_result.makespan - 
                result.milp_result.makespan
            );
        } else {
            result.makespan_difference = -1.0;
        }
        
        // Calculate speedup
        if (result.pseudo_polynomial_result.solution_time > 0 && 
            result.milp_result.solution_time > 0) {
            result.speedup = result.milp_result.solution_time / 
                            result.pseudo_polynomial_result.solution_time;
        } else {
            result.speedup = 0.0;
        }
        
        return result;
    }

    // ============================================
    // SINGLE RESULT JSON WRITING
    // ============================================
    bool JsonWriter::writeSingleResult(const TestResult& result, 
                                      const std::string& filename) {
        try {
            std::ofstream file(filename);
            if (!file.is_open()) {
                std::cerr << "Error: Could not open file " << filename << " for writing" << std::endl;
                return false;
            }
            
            // Create JSON string
            std::stringstream json;
            
            json << "{\n";
            json << "  \"test_name\": \"" << escapeJsonString(result.test_name) << "\",\n";
            json << "  \"problem_parameters\": {\n";
            json << "    \"number_of_jobs\": " << result.number_of_jobs << ",\n";
            json << "    \"number_of_machines\": " << result.number_of_machines << ",\n";
            json << "    \"expected_makespan\": " << result.expected_makespan << "\n";
            json << "  },\n";
            json << "  \"algorithms\": {\n";
            json << "    \"pseudo_polynomial\": {\n";
            json << "      \"makespan\": " << result.pseudo_polynomial_result.makespan << ",\n";
            json << "      \"solution_time\": " << result.pseudo_polynomial_result.solution_time << ",\n";
            json << "      \"status\": \"" << escapeJsonString(result.pseudo_polynomial_result.status) << "\",\n";
            json << "      \"feasibility_checks\": " << result.pseudo_polynomial_result.feasibility_checks << "\n";
            json << "    },\n";
            json << "    \"milp\": {\n";
            json << "      \"makespan\": " << result.milp_result.makespan << ",\n";
            json << "      \"solution_time\": " << result.milp_result.solution_time << ",\n";
            json << "      \"status\": \"" << escapeJsonString(result.milp_result.status) << "\",\n";
            json << "      \"gap\": " << result.milp_result.gap << "\n";
            json << "    }\n";
            json << "  },\n";
            json << "  \"comparison\": {\n";
            json << "    \"solutions_match\": " << (result.solutions_match ? "true" : "false") << ",\n";
            json << "    \"makespan_difference\": " << result.makespan_difference << ",\n";
            json << "    \"speedup\": " << result.speedup << "\n";
            json << "  }\n";
            json << "}";
            
            file << json.str();
            file.close();
            
            std::cout << "✓ Results written to JSON file: " << filename << std::endl;
            return true;
            
        } catch (const std::exception& e) {
            std::cerr << "Error writing JSON: " << e.what() << std::endl;
            return false;
        }
    }

    // ============================================
    // SIMPLIFIED OUTPUT METHODS
    // ============================================
    bool JsonWriter::writeResultsToConsoleAndFile(
        const std::string& test_name,
        int n,
        int m,
        const std::vector<int>& times,
        const PseudoPolySolution& pseudo_solution,
        const MILPSolution& milp_solution,
        const std::string& filename) {
        
        try {
            // Extract data from solutions

            double makespan_milp = milp_solution.isValid() ? milp_solution.makespan : -1.0;
            double makespan_pseudo = pseudo_solution.isValid() ? pseudo_solution.makespan : -1.0;
            
            double time_milp = milp_solution.isValid() ? milp_solution.solve_time : -1.0;
            double time_pseudo = pseudo_solution.isValid() ? pseudo_solution.solve_time : -1.0;
            
            std::string milp_status = milp_solution.status;
            double milp_gap = milp_solution.isValid() ? milp_solution.gap : -1.0;
            
            // Call the quick summary method
            return writeQuickSummary(n, m, makespan_milp, makespan_pseudo, 
                                    time_milp, time_pseudo, milp_status, milp_gap, filename);
            
        } catch (const std::exception& e) {
            std::cerr << "Error in writeResultsToConsoleAndFile: " << e.what() << std::endl;
            return false;
        }
    }

    bool JsonWriter::writeQuickSummary(
        int n,
        int m,
        double makespan_milp,
        double makespan_pseudo,
        double time_milp,
        double time_pseudo,
        const std::string& milp_status,
        double milp_gap,
        const std::string& filename) {
        
        try {
            // ============================================
            // CONSOLE OUTPUT
            // ============================================
            std::cout << "\n" << std::string(60, '=') << "\n";
            std::cout << "TEST RESULTS SUMMARY\n";
            std::cout << std::string(60, '=') << "\n";
            
            // Problem parameters
            std::cout << std::left << std::setw(25) << "Number of jobs:" 
                      << std::right << std::setw(10) << n << "\n";
            std::cout << std::left << std::setw(25) << "Number of machines:" 
                      << std::right << std::setw(10) << m << "\n\n";
            
            // Makespan comparison
            std::cout << "--- MAKESPAN COMPARISON ---\n";
            std::cout << std::left << std::setw(25) << "MILP Makespan:" 
                      << std::right << std::setw(10) << std::fixed << std::setprecision(2) 
                      << makespan_milp << "\n";
            std::cout << std::left << std::setw(25) << "Pseudo Makespan:" 
                      << std::right << std::setw(10) << std::fixed << std::setprecision(2) 
                      << makespan_pseudo << "\n";
            
            double difference = makespan_milp - makespan_pseudo;
            std::cout << std::left << std::setw(25) << "Difference:" 
                      << std::right << std::setw(10) << std::fixed << std::setprecision(2) 
                      << difference;
            
            if (difference > 0) {
                std::cout << "  (Pseudo-polynomial is better)";
            } else if (difference < 0) {
                std::cout << "  (MILP is better)";
            } else {
                std::cout << "  (Solutions match)";
            }
            std::cout << "\n\n";
            
            // Solution times
            std::cout << "--- SOLUTION TIME (seconds) ---\n";
            std::cout << std::left << std::setw(25) << "MILP Time:" 
                      << std::right << std::setw(10) << std::scientific << std::setprecision(2) 
                      << time_milp << "\n";
            std::cout << std::left << std::setw(25) << "Pseudo Time:" 
                      << std::right << std::setw(10) << std::scientific << std::setprecision(2) 
                      << time_pseudo << "\n";
            
            // Speedup calculation
            if (time_pseudo > 0 && time_milp > 0) {
                double speedup = time_milp / time_pseudo;
                std::cout << std::left << std::setw(25) << "Speedup (MILP/Pseudo):" 
                          << std::right << std::setw(10) << std::fixed << std::setprecision(1) 
                          << speedup << "x\n";
            }
            std::cout << "\n";
            
            // MILP specific information
            std::cout << "--- MILP SPECIFIC ---\n";
            std::cout << std::left << std::setw(25) << "MILP Status:" 
                      << std::right << std::setw(10) << milp_status << "\n";
            if (milp_gap >= 0) {
                std::cout << std::left << std::setw(25) << "MILP Gap (%):" 
                          << std::right << std::setw(10) << std::fixed << std::setprecision(2) 
                          << (milp_gap * 100) << "%\n";
            }
            
            std::cout << std::string(60, '=') << "\n\n";
            
            // ============================================
            // SIMPLE JSON FILE OUTPUT
            // ============================================
            try {
                std::ofstream file(filename);
                if (!file.is_open()) {
                    std::cerr << "Error: Could not open file " << filename << " for writing" << std::endl;
                    return false;
                }
                
                std::stringstream json;
                json << "{\n";
                json << "  \"number_of_jobs\": " << n << ",\n";
                json << "  \"number_of_machines\": " << m << ",\n";
                json << "  \"makespan_milp\": " << makespan_milp << ",\n";
                json << "  \"makespan_pseudo\": " << makespan_pseudo << ",\n";
                json << "  \"difference\": " << difference << ",\n";
                json << "  \"solution_time_milp\": " << time_milp << ",\n";
                json << "  \"solution_time_pseudo\": " << time_pseudo << ",\n";
                json << "  \"milp_status\": \"" << escapeJsonString(milp_status) << "\",\n";
                json << "  \"milp_gap\": " << milp_gap << "\n";
                json << "}";
                
                file << json.str();
                file.close();
                
                std::cout << "✓ Quick results saved to JSON file: " << filename << "\n";
                
            } catch (const std::exception& e) {
                std::cerr << "Error writing JSON file: " << e.what() << std::endl;
                return false;
            }
            
            return true;
            
        } catch (const std::exception& e) {
            std::cerr << "Error writing quick summary: " << e.what() << std::endl;
            return false;
        }
    }

} // namespace OutputWriterJson