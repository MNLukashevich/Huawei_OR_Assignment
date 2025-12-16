CPLEXDIR = /Applications/CPLEX_Studio2211
INCDIR   = -I$(CPLEXDIR)/cplex/include -I$(CPLEXDIR)/concert/include -I./include -I./include/tools -I./tests -I/usr/local/include -I/opt/homebrew/include
LIBS     = $(CPLEXDIR)/cplex/lib/arm64_osx/static_pic/libcplex.a \
           $(CPLEXDIR)/cplex/lib/arm64_osx/static_pic/libilocplex.a \
           $(CPLEXDIR)/concert/lib/arm64_osx/static_pic/libconcert.a \
           -lm -lpthread

CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wno-deprecated-declarations

SRC_DIR = src
TEST_DIR = tests
OBJ_DIR = obj
BIN_DIR = bin
RESULTS_DIR = results

# Source files for main program
MAIN_SRCS = $(SRC_DIR)/main.cpp \
            $(SRC_DIR)/models/milp_solver.cpp \
            $(SRC_DIR)/models/pseudo_polynomial.cpp \
            $(SRC_DIR)/io/input_data.cpp \
            $(SRC_DIR)/io/output_writer_json.cpp

# Test files
TEST_SRCS = $(wildcard $(TEST_DIR)/*.cpp)

# Object files for main program
MAIN_OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(MAIN_SRCS))
# Object files for tests
TEST_OBJS = $(patsubst $(TEST_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(TEST_SRCS))

# Executables
MAIN_TARGET = $(BIN_DIR)/huawei
TEST_TARGET = $(BIN_DIR)/huawei_tests

# Main program
$(MAIN_TARGET): $(MAIN_OBJS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)
	@echo "✓ Main program compiled: $(MAIN_TARGET)"

# Test program
$(TEST_TARGET): $(filter-out $(OBJ_DIR)/main.o, $(MAIN_OBJS)) $(TEST_OBJS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)
	@echo "✓ Test runner compiled: $(TEST_TARGET)"

# Default target
all: $(MAIN_TARGET)

# Compile main source files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCDIR) -c $< -o $@

# Compile test files
$(OBJ_DIR)/%.o: $(TEST_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCDIR) -c $< -o $@

# Run main program
run: $(MAIN_TARGET)
	@echo "Running Huawei Job Scheduling..."
	@echo "================================"
	./$(MAIN_TARGET)

# Run tests
test: $(TEST_TARGET)
	@echo "Running Huawei Test Suite..."
	@echo "============================"
	./$(TEST_TARGET)

# ============================================
# TABLE GENERATOR SECTION (STANDALONE TOOL)
# ============================================

# Table generator tool - нужно ДВА файла: main + implementation
TABLE_MAIN_SRC = src/tools/main_table_generator.cpp  # НОВЫЙ файл с main()
TABLE_GEN_SRC = src/tools/table_generator.cpp       # Реализация класса
TABLE_MAIN_OBJ = obj/tools/main_table_generator.o
TABLE_GEN_OBJ = obj/tools/table_generator.o
TABLE_TARGET = bin/generate_table

# Build standalone table generator
$(TABLE_TARGET): $(TABLE_MAIN_OBJ) $(TABLE_GEN_OBJ)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^
	@echo "✓ Standalone table generator compiled: $(TABLE_TARGET)"

# Compile table generator main (файл с main())
$(TABLE_MAIN_OBJ): $(TABLE_MAIN_SRC)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -I./include -I./include/tools -c $< -o $@

# Compile table generator implementation
$(TABLE_GEN_OBJ): $(TABLE_GEN_SRC)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -I./include -I./include/tools -c $< -o $@

# Generate table command (standalone tool)
generate-table: $(TABLE_TARGET)
	@echo "Running standalone table generator..."
	@echo "===================================="
	./$(TABLE_TARGET)

# ============================================
# RESULT ANALYSIS COMMANDS
# ============================================

# Show statistics about results
stats:
	@echo "=== Results Statistics ==="
	@find $(RESULTS_DIR) -name "*.json" -type f 2>/dev/null | wc -l | xargs echo "Total JSON files:"
	@find $(RESULTS_DIR) -name "*.tex" -type f 2>/dev/null | wc -l | xargs echo "LaTeX tables:"
	@find $(RESULTS_DIR) -name "*_summary.txt" -type f 2>/dev/null | wc -l | xargs echo "Summary files:"
	@echo ""
	@echo "Use 'make list-results' to see all result files"
	@echo "Use 'make generate-table' to generate LaTeX tables"

# List all result files
list-results:
	@echo "=== All Result Files in $(RESULTS_DIR) ==="
	@count=0; \
	for file in $(RESULTS_DIR)/*.json $(RESULTS_DIR)/*.tex $(RESULTS_DIR)/*.txt 2>/dev/null; do \
		if [ -f "$$file" ]; then \
			basename=$$(basename "$$file"); \
			size=$$(wc -c < "$$file" 2>/dev/null || echo 0); \
			modified=$$(stat -f "%Sm" "$$file" 2>/dev/null || stat -c "%y" "$$file" 2>/dev/null | cut -c1-16); \
			printf "  %-40s %8d bytes  %s\n" "$$basename" $$size "$$modified"; \
			count=$$((count+1)); \
		fi; \
	done; \
	if [ $$count -eq 0 ]; then \
		echo "  No result files found"; \
	else \
		echo ""; \
		echo "  Total: $$count files"; \
	fi

# Quick view of latest result
latest:
	@echo "=== Latest Result File ==="
	@latest=$$(find $(RESULTS_DIR) -name "*.json" -type f 2>/dev/null | sort -r | head -1); \
	if [ -n "$$latest" ]; then \
		echo "File: $$(basename $$latest)"; \
		echo "Size: $$(wc -c < $$latest 2>/dev/null || echo 0) bytes"; \
		echo "Modified: $$(stat -f "%Sm" "$$latest" 2>/dev/null || stat -c "%y" "$$latest" 2>/dev/null | cut -c1-19)"; \
		echo ""; \
		echo "Content preview:"; \
		echo "---------------"; \
		grep -E "(test_name|number_of_jobs|number_of_machines|makespan|solution_time|status|gap)" "$$latest" | head -10 | sed 's/^/  /'; \
	else \
		echo "No result files found"; \
	fi

# Clean results (careful!)
clean-results:
	@echo "WARNING: This will delete ALL files in $(RESULTS_DIR)/"
	@echo "Are you sure? [y/N] " && read ans && [ $${ans:-N} = y ]
	rm -rf $(RESULTS_DIR)/*
	@echo "✓ Results directory cleaned"

# Backup results
backup:
	@timestamp=$$(date +%Y%m%d_%H%M%S); \
	backup_dir="backup_$$timestamp"; \
	mkdir -p "$$backup_dir"; \
	cp -r $(RESULTS_DIR)/* "$$backup_dir"/ 2>/dev/null || true; \
	echo "✓ Results backed up to: $$backup_dir"

# ============================================
# UTILITY TARGETS
# ============================================

# Clean build files
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
	@echo "✓ Cleaned build files"

# Clean everything including results
clean-all: clean
	rm -rf $(RESULTS_DIR)
	@echo "✓ Cleaned everything including results"

# Debug build
debug: CXXFLAGS = -std=c++17 -g -O0 -Wall -Wno-deprecated-declarations
debug: clean $(MAIN_TARGET)
	@echo "✓ Debug build complete"

# Build everything (main, tests, table generator)
build-all: $(MAIN_TARGET) $(TEST_TARGET) $(TABLE_TARGET)
	@echo "✓ Built all programs:"
	@echo "  - $(MAIN_TARGET) (main program)"
	@echo "  - $(TEST_TARGET) (test runner)"
	@echo "  - $(TABLE_TARGET) (table generator)"

# Create directory structure
init:
	@mkdir -p $(SRC_DIR)/models $(SRC_DIR)/io $(SRC_DIR)/tools \
		$(TEST_DIR) \
		include/models include/io include/tools \
		$(OBJ_DIR)/models $(OBJ_DIR)/io $(OBJ_DIR)/tools $(OBJ_DIR)/tests \
		$(BIN_DIR) $(RESULTS_DIR)
	@echo "✓ Created directory structure"

# Help
help:
	@echo "HUAWEI JOB SCHEDULING - MAKEFILE HELP"
	@echo "======================================"
	@echo ""
	@echo "BUILD TARGETS:"
	@echo "  make all           - Build main program (default)"
	@echo "  make run           - Build and run main program"
	@echo "  make test          - Build and run tests"
	@echo "  make build-all     - Build all programs (main, tests, table generator)"
	@echo "  make clean         - Clean build files"
	@echo "  make clean-all     - Clean everything including results"
	@echo "  make debug         - Build with debug flags"
	@echo "  make init          - Create directory structure"
	@echo ""
	@echo "RESULT ANALYSIS TARGETS:"
	@echo "  make generate-table - Generate LaTeX table from existing results"
	@echo "  make stats          - Show statistics about results"
	@echo "  make list-results   - List all result files with details"
	@echo "  make latest         - Show latest result file"
	@echo "  make clean-results  - Delete all results (careful!)"
	@echo "  make backup         - Backup results folder"
	@echo ""
	@echo "EXECUTABLES:"
	@echo "  ./$(MAIN_TARGET)    - Run main scheduling program"
	@echo "  ./$(TEST_TARGET)    - Run test suite"
	@echo "  ./$(TABLE_TARGET)   - Run standalone table generator"
	@echo ""
	@echo "EXAMPLES:"
	@echo "  make run                     # Run one experiment"
	@echo "  make run && make run         # Run two experiments"
	@echo "  make generate-table          # Generate tables from all results"
	@echo "  make stats && make latest    # Check results"

.PHONY: all run test clean debug build-all init help \
        generate-table stats list-results latest clean-results backup clean-all