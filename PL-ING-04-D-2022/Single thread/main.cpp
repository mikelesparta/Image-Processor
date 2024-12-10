/*
 * Main.cpp
 *
 *  Created on: Fall 2019
 */

#include <stdio.h>
#include <math.h>
#include <CImg.h>

using namespace cimg_library;

// Data type for image components
typedef double data_t;

const char* SOURCE_IMGX     = "bailarina.bmp";
const char* SOURCE_IMGY      = "background_V.bmp";
const char* DESTINATION_IMG = "bailarina2.bmp";

// Filter argument data type
typedef struct {
	data_t *pRsrcX; // Pointers to the R, G and B components image X
	data_t *pGsrcX;
	data_t *pBsrcX;
	data_t *pRsrcY; // Pointers to the R, G and B components image Y
	data_t *pGsrcY;
	data_t *pBsrcY;
	data_t *pRdst;	// Pointers to the R, G and B components image destination
	data_t *pGdst;
	data_t *pBdst;
	uint pixelCount; // Size of the image in pixels
} filter_args_t;


void filter (filter_args_t args) {
    for (uint i = 0; i < args.pixelCount; i++) {
		*(args.pRdst + i) = (*(args.pRsrcY + i) * 256) / (( 255 - *(args.pRsrcX + i)) + 1);
		*(args.pGdst + i) = (*(args.pGsrcY + i) * 256) / (( 255 - *(args.pGsrcX + i)) + 1);
		*(args.pBdst + i) = (*(args.pBsrcY + i) * 256) / (( 255 - *(args.pBsrcX + i)) + 1);

		if (*(args.pRdst + i) > 255) {*(args.pRdst + i) = 255; }
	
		if (*(args.pGdst + i) > 255) {*(args.pGdst + i) = 255; }	
		
		if (*(args.pBdst + i) > 255) {*(args.pBdst + i) = 255; }
	}
}

int main() {
	// Open file and object initialization
	CImg<data_t> srcImageX(SOURCE_IMGX);
	CImg<data_t> srcImageY(SOURCE_IMGY);

	filter_args_t filter_args;
	data_t *pDstImage; // Pointer to the new image pixels

	/***************************************************
	 * TODO: Variables initialization.
	 *   - Prepare variables for the algorithm
	 *   - This is not included in the benchmark time
	 */
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

	if(clock_gettime(CLOCK_REALTIME, &tStart)){
		perror("clock_gettime");
		exit(EXIT_FAILURE);
	}

	for(uint j = 0; j < 25; j++) {
		filter(filter_args);
		}

	if(clock_gettime(CLOCK_REALTIME, &tEnd)){
		perror("clock_gettime");
		exit(EXIT_FAILURE);
	}
	

	dElapsedTimeS = (tEnd.tv_sec - tStart.tv_sec);
	dElapsedTimeS = (tEnd.tv_nsec - tStart.tv_nsec)/1e+9;
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
