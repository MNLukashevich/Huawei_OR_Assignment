#include "io/input_data.hpp"
#include <iostream>
#include <random>

namespace InputData {

    // ============================================
    // PROBLEM DATA IMPLEMENTATIONS
    // ============================================
    
    int get_number_of_jobs() {
        return 50;  
    }
    
    int get_number_of_machines() {
        return 10;  
    }
    
    std::vector<int> get_processing_times(int number_of_jobs) {
        
        std::vector<int> times(number_of_jobs);
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> distrib(1, 24);
        
        for (int i = 0; i < number_of_jobs; i++) {
            times[i] = distrib(gen);
        }
        return times;
    }
    
    
    // ============================================
    // MILP PARAMETERS IMPLEMENTATIONS
    // ============================================
    
    void MILPParameters::print() const {
        std::cout << "\nMILP Solver Parameters:" << std::endl;
        std::cout << "  Time limit: " << time_limit << " seconds" << std::endl;
        std::cout << "  MIP gap: " << mip_gap << " (" << (mip_gap * 100) << "%)" << std::endl;
    }
    
    MILPParameters get_milp_parameters() {
        MILPParameters params;
        params.time_limit = 3600.0;  // in seconds
        params.mip_gap = 0.0;  
        return params;
    }
} 