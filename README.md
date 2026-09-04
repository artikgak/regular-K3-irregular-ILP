# ILP K3-Irregular Graph Generator

This repository contains a C++ program that formulates the search for regular $K_3$-irregular graphs as an Integer Linear Programming (ILP) problem. It generates models in the standard `.lp` format. Here I use [SCIP Optimization Suite](https://scipopt.org/) for solving (and performed some experiments with gurobi).

> **Note:** This repository is currently undergoing cleanup and restructuring to improve organization and documentation.

## Overview

A $K_3$-irregular graph is a graph where no two vertices belong to the same number of triangles (i.e., every vertex has a distinct $K_3$-degree). This program is designed to test the existence of $r$-regular $K_3$-irregular graphs of order $n$. By formulating the structural constraints as an ILP problem, we can use an exact solver like SCIP to computationally prove whether such graphs exist for specific parameter combinations, particularly for small values of $r$ and $n$ (such as $r=8$ or $r=9$).

The ILP models use several theoretical lemmas to significantly reduce the search space through symmetry breaking, bounds tightening, and specific subcase partitioning (e.g., anchoring a vertex with maximum or minimum $K_3$-degree).

**The main computational result achieved using this repository is the exhaustive proof of the non-existence of $8$-regular $K_3$-irregular graphs.** 

This project builds upon our previous work: [**Regular $K_3$-irregular graphs**](https://arxiv.org/abs/2507.18776). 

A follow-up paper detailing the specific mathematical constraints and computational methodology for $r=8$ is currently in preparation and a link will be added here once it is published on arXiv.

## How to Build

The project is built using C++ and requires the Boost Graph Library.

1. Install **Boost** (e.g., `boost_1_82_0`).
2. Open `SCIP_irreg.sln` in Visual Studio.
3. Update the `Additional Include Directories` in the project properties (`SCIP_irreg.vcxproj`) to point to your local Boost installation path.
4. Compile the project.

## Usage

1. Open `main.cpp` and modify the `GraphConfig cfg` object to set the desired graph parameters ($n$, $r$, $K_3$-degree bounds, and specific constraints like `use_split_AB` or `fixVertexInB`).
2. Run the compiled executable. It will generate a `.lp` file in the working directory (e.g., `N22_R8_K3_0_21_split_21...lp`).
3. Use SCIP to solve the generated `.lp` file.

### Example SCIP Execution:
scip -c "read N22_R8_K3_0_21_split_21.lp optimize display solution" > N22_R8_log.txt

## Repository Structure

- `main.cpp`: Entry point and configuration template.
- `generator.cpp` / `generator.h`: Contains the core logic for translating the graph theoretical constraints and lemmas into ILP equations.
- `graphhelper.h`: Utilities for graph parsing and validating SCIP solutions.
- `SCIP_Runs/`: Directory structure intended for organizing ILP solver outputs (logs, `.sol`, `.mst`, `.lp` files are ignored by git).
- `run_commands_history.txt`: Archive of specific solver command-line invocations used during computational experiments.
