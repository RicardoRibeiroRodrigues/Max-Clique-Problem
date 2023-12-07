#!/bin/bash
#SBATCH --job-name=MCP_MPI
#SBATCH -n 8

mpirun -np 8 --alow-run-as-root ./mpi_mcp