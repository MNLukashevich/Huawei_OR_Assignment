#include "test_cases.hpp"
#include <vector>
#include <string>
#include <iostream>

std::vector<TestCase> get_validation_test_cases() {
    std::vector<TestCase> test_cases;
    
    {
        TestCase tc;
        tc.times = {2, 3, 5, 7};
        tc.m = 3;
        tc.name = "Assignment Example";
        tc.expected_makespan = 7.0;
        test_cases.push_back(tc);
    }
    
    {
        TestCase tc;
        tc.times = {1, 2, 3, 4, 5, 6};
        tc.m = 2;
        tc.name = "Simple Test 1";
        tc.expected_makespan = 11.0;
        test_cases.push_back(tc);
    }
    
    {
        TestCase tc;
        tc.times = {3, 1, 4, 2, 5};
        tc.m = 2;
        tc.name = "Simple Test 2";
        tc.expected_makespan = 8.0;
        test_cases.push_back(tc);
    }
    
    {
        TestCase tc;
        tc.times = {1, 1, 1, 1, 1};
        tc.m = 3;
        tc.name = "All Equal Times";
        tc.expected_makespan = 2.0;
        test_cases.push_back(tc);
    }
    
    {
        TestCase tc;
        tc.times = {24, 1, 1, 1, 1};
        tc.m = 2;
        tc.name = "One Large Job";
        tc.expected_makespan = 24.0;
        test_cases.push_back(tc);
    }
    
    {
        TestCase tc;
        tc.times = {8, 6, 4, 2, 10, 5, 3, 7, 9, 1};
        tc.m = 4;
        tc.name = "Medium Test 1";
        tc.expected_makespan = 16.0;
        test_cases.push_back(tc);
    }
    
    {
        TestCase tc;
        tc.times = {15, 10, 8, 12, 6, 7, 9, 5};
        tc.m = 3;
        tc.name = "Medium Test 2";
        tc.expected_makespan = 26.0;
        test_cases.push_back(tc);
    }
    
    {
        TestCase tc;
        tc.times = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        tc.m = 3;
        tc.name = "Increasing Sequence";
        tc.expected_makespan = 21.0;
        test_cases.push_back(tc);
    }
    
    {
        TestCase tc;
        tc.times = {10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
        tc.m = 3;
        tc.name = "Decreasing Sequence";
        tc.expected_makespan = 21.0;
        test_cases.push_back(tc);
    }
    
    {
        TestCase tc;
        tc.times = {5, 8, 3, 9, 2, 7, 4, 6, 1};
        tc.m = 8;
        tc.name = "n close to m";
        tc.expected_makespan = 9.0;
        test_cases.push_back(tc);
    }
    
    {
        TestCase tc;
        tc.times = {4, 4, 4, 4, 4, 4};
        tc.m = 3;
        tc.name = "Perfect Balance";
        tc.expected_makespan = 8.0;
        test_cases.push_back(tc);
    }
    
    {
        TestCase tc;
        tc.times = {7, 3, 9, 2, 8, 5, 6, 4, 1, 10, 7, 3};
        tc.m = 4;
        tc.name = "Random Medium Test";
        tc.expected_makespan = 20.0;
        test_cases.push_back(tc);
    }
    
    {
        TestCase tc;
        tc.times = {5, 8, 2, 6, 9, 3, 7, 4, 1, 5, 8, 2, 6, 9, 3, 7, 4, 1};
        tc.m = 5;
        tc.name = "Large Performance Test";
        tc.expected_makespan = 21.0;
        test_cases.push_back(tc);
    }
    
    {
        TestCase tc;
        tc.times = {1, 2, 3, 20, 3, 2, 1};
        tc.m = 3;
        tc.name = "Peak in Middle";
        tc.expected_makespan = 20.0;
        test_cases.push_back(tc);
    }
    
    {
        TestCase tc;
        tc.times = {6, 5, 4, 3, 2, 1, 7};
        tc.m = 3;
        tc.name = "Greedy Algorithm Trap";
        tc.expected_makespan = 11.0;
        test_cases.push_back(tc);
    }
    
    {
        TestCase tc;
        tc.times = {3, 3, 3, 7, 7, 7, 5, 5, 5};
        tc.m = 3;
        tc.name = "Uniform Groups";
        tc.expected_makespan = 16.0;
        test_cases.push_back(tc);
    }
    
    {
        TestCase tc;
        tc.times = {10, 1, 10, 1, 10, 1, 10, 1};
        tc.m = 3;
        tc.name = "Alternating Values";
        tc.expected_makespan = 21.0;
        test_cases.push_back(tc);
    }
    
    {
        TestCase tc;
        tc.times = {5, 5, 5, 5, 5};
        tc.m = 1;
        tc.name = "Single Machine";
        tc.expected_makespan = 25.0;
        test_cases.push_back(tc);
    }

    {
        TestCase tc;
        tc.times = {20, 15, 18, 10, 24, 16, 12, 14};
        tc.m = 2;
        tc.name = "Large Numbers Test";
        tc.expected_makespan = 66.0;
        test_cases.push_back(tc);
    }
    
    return test_cases;
}