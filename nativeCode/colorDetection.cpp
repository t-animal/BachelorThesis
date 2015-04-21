#include "colorDetection.h"

#include <opencv2/imgproc/imgproc.hpp>

#include "evaluation.h"

using namespace std;
using namespace cv;

void ColorDetector::getColors(uchar *pieces){
	rectangle(src, Point(src.cols/2-20, src.rows/2-20), Point(src.cols/2+20, src.rows/2+20), Scalar(0,0,0), -1);
	floodFill(src, noArray(), Point(src.cols/2, src.rows/2), Scalar(120), NULL);
	//todo:rectangle dinge!
	//todo const garantieren!
	uchar *start = src.datastart;
	while(start != src.dataend){
		if(*start == 0)
			*start = 255;

		if(*start != 255)
			*start = 0;

		start++;
	}

	//optimized values from "zusammenfassung_colors_best.yml"
	int speckleSize = Evaluater::conf("COLORS_SPECKLESIZE", 1L);
	int rectSize = Evaluater::conf("COLORS_RECTSIZE", 31L);
	int blackThresh = Evaluater::conf("COLORS_BLACKTHRESH", 76L);
	int whiteThresh = Evaluater::conf("COLORS_WHITETHRESH", 224L);

	erode(src, src, Mat(), Point(-1, -1), 2);
	dilate(src, src, Mat(), Point(-1, -1), 2);

	int curPiece = 0;
	for(auto i : intersections){
		Mat subPix;
		getRectSubPix(src, Size(20,20), i, subPix);
		if((sum(subPix)/400)[0] < blackThresh){
			pieces[curPiece++] = 'b';//sum(subPix)[0]/100*(-1);
			continue;
		}

		if((sum(subPix)/400)[0] > whiteThresh){
			pieces[curPiece++] = 'w';//sum(subPix)[0]/100;
			continue;
		}

		pieces[curPiece++] = '0';
	}
}
