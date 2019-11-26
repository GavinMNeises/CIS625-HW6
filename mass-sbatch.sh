for x in 2
do
	sbatch --mem-per-cpu=4G --constraint=elves --ntasks-per-node=2 --nodes=$x --time=00-00:01:00 mpi_omp_sbatch.sh $x
done;

