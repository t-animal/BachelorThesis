#include "colorDetection.h"

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>

#include "evaluation.h"

using namespace std;
using namespace cv;

void ColorDetector::getColors(uchar *pieces) {
	Mat mask;
	src.copyTo(mask);

	rectangle(mask, Point(src.cols / 2 - 20, src.rows / 2 - 20), Point(src.cols / 2 + 20, src.rows / 2 + 20),
			Scalar(0, 0, 0), -1);
	floodFill(mask, noArray(), Point(src.cols / 2, src.rows / 2), Scalar(120), NULL);
	bitwise_xor(src, mask, src);

	uchar *start = src.datastart;
	while (start != src.dataend) {
		if (*start == 120)
			*start = 0;
		else
			*start = 255;

		start++;
	}

	int speckleSize = Evaluater::conf("COLORS_SPECKLESIZE", 1L);
	int rectSize = Evaluater::conf("COLORS_RECTSIZE", 20L);
	int blackThresh = Evaluater::conf("COLORS_BLACKTHRESH", 76L);
	int whiteThresh = Evaluater::conf("COLORS_WHITETHRESH", 215L);
	int borderLower = Evaluater::conf("COLORS_BORDERLOWERING", 35L);

	erode(src, src, Mat(), Point(-1, -1), speckleSize);
	dilate(src, src, Mat(), Point(-1, -1), speckleSize);

	int curPiece = 0;
	for (auto i : intersections) {
		Mat subPix;
		getRectSubPix(src, Size(rectSize, rectSize), i, subPix);
		float pixelSum = (float)sum(subPix)[0] / rectSize/rectSize;
		if (pixelSum < blackThresh) {
			pieces[curPiece++] = 'b'; //sum(subPix)[0]/100*(-1);
			continue;
		}

		bool isBorder = curPiece / 9 == 0 || curPiece / 9 == 8 || curPiece % 9 == 0 || curPiece % 9 == 8;
		if(isBorder)
			cout << pixelSum << " " << (pixelSum > whiteThresh + borderLower) << endl;
		if (pixelSum > (whiteThresh + (isBorder?borderLower:0))) {
			pieces[curPiece++] = 'w'; //sum(subPix)[0]/100;
			continue;
		}

		pieces[curPiece++] = '0';
	}
}
