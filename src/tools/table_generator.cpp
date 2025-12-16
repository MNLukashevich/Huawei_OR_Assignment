#include "tools/table_generator.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <limits>
#include <vector>
#include <map>
#include <utility>
#include <cmath>

using namespace std;

// ============================================
// PRIVATE HELPER METHODS IMPLEMENTATION
// ============================================

double TableGenerator::extractDouble(const string& json, const string& key) {
    size_t pos = json.find("\"" + key + "\":");
    if (pos == string::npos) return -1.0;
    
    pos += key.length() + 3;
    // Skip whitespace
    while (pos < json.size() && isspace(json[pos])) pos++;
    
    size_t end = pos;
    while (end < json.size() && json[end] != ',' && json[end] != '}' && json[end] != '\n') {
        end++;
    }
    
    string value = json.substr(pos, end - pos);
    // Clean the value
    value.erase(remove(value.begin(), value.end(), '"'), value.end());
    value.erase(remove(value.begin(), value.end(), ' '), value.end());
    value.erase(remove(value.begin(), value.end(), '\t'), value.end());
    
    try {
        return stod(value);
    } catch (...) {
        return -1.0;
    }
}

int TableGenerator::extractInt(const string& json, const string& key) {
    return static_cast<int>(extractDouble(json, key));
}

bool TableGenerator::extractBool(const string& json, const string& key) {
    size_t pos = json.find("\"" + key + "\":");
    if (pos == string::npos) return false;
    
    pos += key.length() + 3;
    // Skip whitespace
    while (pos < json.size() && isspace(json[pos])) pos++;
    
    string value = json.substr(pos, 5);
    return value.find("true") != string::npos;
}

string TableGenerator::extractString(const string& json, const string& key) {
    size_t pos = json.find("\"" + key + "\":");
    if (pos == string::npos) return "";
    
    pos += key.length() + 3;
    // Skip whitespace
    while (pos < json.size() && isspace(json[pos])) pos++;
    
    if (json[pos] != '"') return "";
    pos++;
    
    size_t end = json.find('"', pos);
    if (end == string::npos) return "";
    
    return json.substr(pos, end - pos);
}

// Helper to find value in specific section (e.g., "algorithms/milp/makespan" or "comparison/speedup")
double TableGenerator::extractDoubleInSection(const string& json, const string& section_path) {
    vector<string> parts;
    stringstream ss(section_path);
    string part;
    
    while (getline(ss, part, '/')) {
        parts.push_back(part);
    }
    
    if (parts.empty()) return -1.0;
    
    // Find the main section
    size_t section_pos = json.find("\"" + parts[0] + "\":");
    if (section_pos == string::npos) return -1.0;
    
    // If we have only 2 parts (section/key), e.g., "comparison/speedup"
    if (parts.size() == 2) {
        // Find the key in the current section
        string key_to_find = "\"" + parts[1] + "\":";
        size_t key_pos = json.find(key_to_find, section_pos);
        if (key_pos == string::npos) return -1.0;
        
        key_pos += key_to_find.length();
        // Skip whitespace
        while (key_pos < json.size() && isspace(json[key_pos])) key_pos++;
        
        size_t end = key_pos;
        while (end < json.size() && json[end] != ',' && json[end] != '}' && json[end] != '\n') {
            end++;
        }
        
        string value = json.substr(key_pos, end - key_pos);
        // Clean the value
        value.erase(remove(value.begin(), value.end(), '"'), value.end());
        value.erase(remove(value.begin(), value.end(), ' '), value.end());
        value.erase(remove(value.begin(), value.end(), '\t'), value.end());
        
        try {
            return stod(value);
        } catch (...) {
            return -1.0;
        }
    }
    // If we have 3 parts (section/subsection/key), e.g., "algorithms/milp/makespan"
    else if (parts.size() >= 3) {
        // Find the subsection
        string subsection_key = "\"" + parts[1] + "\":";
        size_t subsection_pos = json.find(subsection_key, section_pos);
        if (subsection_pos == string::npos) return -1.0;
        
        // Now find the key within this subsection
        string key_to_find = "\"" + parts[2] + "\":";
        size_t key_pos = json.find(key_to_find, subsection_pos);
        if (key_pos == string::npos) return -1.0;
        
        key_pos += key_to_find.length();
        // Skip whitespace
        while (key_pos < json.size() && isspace(json[key_pos])) key_pos++;
        
        size_t end = key_pos;
        while (end < json.size() && json[end] != ',' && json[end] != '}' && json[end] != '\n') {
            end++;
        }
        
        string value = json.substr(key_pos, end - key_pos);
        // Clean the value
        value.erase(remove(value.begin(), value.end(), '"'), value.end());
        value.erase(remove(value.begin(), value.end(), ' '), value.end());
        value.erase(remove(value.begin(), value.end(), '\t'), value.end());
        
        try {
            return stod(value);
        } catch (...) {
            return -1.0;
        }
    }
    
    return -1.0;
}

string TableGenerator::extractStringInSection(const string& json, const string& section_path) {
    vector<string> parts;
    stringstream ss(section_path);
    string part;
    
    while (getline(ss, part, '/')) {
        parts.push_back(part);
    }
    
    if (parts.empty()) return "";
    
    // Find the main section
    size_t section_pos = json.find("\"" + parts[0] + "\":");
    if (section_pos == string::npos) return "";
    
    // If we have only 2 parts (section/key)
    if (parts.size() == 2) {
        // Find the key in the current section
        string key_to_find = "\"" + parts[1] + "\":";
        size_t key_pos = json.find(key_to_find, section_pos);
        if (key_pos == string::npos) return "";
        
        key_pos += key_to_find.length();
        // Skip whitespace
        while (key_pos < json.size() && isspace(json[key_pos])) key_pos++;
        
        if (json[key_pos] != '"') return "";
        key_pos++;
        
        size_t end = json.find('"', key_pos);
        if (end == string::npos) return "";
        
        return json.substr(key_pos, end - key_pos);
    }
    // If we have 3 parts (section/subsection/key)
    else if (parts.size() >= 3) {
        // Find the subsection
        string subsection_key = "\"" + parts[1] + "\":";
        size_t subsection_pos = json.find(subsection_key, section_pos);
        if (subsection_pos == string::npos) return "";
        
        // Now find the key within this subsection
        string key_to_find = "\"" + parts[2] + "\":";
        size_t key_pos = json.find(key_to_find, subsection_pos);
        if (key_pos == string::npos) return "";
        
        key_pos += key_to_find.length();
        // Skip whitespace
        while (key_pos < json.size() && isspace(json[key_pos])) key_pos++;
        
        if (json[key_pos] != '"') return "";
        key_pos++;
        
        size_t end = json.find('"', key_pos);
        if (end == string::npos) return "";
        
        return json.substr(key_pos, end - key_pos);
    }
    
    return "";
}

// ============================================
// MAIN METHODS
// ============================================

vector<ResultData> TableGenerator::readResultsFromDirectory(const string& directory_path) {
    vector<ResultData> results;
    
    if (!filesystem::exists(directory_path)) {
        cerr << "Directory does not exist: " << directory_path << endl;
        return results;
    }
    
    for (const auto& entry : filesystem::directory_iterator(directory_path)) {
        if (entry.path().extension() == ".json") {
            try {
                ifstream file(entry.path());
                if (!file.is_open()) {
                    cerr << "Could not open: " << entry.path() << endl;
                    continue;
                }
                
                stringstream buffer;
                buffer << file.rdbuf();
                string json_str = buffer.str();
                
                ResultData rd;
                
                // Extract problem parameters
                rd.n = TableGenerator::extractInt(json_str, "number_of_jobs");
                rd.m = TableGenerator::extractInt(json_str, "number_of_machines");
                
                // Extract algorithm results using full paths
                rd.ms_milp = TableGenerator::extractDoubleInSection(json_str, "algorithms/milp/makespan");
                rd.ms_pseudo = TableGenerator::extractDoubleInSection(json_str, "algorithms/pseudo_polynomial/makespan");
                
                // Extract other MILP data
                rd.cpu_time = TableGenerator::extractDoubleInSection(json_str, "algorithms/milp/solution_time");
                rd.gap = TableGenerator::extractDoubleInSection(json_str, "algorithms/milp/gap");
                
                // Extract comparison data - speedup is in "comparison/speedup" (2 parts)
                rd.speedup = TableGenerator::extractDoubleInSection(json_str, "comparison/speedup");
                
                // For boolean, we need to search differently
                size_t match_pos = json_str.find("\"solutions_match\":");
                if (match_pos != string::npos) {
                    match_pos += 18; // length of "\"solutions_match\":"
                    // Skip whitespace
                    while (match_pos < json_str.size() && isspace(json_str[match_pos])) match_pos++;
                    string bool_str = json_str.substr(match_pos, 5);
                    rd.solutions_match = (bool_str.find("true") != string::npos);
                } else {
                    rd.solutions_match = false;
                }
                
                // Extract statuses
                rd.status_milp = TableGenerator::extractStringInSection(json_str, "algorithms/milp/status");
                rd.status_pseudo = TableGenerator::extractStringInSection(json_str, "algorithms/pseudo_polynomial/status");
                
                // Set default values if not found
                if (rd.status_milp.empty()) {
                    rd.status_milp = (rd.ms_milp >= 0) ? "feasible" : "failed";
                }
                
                if (rd.status_pseudo.empty()) {
                    rd.status_pseudo = (rd.ms_pseudo >= 0) ? "optimal" : "failed";
                }
                
                // Calculate delta only if both values are valid
                if (rd.ms_milp >= 0 && rd.ms_pseudo >= 0) {
                    rd.delta = rd.ms_milp - rd.ms_pseudo;
                } else {
                    rd.delta = 0.0;
                }
                
                results.push_back(rd);
                
            } catch (const exception& e) {
                cerr << "Error reading " << entry.path() << ": " << e.what() << endl;
            }
        }
    }
    
    // Sort results by n, then by m
    sort(results.begin(), results.end(), 
              [](const ResultData& a, const ResultData& b) {
                  if (a.n != b.n) return a.n < b.n;
                  return a.m < b.m;
              });
    
    return results;
}

void TableGenerator::generateLatexTable(const vector<ResultData>& results, 
                                       const string& output_file) {
    ofstream file(output_file);
    if (!file.is_open()) {
        cerr << "Could not open output file: " << output_file << endl;
        return;
    }
    
    file << "\\begin{table}[ht]\n";
    file << "\\centering\n";
    file << "\\begin{tabular}{|c|c|c|c|c|c|c|c|c|c|}\n";
    file << "\\hline\n";
    file << "\\textbf{$n$} & \\textbf{$m$} & \\textbf{$ms_{milp}$} & \\textbf{$ms_{pseudo}$} & ";
    file << "\\textbf{$\\Delta$} & \\textbf{$milp_{cpu}$} & ";
    file << "\\textbf{$milp_{gap}$} & \\textbf{$milp_{status}$} & \\textbf{$speedup$} & \\textbf{$match$} \\\\\n";
    file << "\\hline\n";
    
    for (const auto& rd : results) {
        file << rd.n << " & " 
             << rd.m << " & ";
        
        // MILP makespan (show --- if -1)
        if (rd.ms_milp >= 0) {
            file << fixed << setprecision(0) << rd.ms_milp << " & ";
        } else {
            file << "--- & ";
        }
        
        // Pseudo makespan
        if (rd.ms_pseudo >= 0) {
            file << fixed << setprecision(0) << rd.ms_pseudo << " & ";
        } else {
            file << "--- & ";
        }
        
        // Delta (only if both values valid)
        if (rd.ms_milp >= 0 && rd.ms_pseudo >= 0) {
            file << showpos << fixed << setprecision(1) << rd.delta 
                 << noshowpos << " & ";
        } else {
            file << "--- & ";
        }
        
        // CPU time
        if (rd.cpu_time >= 0) {
            if (rd.cpu_time < 0.001) {
                file << scientific << setprecision(1) << rd.cpu_time << " & ";
            } else if (rd.cpu_time < 1.0) {
                file << fixed << setprecision(3) << rd.cpu_time << " & ";
            } else if (rd.cpu_time < 10.0) {
                file << fixed << setprecision(2) << rd.cpu_time << " & ";
            } else {
                file << fixed << setprecision(1) << rd.cpu_time << " & ";
            }
        } else {
            file << "--- & ";
        }
        
        // Gap (as percentage)
        if (rd.gap >= 0) {
            if (rd.gap < 0.0001) {
                file << "$\\approx$0\\% & ";
            } else if (rd.gap < 0.01) {
                file << fixed << setprecision(3) << (rd.gap * 100) << "\\% & ";
            } else {
                file << fixed << setprecision(2) << (rd.gap * 100) << "\\% & ";
            }
        } else {
            file << "--- & ";
        }
        
        // MILP Status
        file << rd.status_milp << " & ";
        
        // Speedup
        if (rd.speedup > 0) {
            if (rd.speedup < 1.0) {
                file << fixed << setprecision(3) << rd.speedup << " & ";
            } else if (rd.speedup < 10.0) {
                file << fixed << setprecision(2) << rd.speedup << " & ";
            } else if (rd.speedup < 1000.0) {
                file << fixed << setprecision(1) << rd.speedup << " & ";
            } else if (rd.speedup < 1e6) {
                file << fixed << setprecision(0) << rd.speedup << " & ";
            } else {
                file << scientific << setprecision(1) << rd.speedup << " & ";
            }
        } else {
            file << "--- & ";
        }
        
        // Match status
        file << (rd.solutions_match ? "$\\checkmark$" : "$\\times$") << " \\\\\n";
        file << "\\hline\n";
    }
    
    file << "\\end{tabular}\n";
    file << "\\caption{Comparison of MILP and Pseudo-Polynomial Algorithm Results for Job Scheduling}\n";
    file << "\\label{tab:results_comparison}\n";
    
    // Add note
    file << "\\vspace{0.2cm}\n";
    file << "\\footnotesize\n";
    file << "\\textit{Note:} $\\Delta = ms_{milp} - ms_{pseudo}$; ";
    file << "Speedup = $t_{milp}/t_{pseudo}$; ";
    file << "Gap in \\%; ";
    file << "`---' indicates missing value.\n";
    
    file << "\\end{table}\n";
    
    cout << "✓ LaTeX table generated: " << output_file << endl;
}

void TableGenerator::generateStatistics(const vector<ResultData>& results,
                                       const string& output_file) {
    ofstream file(output_file);
    if (!file.is_open()) {
        cerr << "Could not open summary file: " << output_file << endl;
        return;
    }
    
    file << "=== RESULTS SUMMARY STATISTICS ===\n";
    file << "Total experiments: " << results.size() << "\n\n";
    
    // Counters
    int milp_success = 0;
    int pseudo_success = 0;
    int match_count = 0;
    int milp_better = 0;
    int pseudo_better = 0;
    int equal_makespan = 0;
    
    // MILP status counts
    map<string, int> milp_status_counts;
    map<string, int> pseudo_status_counts;
    
    // Statistics for speedup
    double speedup_sum = 0.0;
    double min_speedup = numeric_limits<double>::max();
    double max_speedup = 0.0;
    int speedup_count = 0;
    
    for (const auto& rd : results) {
        // Count successes
        if (rd.ms_milp >= 0) milp_success++;
        if (rd.ms_pseudo >= 0) pseudo_success++;
        if (rd.solutions_match) match_count++;
        
        // Count statuses
        milp_status_counts[rd.status_milp]++;
        pseudo_status_counts[rd.status_pseudo]++;
        
        // Compare makespans if both available
        if (rd.ms_milp >= 0 && rd.ms_pseudo >= 0) {
            if (abs(rd.delta) < 0.001) {
                equal_makespan++;
            } else if (rd.delta > 0) {
                pseudo_better++;  // Pseudo has smaller makespan
            } else {
                milp_better++;    // MILP has smaller makespan
            }
        }
        
        // Speedup statistics
        if (rd.speedup > 0) {
            speedup_sum += rd.speedup;
            min_speedup = min(min_speedup, rd.speedup);
            max_speedup = max(max_speedup, rd.speedup);
            speedup_count++;
        }
    }
    
    // Write statistics
    file << "--- Success Rates ---\n";
    file << "MILP successful: " << milp_success << "/" << results.size() 
         << " (" << fixed << setprecision(1) << (milp_success * 100.0 / results.size()) << "%)\n";
    file << "Pseudo-polynomial successful: " << pseudo_success << "/" << results.size() 
         << " (" << fixed << setprecision(1) << (pseudo_success * 100.0 / results.size()) << "%)\n";
    file << "Matching solutions: " << match_count << "/" << results.size() 
         << " (" << fixed << setprecision(1) << (match_count * 100.0 / results.size()) << "%)\n\n";
    
    file << "--- Solution Quality (when both available) ---\n";
    int both_available = milp_better + pseudo_better + equal_makespan;
    if (both_available > 0) {
        file << "MILP better: " << milp_better << " (" 
             << fixed << setprecision(1) << (milp_better * 100.0 / both_available) << "%)\n";
        file << "Pseudo-polynomial better: " << pseudo_better << " (" 
             << fixed << setprecision(1) << (pseudo_better * 100.0 / both_available) << "%)\n";
        file << "Equal makespan: " << equal_makespan << " (" 
             << fixed << setprecision(1) << (equal_makespan * 100.0 / both_available) << "%)\n";
    } else {
        file << "No experiments with both algorithms successful\n";
    }
    
    file << "\n--- Speedup Statistics ---\n";
    if (speedup_count > 0) {
        double avg_speedup = speedup_sum / speedup_count;
        file << "Average speedup: ";
        if (avg_speedup < 1000) {
            file << fixed << setprecision(1) << avg_speedup << "\n";
        } else if (avg_speedup < 1e6) {
            file << fixed << setprecision(0) << avg_speedup << "\n";
        } else {
            file << scientific << setprecision(1) << avg_speedup << "\n";
        }
        file << "Min speedup: ";
        if (min_speedup < 1000) {
            file << fixed << setprecision(1) << min_speedup << "\n";
        } else if (min_speedup < 1e6) {
            file << fixed << setprecision(0) << min_speedup << "\n";
        } else {
            file << scientific << setprecision(1) << min_speedup << "\n";
        }
        file << "Max speedup: ";
        if (max_speedup < 1000) {
            file << fixed << setprecision(1) << max_speedup << "\n";
        } else if (max_speedup < 1e6) {
            file << fixed << setprecision(0) << max_speedup << "\n";
        } else {
            file << scientific << setprecision(1) << max_speedup << "\n";
        }
        file << "Experiments with speedup: " << speedup_count << "/" << results.size() 
             << " (" << fixed << setprecision(1) << (speedup_count * 100.0 / results.size()) << "%)\n";
    } else {
        file << "No speedup data available\n";
    }
    
    file << "\n--- MILP Status Distribution ---\n";
    for (const auto& [status, count] : milp_status_counts) {
        file << "  " << status << ": " << count << " (" 
             << fixed << setprecision(1) << (count * 100.0 / results.size()) << "%)\n";
    }
    
    file << "\n--- Pseudo-polynomial Status Distribution ---\n";
    for (const auto& [status, count] : pseudo_status_counts) {
        file << "  " << status << ": " << count << " (" 
             << fixed << setprecision(1) << (count * 100.0 / results.size()) << "%)\n";
    }
    
    file << "\n--- Problem Size Range ---\n";
    int min_n = numeric_limits<int>::max();
    int max_n = 0;
    int min_m = numeric_limits<int>::max();
    int max_m = 0;
    
    for (const auto& rd : results) {
        min_n = min(min_n, rd.n);
        max_n = max(max_n, rd.n);
        min_m = min(min_m, rd.m);
        max_m = max(max_m, rd.m);
    }
    
    file << "n: " << min_n << " to " << max_n << "\n";
    file << "m: " << min_m << " to " << max_m << "\n";
    file << "m/n ratio: " << fixed << setprecision(3) << (static_cast<double>(min_m)/min_n) 
         << " to " << (static_cast<double>(max_m)/max_n) << "\n";
    
    cout << "✓ Statistics generated: " << output_file << endl;
}

void TableGenerator::printConsoleTable(const vector<ResultData>& results) {
    if (results.empty()) {
        cout << "No results to display." << endl;
        return;
    }
    
    cout << "\n" << string(120, '=') << endl;
    cout << "RESULTS SUMMARY TABLE" << endl;
    cout << string(120, '=') << endl;
    
    // Header with new order: n, m, ms_milp, ms_pseudo, delta, cpu_time, gap, status_milp, speedup, match
    cout << left 
              << setw(6) << "n" 
              << setw(6) << "m" 
              << setw(12) << "MILP_ms" 
              << setw(12) << "Pseudo_ms"
              << setw(8) << "Δ"
              << setw(12) << "CPU(s)" 
              << setw(10) << "Gap(%)"
              << setw(15) << "MILP_Status"
              << setw(14) << "Speedup"
              << setw(8) << "Match"
              << endl;
    cout << string(120, '-') << endl;
    
    for (const auto& rd : results) {
        cout << left 
                  << setw(6) << rd.n 
                  << setw(6) << rd.m;
        
        // Makespans - handle -1 values
        if (rd.ms_milp >= 0) {
            cout << setw(12) << fixed << setprecision(0) << rd.ms_milp;
        } else {
            cout << setw(12) << "---";
        }
        
        if (rd.ms_pseudo >= 0) {
            cout << setw(12) << fixed << setprecision(0) << rd.ms_pseudo;
        } else {
            cout << setw(12) << "---";
        }
        
        // Delta (only if both values valid)
        if (rd.ms_milp >= 0 && rd.ms_pseudo >= 0) {
            cout << setw(8) << showpos << fixed << setprecision(1) 
                      << rd.delta << noshowpos;
        } else {
            cout << setw(8) << "---";
        }
        
        // CPU time
        if (rd.cpu_time >= 0) {
            if (rd.cpu_time < 0.001) {
                cout << setw(12) << scientific << setprecision(1) << rd.cpu_time;
            } else {
                cout << setw(12) << fixed << setprecision(3) << rd.cpu_time;
            }
        } else {
            cout << setw(12) << "---";
        }
        
        // Gap
        if (rd.gap >= 0) {
            cout << setw(10) << fixed << setprecision(2) << (rd.gap * 100);
        } else {
            cout << setw(10) << "---";
        }
        
        // MILP Status
        cout << setw(15) << rd.status_milp;
        
        // Speedup (skip 0 or negative)
        if (rd.speedup > 0) {
            if (rd.speedup < 10.0) {
                cout << setw(14) << fixed << setprecision(2) << rd.speedup;
            } else if (rd.speedup < 1000.0) {
                cout << setw(14) << fixed << setprecision(1) << rd.speedup;
            } else if (rd.speedup < 1e6) {
                cout << setw(14) << fixed << setprecision(0) << rd.speedup;
            } else {
                cout << setw(14) << scientific << setprecision(1) << rd.speedup;
            }
        } else {
            cout << setw(14) << "---";
        }
        
        // Match
        cout << setw(8) << (rd.solutions_match ? "✓" : "✗");
        
        cout << endl;
    }
    
    cout << string(120, '=') << endl;
    
    // Summary
    int total = results.size();
    int milp_success = 0;
    int pseudo_success = 0;
    int match_count = 0;
    int speedup_count = 0;
    double max_speedup = 0.0;
    double min_speedup = numeric_limits<double>::max();
    double speedup_sum = 0.0;
    
    for (const auto& rd : results) {
        if (rd.ms_milp >= 0) milp_success++;
        if (rd.ms_pseudo >= 0) pseudo_success++;
        if (rd.solutions_match) match_count++;
        if (rd.speedup > 0) {
            speedup_count++;
            speedup_sum += rd.speedup;
            max_speedup = max(max_speedup, rd.speedup);
            min_speedup = min(min_speedup, rd.speedup);
        }
    }
    
    cout << "\nSummary:\n";
    cout << "  Experiments: " << total << endl;
    cout << "  MILP successful: " << milp_success << " (" 
         << fixed << setprecision(1) << (milp_success * 100.0 / total) << "%)" << endl;
    cout << "  Pseudo successful: " << pseudo_success << " (" 
         << fixed << setprecision(1) << (pseudo_success * 100.0 / total) << "%)" << endl;
    cout << "  Matching: " << match_count << " (" 
         << fixed << setprecision(1) << (match_count * 100.0 / total) << "%)" << endl;
    
    if (speedup_count > 0) {
        double avg_speedup = speedup_sum / speedup_count;
        cout << "  Speedup (avg): ";
        if (avg_speedup < 1000) {
            cout << fixed << setprecision(1) << avg_speedup;
        } else if (avg_speedup < 1e6) {
            cout << fixed << setprecision(0) << avg_speedup;
        } else {
            cout << scientific << setprecision(1) << avg_speedup;
        }
        cout << " (min: ";
        if (min_speedup < 1000) {
            cout << fixed << setprecision(1) << min_speedup;
        } else if (min_speedup < 1e6) {
            cout << fixed << setprecision(0) << min_speedup;
        } else {
            cout << scientific << setprecision(1) << min_speedup;
        }
        cout << ", max: ";
        if (max_speedup < 1000) {
            cout << fixed << setprecision(1) << max_speedup;
        } else if (max_speedup < 1e6) {
            cout << fixed << setprecision(0) << max_speedup;
        } else {
            cout << scientific << setprecision(1) << max_speedup;
        }
        cout << ")" << endl;
    }
}