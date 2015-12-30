#!/bin/bash

#SBATCH --time=01:00:00   # walltime
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

for i in 16 8 4 2 1; do
        echo "-----------------------"
        echo $i threads
        echo "-----------------------"
        export OMP_NUM_THREADS=$i
        echo -n "OMP, "
        ./bin/ompMain.o
        echo -n "Pthread, "
        ./bin/pthreadMain.o $i
        echo -n "MPI Lin, "
        mpirun -np $i ./bin/mpiLinMain.o
        echo -n "MPI Log, "
        mpirun -np $i ./bin/mpiLogMain.o
        echo ""
done