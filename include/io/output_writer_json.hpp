#ifndef OUTPUT_WRITER_JSON_HPP
#define OUTPUT_WRITER_JSON_HPP

#include <string>
#include <vector>
#include <map>
#include "models/milp_solver.hpp"
#include "models/pseudo_polynomial.hpp"

namespace OutputWriterJson {

    // ============================================
    // ALGORITHM RESULT STRUCTURE
    // ============================================
    struct AlgorithmResult {
        std::string algorithm_name;
        double makespan;
        double solution_time;
        std::string status;
        double gap;                     // only for MILP
        int feasibility_checks;         // only for pseudo-polynomial
        std::map<int, std::vector<int>> assignments;
        std::vector<int> machine_loads;
    };

    // ============================================
    // TEST RESULT STRUCTURE
    // ============================================
    struct TestResult {
        std::string test_name;
        int number_of_jobs;
        int number_of_machines;
        std::vector<int> processing_times;
        double expected_makespan;
        
        AlgorithmResult pseudo_polynomial_result;
        AlgorithmResult milp_result;
        
        bool solutions_match;
        double makespan_difference;
        double speedup;
    };

    class JsonWriter {
    public:
        // ============================================
        // SINGLE TEST RESULT METHODS
        // ============================================
        
        /**
         * @brief Write single test result to JSON file
         * @param result Test result data
         * @param filename Output JSON filename
         * @return true if successful, false otherwise
         */
        static bool writeSingleResult(const TestResult& result, 
                                     const std::string& filename);
        
        /**
         * @brief Create test result from algorithm solutions
         * @param name Test name
         * @param n Number of jobs
         * @param m Number of machines
         * @param times Processing times vector
         * @param expected_makespan Expected makespan (-1 if unknown)
         * @param pseudo_solution Pseudo-polynomial algorithm solution
         * @param milp_solution MILP algorithm solution
         * @return TestResult object
         */
        static TestResult createTestResult(
            const std::string& name,
            int n,
            int m,
            const std::vector<int>& times,
            double expected_makespan,
            const PseudoPolySolution& pseudo_solution,
            const MILPSolution& milp_solution
        );
        
        // ============================================
        // ALGORITHM RESULT CREATION METHODS (PUBLIC)
        // ============================================
        
        /**
         * @brief Create algorithm result from pseudo-polynomial solution
         * @param solution Pseudo-polynomial solution
         * @param times Processing times vector
         * @return AlgorithmResult object
         */
        static AlgorithmResult createPseudoPolyResult(
            const PseudoPolySolution& solution,
            const std::vector<int>& times
        );
        
        /**
         * @brief Create algorithm result from MILP solution
         * @param solution MILP solution
         * @param times Processing times vector
         * @return AlgorithmResult object
         */
        static AlgorithmResult createMILPResult(
            const MILPSolution& solution,
            const std::vector<int>& times
        );
        
        // ============================================
        // SIMPLIFIED OUTPUT METHODS
        // ============================================
        
        /**
         * @brief Write results to console and JSON file (simplified format)
         * @param test_name Name of the test
         * @param n Number of jobs
         * @param m Number of machines
         * @param times Processing times vector
         * @param pseudo_solution Pseudo-polynomial algorithm solution
         * @param milp_solution MILP algorithm solution
         * @param filename Output JSON filename
         * @return true if successful, false otherwise
         */
        static bool writeResultsToConsoleAndFile(
            const std::string& test_name,
            int n,
            int m,
            const std::vector<int>& times,
            const PseudoPolySolution& pseudo_solution,
            const MILPSolution& milp_solution,
            const std::string& filename
        );
        
        /**
         * @brief Quick summary method with explicit parameters
         * @param n Number of jobs
         * @param m Number of machines
         * @param makespan_milp MILP makespan value
         * @param makespan_pseudo Pseudo-polynomial makespan value
         * @param time_milp MILP solution time in seconds
         * @param time_pseudo Pseudo-polynomial solution time in seconds
         * @param milp_status MILP solver status string
         * @param milp_gap MILP optimality gap (0.0 to 1.0)
         * @param filename Output JSON filename
         * @return true if successful, false otherwise
         */
        static bool writeQuickSummary(
            int n,
            int m,
            double makespan_milp,
            double makespan_pseudo,
            double time_milp,
            double time_pseudo,
            const std::string& milp_status,
            double milp_gap,
            const std::string& filename
        );
        
    private:
        /**
         * @brief Check if solutions match within tolerance
         * @param pseudo_result Pseudo-polynomial algorithm result
         * @param milp_result MILP algorithm result
         * @param tolerance Tolerance for makespan comparison
         * @return true if solutions match, false otherwise
         */
        static bool checkSolutionsMatch(const AlgorithmResult& pseudo_result,
                                       const AlgorithmResult& milp_result,
                                       double tolerance = 0.001);
        
        /**
         * @brief Escape string for JSON
         * @param str Input string
         * @return Escaped string
         */
        static std::string escapeJsonString(const std::string& str);
    };

} // namespace OutputWriterJson

#endif // OUTPUT_WRITER_JSON_HPP