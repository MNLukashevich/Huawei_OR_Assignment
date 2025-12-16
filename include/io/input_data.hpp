#ifndef INPUT_DATA_HPP
#define INPUT_DATA_HPP

#include <vector>
#include <string>

namespace InputData {

    // ============================================
    // PROBLEM DATA FUNCTIONS
    // ============================================

    int get_number_of_jobs();
    int get_number_of_machines();
    std::vector<int> get_processing_times(int number_of_jobs);
    
    
    // ============================================
    // MILP SOLVER PARAMETERS STRUCTURE
    // ============================================
    
    struct MILPParameters {
        double time_limit; 
        double mip_gap;         
        
        void print() const;
    };
    
    MILPParameters get_milp_parameters();
} 

#endif