#include "stdafx.h"
#include <mpi.h>
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <string>
#include <random>


int world_rank;
int world_size;
int *totalArray;
int totalMean;
int totalDifferenceSum = 0;

// print an array
template<typename Type>
void printArray(Type *array, int size)
{
	for (int i = 0; i<size; i++)
	{
		std::cout << array[i] << "\t";
	}
	std::cout << std::endl;
}

// sum an array and return the sum
template<typename Type>
Type sumArray(Type *array, int size)
{
	Type sum;
	for (int i = 0; i<size; i++)
	{
		if(i == 0)
		{
			sum = array[i];
		}
		else
		{
			sum = sum + array[i];
		}
	}
	return sum;
}

// sumDifferences
template<typename Type>
Type sumDifferences(Type *array, int size, Type overall_mean)
{
	Type sumDifference;
	for (int i = 0; i<size; i++)
	{
		Type differenceSquare = (array[i] - overall_mean) * (array[i] - overall_mean);
		if (i == 0)
		{
			sumDifference = differenceSquare;
		}
		else
		{
			sumDifference = sumDifference + differenceSquare;
		}
	}
	return sumDifference;
}

// generate random numbers between fMin and fMax
double fRand(double fMin, double fMax)
{
	double f = (double)rand() / RAND_MAX;
	return fMin + f * (fMax - fMin);
}


/* master node method */
void coordinator(int array_size)
{
	double start, end;
	start = MPI_Wtime();

	srand(1);
	totalArray = new int[array_size];
	for (int i = 0; i < array_size; i++)
	{
		totalArray[i] = fRand(0, 51);
	}

	// calculate the size for each node
	int size_for_each_node = array_size / world_size;

	// allocate the partition for this node
	int *partition = new int[size_for_each_node];

	// broadcast the partition size
	MPI_Bcast(&size_for_each_node, 1, MPI_INT, 0, MPI_COMM_WORLD);

	// scatter the partition to each node
	MPI_Scatter(totalArray, size_for_each_node, MPI_INT, partition, size_for_each_node,
		MPI_INT, 0, MPI_COMM_WORLD);

	// sum up the values in this partition
	int sum = sumArray(partition, size_for_each_node);

	// calulate the mean for this node
	int mean = sum / size_for_each_node;

	// reduce mean to totalMean
	MPI_Reduce(&mean, &totalMean, 1, MPI_LONG_LONG_INT, MPI_SUM, 0, MPI_COMM_WORLD);

	// calulate overall mean
	int overallAverage = totalMean / world_size;

	// broadcast the overall mean to every node
	MPI_Bcast(&overallAverage, 1, MPI_INT, 0, MPI_COMM_WORLD);

	// calulate the sum of the differences
	int sumDiff = sumDifferences(partition, size_for_each_node, overallAverage);

	// reduce the sum to totalDifferenceSum
	MPI_Reduce(&sumDiff, &totalDifferenceSum, 1, MPI_LONG_LONG_INT, MPI_SUM, 0, MPI_COMM_WORLD);

	// calculate the standard deviation
	double standardDeviation = sqrt(totalDifferenceSum / array_size);

	// get the end time
	end = MPI_Wtime();
	// std::cout << "Time spend: " << end-start << std::endl;
	// print the array and output the results
	// printArray(totalArray, array_size);
	std::cout << end - start << std::endl;
	//std::cout << "Total mean: " << overallAverage << std::endl;
	//std::cout << "Standard Deviation: " << standardDeviation << std::endl;


	// clean up memory
	delete partition;
}

/* slave node method */
void participant()
{
	int size = 0;
	// receive partition size
	MPI_Bcast(&size, 1, MPI_INT, 0, MPI_COMM_WORLD);
	int *partition = new int[size];

	// receive the partition of the total array this node works on
	MPI_Scatter(totalArray, size, MPI_INT, partition, size,
		MPI_INT, 0, MPI_COMM_WORLD);

	// calculate the sum
	int sum = sumArray(partition, size);
	// calculate the mean
	int mean = sum / size;

	// reduce mean to totalMean
	MPI_Reduce(&mean, &totalMean, 1, MPI_LONG_LONG_INT, MPI_SUM, 0, MPI_COMM_WORLD);

	// get the overall average from coordinator
	int overallAverage = 0;
	MPI_Bcast(&overallAverage, 1, MPI_INT, 0, MPI_COMM_WORLD);

	// calulate the sum of the differences
	int sumDiff = sumDifferences(partition, size, overallAverage);

	// reduce the sum to totalDifferenceSum
	MPI_Reduce(&sumDiff, &totalDifferenceSum, 1, MPI_LONG_LONG_INT, MPI_SUM, 0, MPI_COMM_WORLD);

	// clean up memory
	delete partition;
}



int main(int argc, char** argv) 
{
	MPI_Init(NULL, NULL);

	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

	int array_size = 0;
	// int *totalArray;


	/* get the number of iterations*/
	sscanf_s(argv[1], "%d", &array_size);
	// sscanf_s(argv[2], "%d", &totalArray);

	if(array_size == 0)
	{
		array_size = 120;
	}
	
	if (!(array_size % world_size == 0))
	{
		if (world_rank == 0)
		{
			std::cout << "Error: array size % world_size-1 != 0" << std::endl;
		}
		MPI_Finalize();
		return 0;
	}

	// master node 
	if (world_rank == 0)
	{
		coordinator(array_size);
	}
	// other nodes
	else
	{
		participant();
	}

	MPI_Finalize();
	return 0;
}