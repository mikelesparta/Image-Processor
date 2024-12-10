/*
 * Main.cpp
 *
 *  Created on: Fall 2019
 */

#include <stdio.h>
#include <math.h>
#include <pthread.h>
#include <CImg.h>
#include <thread>

using namespace cimg_library;

// Data type for image components
// FIXME: Change this type according to your group assignment
typedef double data_t;



const char* SOURCE_IMGX     = "bailarina.bmp";
const char* SOURCE_IMGY      = "background_V.bmp";
const char* DESTINATION_IMG = "bailarina2.bmp";
const uint NUM_THREADS = std::thread::hardware_concurrency();

// Filter argument data type
typedef struct {
	data_t *pRsrcX; // Pointers to the R, G and B components
	data_t *pGsrcX;
	data_t *pBsrcX;
	data_t *pRsrcY; // Pointers to the R, G and B components
	data_t *pGsrcY;
	data_t *pBsrcY;
	data_t *pRdst;
	data_t *pGdst;
	data_t *pBdst;
	uint pixelCount; // Size of the image in pixels
} filter_args_t;


struct threadStruct{
	uint startRow, numberOfRows;
	filter_args_t args;
};


void* filter (void* t) {
	threadStruct* thread = (threadStruct*) t;
	threadStruct threadS = *thread;

	for (uint i =  threadS.startRow; i < threadS.args.pixelCount; i++) {
		*(threadS.args.pRdst + i) = (*(threadS.args.pRsrcY + i) * 256) / (( 255 - *(threadS.args.pRsrcX + i)) + 1);
		*(threadS.args.pGdst + i) = (*(threadS.args.pGsrcY + i) * 256) / (( 255 - *(threadS.args.pGsrcX + i)) + 1);
		*(threadS.args.pBdst + i) = (*(threadS.args.pBsrcY + i) * 256) / (( 255 - *(threadS.args.pBsrcX + i)) + 1);

		if (*(threadS.args.pRdst + i) > 255) {*(threadS.args.pRdst + i) = 255; }
	
		if (*(threadS.args.pGdst + i) > 255) {*(threadS.args.pGdst + i) = 255; }	
		
		if (*(threadS.args.pBdst + i) > 255) {*(threadS.args.pBdst + i) = 255; }
	}
    return NULL;
}

int main() {
	// Open file and object initialization
	CImg<data_t> srcImageX(SOURCE_IMGX);
	CImg<data_t> srcImageY(SOURCE_IMGY);

	filter_args_t filter_args;
	data_t *pDstImage; // Pointer to the new image pixels


	struct timespec tStart, tEnd;
	double dElapsedTimeS;

	srcImageX.display(); // Displays the source image
	uint widthX = srcImageX.width();// Getting information from the source image
	uint heightX = srcImageX.height();	
	uint nCompX = srcImageX.spectrum();// source image number of components
	         // Common values for spectrum (number of image components):
				//  B&W images = 1
				//	Normal color images = 3 (RGB)
				//  Special color images = 4 (RGB and alpha/transparency channel)
	srcImageY.display(); // Displays the source image
	uint widthY = srcImageY.width();// Getting information from the source image
	uint heightY = srcImageY.height();	
	uint nCompY = srcImageY.spectrum();

	if (widthY!= widthX || heightX != heightY || nCompX != nCompY) {
		perror("The images are not the same size");
		exit(-2);
	}		

	// Calculating image size in pixels
	filter_args.pixelCount = widthX * heightX;

	// Allocate memory space for destination image components
	pDstImage = (data_t *) malloc (filter_args.pixelCount * nCompX * sizeof(data_t));
	if (pDstImage == NULL) {
		perror("Allocating destination image");
		exit(-2);
	}

	// Pointers to the componet arrays of the source image
	filter_args.pRsrcX = srcImageX.data(); // pRcomp points to the R component array
	filter_args.pGsrcX = filter_args.pRsrcX + filter_args.pixelCount; // pGcomp points to the G component array
	filter_args.pBsrcX = filter_args.pGsrcX + filter_args.pixelCount; // pBcomp points to B component array

	filter_args.pRsrcY = srcImageY.data(); // pRcomp points to the R component array
	filter_args.pGsrcY = filter_args.pRsrcY + filter_args.pixelCount; // pGcomp points to the G component array
	filter_args.pBsrcY = filter_args.pGsrcY + filter_args.pixelCount; // pBcomp points to B component array
	
	// Pointers to the RGB arrays of the destination image
	filter_args.pRdst = pDstImage;
	filter_args.pGdst = filter_args.pRdst + filter_args.pixelCount;
	filter_args.pBdst = filter_args.pGdst + filter_args.pixelCount;


    uint sections = filter_args.pixelCount / NUM_THREADS; //Sections in which the image is divided according to the number of threads
	pthread_t threads[NUM_THREADS]; //Array of threads to be used to execute the algorithm in parallel
	struct threadStruct threadStructs[NUM_THREADS]; //Array of our own structs used to give the needed details to the threads to be used


	if(clock_gettime(CLOCK_REALTIME, &tStart)){
		perror("clock_gettime");
		exit(EXIT_FAILURE);
	}

	for(uint j = 0; j < 25; j++) {
		for(uint i = 0; i < NUM_THREADS; i++){ //Repeating the process for each thread to be created
			threadStructs[i] = {}; //Making all the values of the struct 0 to make it easier for us to give the values of the attributes
			if(i != 0){  //Placing the corresponding start row to the threads
				threadStructs[i].startRow = threadStructs[i-1].startRow + threadStructs[i-1].numberOfRows + 1;
			}
		
			threadStructs[i].numberOfRows = sections; //Places the correct number of rows in the threads			

			if( i == NUM_THREADS-1){ //Fixes the number of rows corresponding to the last thread by assigning to it the remaining pixels
				threadStructs[i].numberOfRows = filter_args.pixelCount - threadStructs[i].startRow;
			}

			threadStructs[i].args = filter_args;

			if(pthread_create(&threads[i], NULL, filter, &threadStructs[i])!=0){ //Creates the thread with the given values
				perror("creating thread");
				exit(EXIT_FAILURE);
			}
		}
    }
	
	
	for(uint i = 0; i< NUM_THREADS; i++){ //Iterates over the array of threads joining them
		if(0!=pthread_join(threads[i],NULL)){
			perror("join");
			exit(EXIT_FAILURE);
		}
	}

	if(clock_gettime(CLOCK_REALTIME, &tEnd)){
		perror("clock_gettime");
		exit(EXIT_FAILURE);
	}
	

	dElapsedTimeS = (tEnd.tv_sec - tStart.tv_sec);
	dElapsedTimeS += (tEnd.tv_nsec - tStart.tv_nsec)/1e+9;
	printf("Elapsed time	: %f s.\n", dElapsedTimeS);
	// Create a new image object with the calculated pixels
	// In case of normal color images use nComp=3,
	// In case of B/W images use nComp=1.
	CImg<data_t> dstImage(pDstImage, widthX, heightX, 1, nCompX);

	// Store destination image in disk
	dstImage.save(DESTINATION_IMG); 

	// Display destination image
	dstImage.display();
	
	// Free memory
	free(pDstImage);

	return 0;
}