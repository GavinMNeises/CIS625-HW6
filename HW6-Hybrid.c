#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <mpi.h>
#include <omp.h>
#include <math.h>

//Method to generate random numbers
int* RandomNumberGeneration(long numOfNumbers){
	int i = 0;
	int* randomNumberArray;
	randomNumberArray = (int *) malloc(sizeof(int) * numOfNumbers);
	
	for(i = 0; i < numOfNumbers; i++){
		randomNumberArray[i] = (int)(100 * (rand()/(double)RAND_MAX));
	}
	
	return randomNumberArray;
}

void main(int argc, char *argv[]){
	int rc, numMPITasks, rank, numOMPTasks = atoi(argv[2]);
	
	long i, localNumOfNumbers, numOfNumbers = atoi(argv[1]); 
	double globalSum, localSum = 0, standardDev, globalMean, globalSumOfSquares, localMean, localSumOfSquares = 0;

	struct timeval t1, t2;
	double timeElapsed;
	
	//Set up MPI
	MPI_Init(NULL, NULL);
	MPI_Comm_size(MPI_COMM_WORLD, &numMPITasks);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	printf("Number of tasks= %d, My rank= %d\n", numMPITasks, rank);
	
	localNumOfNumbers = (numOfNumbers/numMPITasks);
	
	//Set up OMP
	omp_set_num_threads(numOMPTasks);
	
	if(!rank) {
		gettimeofday(&t1, NULL);
	}
	
	//Generate random numbers
	int* randomNumbers = RandomNumberGeneration(localNumOfNumbers);
	
	int id;
	long startPos, endPos, blockSum = 0, blockNumOfNums = localNumOfNumbers/numOMPTasks;
	/*for(i = 0; i < localNumOfNumbers; i++) {
			localSum += randomNumbers[i];
		}*/
	//Find local sum 
	#pragma omp parallel private(blockSum, i, id)
	{
		id = omp_get_thread_num();
		startPos = blockNumOfNums * id;
		endPos = startPos + blockNumOfNums;
		for(i = startPos; i < endPos; i++) {
			blockSum += randomNumbers[i];
		}
		#pragma omp critical
		{
			localSum += blockSum;
		}
	}
	
	//Reduce sums
	MPI_Reduce(&localSum, &globalSum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
	
	//Calculate global mean
	if(!rank) {
		globalMean = globalSum/numOfNumbers;
	}
	
	//Send global mean
	MPI_Scatter(&globalMean, 1, MPI_DOUBLE, &localMean, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	
	double blockSumOfSquares = 0;
	//Find local sum of squares
	#pragma omp parallel private(blockSumOfSquares, i, id)
	{
		id = omp_get_thread_num();
		startPos = blockNumOfNums * id;
		endPos = startPos + blockNumOfNums;
		for(i = startPos; i < endPos; i++) {
			blockSumOfSquares += pow((randomNumbers[i] - localMean), 2);
		}
		#pragma omp critical
		{
			localSumOfSquares += blockSumOfSquares;
		}
	}
	
	//Reduce sum of squares
	MPI_Reduce(&localSumOfSquares, &globalSumOfSquares, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
	
	if(!rank) {
		standardDev = sqrt((globalSumOfSquares/numOfNumbers));
		gettimeofday(&t2, NULL);
		timeElapsed = (t2.tv_sec - t1.tv_sec) * 1000.0; //sec to ms
		timeElapsed += (t2.tv_usec - t1.tv_usec) / 1000.0; //us to ms
		//globalSum, globalMean, globalSumOfSquares, standard deviation, timeElapsed
		printf("DATA, %d, %d, %d, %lf, %lf, %lf, %lf, %lf\n", numMPITasks, numOMPTasks, (numMPITasks*numOMPTasks), globalSum, localMean, globalSumOfSquares, standardDev, timeElapsed);
	}
	
	free(randomNumbers);
	MPI_Finalize();
}

