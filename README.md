# Job Scheduling Optimization

This project aims to solve a job scheduling problem, where jobs need to be assigned to machines in a way that minimizes the maximal completion time (makespan). The goal is to balance the workload across machines efficiently and optimize the overall scheduling process. The project implements both Mixed Integer Linear Programming (MILP) and a pseudo-polynomial algorithm to solve the problem.

## Table of Contents

- [Project Description](#project-description)
- [Requirements](#requirements)
- [Installation](#installation)
- [Building the Project](#building-the-project)
- [Makefile Commands](#makefile-commands)
- [Running the Program](#running-the-program)
- [Testing](#testing)
- [Results Analysis](#results-analysis)
- [Backup and Clean](#backup-and-clean)

## Project Description

The task is to optimize the scheduling of jobs between machines such that the makespan (maximum load) across all machines is minimized. The project includes a MILP model and a pseudo-polynomial algorithm to solve the problem.

### Problem Statement:
- You are given `n` jobs with processing times `t_i` and `m` machines.
- Each machine can process only one job at a time.
- Jobs must be assigned to machines such that the processing times on each machine are as balanced as possible.

The solution aims to minimize the maximal completion time (makespan).

### Solution Methods:
1. **MILP Optimization Model:** A Mixed Integer Linear Programming model to solve the problem.
2. **Pseudo-polynomial Algorithm:** A heuristic approach based on binary search and feasibility checks.

## Requirements

- **C++17 or later**
- **CPLEX Optimization Studio** (for the MILP model)
- **g++** (for compiling the code)

### Dependencies:
- **CPLEX Libraries:** Ensure that the CPLEX libraries are installed and the `CPLEXDIR` is correctly set in the `Makefile`.

## Installation

1. Clone the repository:

   ```bash
   git clone https://github.com/MNLukashevich/Huawei_OR_Assignment.git
   cd Huawei_OR_Assignment
