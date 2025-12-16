#include "tools/table_generator.hpp"
#include <iostream>
#include <string>

int main() {
    std::string results_dir = "results";
    std::string latex_output = "results/results_table.tex";
    std::string summary_output = "results/results_summary.txt";
    
    std::cout << "==========================================" << std::endl;
    std::cout << "  STANDALONE TABLE GENERATOR" << std::endl;
    std::cout << "==========================================" << std::endl;
    
    std::cout << "\n[1/3] Reading JSON files from: " << results_dir << std::endl;
    
    try {
        auto results = TableGenerator::readResultsFromDirectory(results_dir);
        
        if (results.empty()) {
            std::cout << "✗ No JSON files found in " << results_dir << std::endl;
            std::cout << "  Run the main program first: make run" << std::endl;
            return 1;
        }
        
        std::cout << "✓ Found " << results.size() << " result files" << std::endl;
        
        std::cout << "\n[2/3] Generating console table..." << std::endl;
        TableGenerator::printConsoleTable(results);
        
        std::cout << "\n[3/3] Generating output files..." << std::endl;
        TableGenerator::generateLatexTable(results, latex_output);
        TableGenerator::generateStatistics(results, summary_output);
        
        std::cout << "\n==========================================" << std::endl;
        std::cout << "  SUCCESS! Files created:" << std::endl;
        std::cout << "==========================================" << std::endl;
        std::cout << "  ✓ " << latex_output << " (LaTeX table)" << std::endl;
        std::cout << "  ✓ " << summary_output << " (statistics)" << std::endl;
        std::cout << "\nTo use in LaTeX:" << std::endl;
        std::cout << "  1. Copy results_table.tex content" << std::endl;
        std::cout << "  2. Paste into your paper" << std::endl;
        std::cout << "  3. Compile with pdflatex" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "\n✗ ERROR: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}