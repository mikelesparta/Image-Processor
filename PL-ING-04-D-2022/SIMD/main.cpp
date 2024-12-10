/*
 * Main.cpp
 *
 *  Created on: Fall 2019
 */

#include <stdio.h>
#include <immintrin.h> // Required to use intrinsic functions
#include <math.h>
#include <CImg.h>

#define VECTOR_SIZE       18 // Array size. Note: It is not a multiple of 8
#define ITEMS_PER_PACKET (sizeof(__m128)/sizeof(double))
typedef double data_t;

const char* SOURCE_IMGX = "bailarina.bmp";
const char* SOURCE_IMGY = "background_V.bmp";
const char* DESTINATION_IMG = "bailarina2.bmp";

using namespace cimg_library;

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

__m128d RsrcY, GsrcY, BsrcY, RsrcX, GsrcX, BsrcX;
__m128d num256 = _mm_set1_pd(256);
__m128d resRsrc256,resGsrc256,resBsrc256;
__m128d res255MinusRsrc,res255MinusGsrc,res255MinusBsrc;
__m128d resFinR,resFinG,resFinB; //Res ffinal result after division


void filter (filter_args_t args) {
	for (uint i = 0; i < args.pixelCount; i+=ITEMS_PER_PACKET) {
        RsrcY = _mm_loadu_pd(args.pRsrcY + i);
        GsrcY = _mm_loadu_pd(args.pGsrcY + i);
        BsrcY = _mm_loadu_pd(args.pBsrcY + i);
        
        RsrcX = _mm_loadu_pd(args.pRsrcX + i);
        GsrcX = _mm_loadu_pd(args.pGsrcX + i);
        BsrcX = _mm_loadu_pd(args.pBsrcX + i);

        resRsrc256 = _mm_mul_pd(RsrcY,num256);
        resGsrc256 = _mm_mul_pd(GsrcY,num256);
        resBsrc256 = _mm_mul_pd(BsrcY,num256);

        res255MinusRsrc = _mm_sub_pd(num256,RsrcX);
        res255MinusGsrc = _mm_sub_pd(num256,GsrcX);
        res255MinusBsrc = _mm_sub_pd(num256,BsrcX);

        resFinR = _mm_div_pd(resRsrc256,res255MinusRsrc);
        resFinG = _mm_div_pd(resGsrc256,res255MinusGsrc);
        resFinB = _mm_div_pd(resBsrc256,res255MinusBsrc);

        _mm_storeu_pd(args.pRdst + i,resFinR);
        _mm_storeu_pd(args.pGdst + i,resFinG);
        _mm_storeu_pd(args.pBdst + i,resFinB);

	    for(uint j = 0; j < ITEMS_PER_PACKET;j++) {
		    if ((args.pRdst + i)[j] > 255) {(args.pRdst + i)[j] = 255; }
		    if ((args.pGdst + i)[j] > 255) {(args.pGdst + i)[j] = 255; }	
		    if ((args.pBdst + i)[j] > 255) {(args.pBdst + i)[j] = 255; }
        }
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
	pDstImage = (data_t *) _mm_malloc (filter_args.pixelCount * nCompX * sizeof(data_t),sizeof(__m128));
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

	for (uint j = 0; j < 25; j++) {
		filter(filter_args);
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
	_mm_free(pDstImage);
    return 0;
}
