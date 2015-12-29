#!/bin/bash

#SBATCH --time=00:30:00   # walltime
#SBATCH --ntasks=16 	# number of processor cores
#SBATCH --nodes=1   # number of nodes
#SBATCH --mem-per-cpu=1024M   # memory per CPU core
#SBATCH --qos=test

# Compatibility variables for PBS. Delete if not needed.
export PBS_NODEFILE=`/fslapps/fslutils/generate_pbs_nodefile`
export PBS_JOBID=$SLURM_JOB_ID
export PBS_O_WORKDIR="$SLURM_SUBMIT_DIR"
export PBS_QUEUE=batch

# LOAD MODULES, INSERT CODE, AND RUN YOUR PROGRAMS HERE
make clean
make all

for i in 1 2 4 8 16; do
        echo "-----------------------"
        echo $i threads
        echo "-----------------------"
        # echo "OMP"
        # ./bin/ompMain.o $i
        # echo "PTHREAD"
        # ./bin/pthreadMain.o $i
        echo "MPI"
        mpirun -np $i ./bin/mpiMain.o
        echo ""
done