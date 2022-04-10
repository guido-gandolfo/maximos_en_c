#!/bin/bash
#SBATCH -N 4
#SBATCH --exclusive
#SBATCH --tasks-per-node=8
#SBATCH -o ./outputPar.txt
#SBATCH -e ./erroresPar.txt
mpirun -np 4 paralelo $1 $2