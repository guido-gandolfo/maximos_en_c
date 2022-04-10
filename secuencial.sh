#!/bin/bash
#SBATCH -N 1
#SBATCH --exclusive
#SBATCH -o ./outputSec.txt
#SBATCH -e ./erroresSec.txt
./secuencial $1