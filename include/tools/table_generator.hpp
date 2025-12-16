#ifndef TABLE_GENERATOR_HPP
#define TABLE_GENERATOR_HPP

#include <string>
#include <vector>
#include <map>
#include <utility>

struct ResultData {
    int n;                      // number of jobs
    int m;                      // number of machines
    double ms_milp;            // makespan MILP
    double ms_pseudo;          // makespan pseudo-polynomial
    double delta;              // difference (ms_milp - ms_pseudo)
    double cpu_time;           // CPU time for MILP
    double gap;                // optimality gap
    double speedup;            // speedup (milp_time / pseudo_time)
    bool solutions_match;      // whether solutions match
    std::string status_milp;   // status of MILP solution
    std::string status_pseudo; // status of pseudo solution
};

class TableGenerator {
public:
    // Read all JSON results from directory
    static std::vector<ResultData> readResultsFromDirectory(const std::string& directory_path);
    
    // Generate LaTeX table
    static void generateLatexTable(const std::vector<ResultData>& results, 
                                   const std::string& output_file);
    
    // Generate statistics summary
    static void generateStatistics(const std::vector<ResultData>& results,
                                   const std::string& output_file);
    
    // Simple console output
    static void printConsoleTable(const std::vector<ResultData>& results);
    
private:
    // Private helper methods for JSON parsing
    static double extractDouble(const std::string& json, const std::string& key);
    static int extractInt(const std::string& json, const std::string& key);
    static bool extractBool(const std::string& json, const std::string& key);
    static std::string extractString(const std::string& json, const std::string& key);
    static double extractDoubleInSection(const std::string& json, const std::string& section_path);
    static std::string extractStringInSection(const std::string& json, const std::string& section_path);
};

#endif // TABLE_GENERATOR_HPP