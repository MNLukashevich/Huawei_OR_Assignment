#ifndef TEST_CASES_HPP
#define TEST_CASES_HPP

#include <vector>
#include <string>

struct TestCase {
    std::vector<int> times;        // Processing times
    int m;                         // Number of machines
    std::string name;              // Test case name
    double expected_makespan;      // Expected optimal makespan
    
    void print() const;
};

std::vector<TestCase> get_validation_test_cases();

#endif