
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <mpi.h>

int numOfNumbers = 20;

//Method to generate random numbers
int* RandomNumberGeneration(){

}

void main(int argc, char *argv){
        int rc, numTasks, rank;

        //Generate random numbers
        randomNumbers = RandomNumberGeneration();

        //Check rc
        rc = MPI_Init(&argc, &argv);
        if(rc != MPI_SUCCESS){
                printf("Error starting MPI program. Terminating. \n");
                MPI_Abort(MPI_COMM_WORLD, rc);
        }

        //Sum the numbers locally
        float localSum = 0;
        int i;
        for(i = 0; i < numOfNumbers; i++){
                localSum[i] = randomNumbers[i];
        }

        //Reduce local sums into a global sum
        //Calculate the mean
        float globalSum;
        MPI_Allreduce(&localSum, &globalSum, 1, MPI_FLOAT, MPI_SUM, MPI_COMM_WORLD);
        float mean =

        //Compute local sum of the squared differences from the mean
        float localSquared = 0;
        for(i = 0; i < numOfNumbers; i++){
                localSquared += (randomNumbers[i] - mean) * (randomNumbers[i] - mean);
        }

        //Reduce global sum of the squared differences to the root process
        float globalSquared;
        MPI_Reduce(&localSquared, &globalSquared, 1, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);

        //When rank is 0, print the results
        if(rank == 0){
                float stddev = sqrt(globalSquared / (numOfNumbers * NUM_THREADS));
                printf("Mean: %f, Standard Deviation: %f\n", mean, stddev);
        }
}

