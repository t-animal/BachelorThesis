#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <iostream>

#include "pieceDetection.h"
#include "evaluation.h"
#include "lineDetection.h"
#include "intersectionDetection.h"
#include "util.h"

using namespace cv;
using namespace std;

void PieceDetector::detectPieces(vector<Point3f> &darkPieces, vector<Point3f> &lightPieces) {
	vector<Mat> channels;
	split(src, channels);
	Mat h = channels[0];
	Mat s = channels[1];
	Mat v = channels[2];

	//THE OPENCV DOCU HERE IS CRAP! => 0 ≤ v ≤ 255; 0 ≤ s ≤ 1; 0 ≤ h ≤ 360

	//parameter fuer preprocessing von 2015-05-04 00:17:33.639373 und 2015-05-04 01:51:45.506732
	int vGauss = Evaluater::conf("PIECES_GAUSS_V", 9L);
	int sGauss = Evaluater::conf("PIECES_GAUSS_S", 23L);

	int vThresh = Evaluater::conf("PIECES_THRESH_V", 77L);
	double sThresh = Evaluater::conf("PIECES_THRESH_S", 0.22);
	int hThresh = Evaluater::conf("PIECES_THRESH_H", 195L);

	int speckleSize = Evaluater::conf("PIECES_SPECKLES", 7L);

	GaussianBlur(v, v, Size(vGauss, vGauss), 0);
	GaussianBlur(s, s, Size(sGauss, sGauss), 0);
	GaussianBlur(h, h, Size(sGauss, sGauss), 0);

	threshold(v, v, vThresh, 255, THRESH_BINARY);
	threshold(s, s, sThresh, 1, THRESH_BINARY);
	threshold(h, h, hThresh, 360, THRESH_BINARY_INV);

	//remove speckles
	dilate(h, h, Mat(), Point(-1, -1), speckleSize);
	dilate(s, s, Mat(), Point(-1, -1), speckleSize);
	dilate(v, v, Mat(), Point(-1, -1), speckleSize);
	erode(h, h, Mat(), Point(-1, -1), speckleSize);
	erode(s, s, Mat(), Point(-1, -1), speckleSize);
	erode(v, v, Mat(), Point(-1, -1), speckleSize);


	if (countNonZero(h) < h.rows * h.cols * 4 / 5) { //wenn weniger als 80% weiss, discarde
		h = Mat::ones(h.size(), h.type()) * 360;
	}
	if (countNonZero(s) < s.rows * s.cols * 4 / 5) { //wenn weniger als 80% weiss, discarde
		s = Mat::ones(s.size(), s.type());
	}

	h.convertTo((h /= 360) *= 255, CV_8UC1); //0.70833=255/360
	s.convertTo(s *= 255, CV_8UC1);
	v.convertTo(v, CV_8UC1);

	//improve performance by only performing detection once
	//TODO: increase performance further, evtl only one houghcircle somehow?
	bitwise_and(s, h, h);
#define CIRCLE_HOUGH
#ifdef CIRCLE_HOUGH

	int minDistDark = Evaluater::conf("PIECES_MINDIST_DARK", 32L);
	int minRadDark = Evaluater::conf("PIECES_MINRAD_DARK", 24L);
	int maxRadDark = Evaluater::conf("PIECES_MAXRAD_DARK", 45L);
	int minDistLight = Evaluater::conf("PIECES_MINDIST_LIGHT", 36L);
	int minRadLight = Evaluater::conf("PIECES_MINRAD_LIGHT", 16L);
	int maxRadLight = Evaluater::conf("PIECES_MAXRAD_LIGHT", 45L);
	int accuThreshLight = Evaluater::conf("PIECES_ACCUTHRESH_LIGHT", 50L);
	int accuThreshDark = Evaluater::conf("PIECES_ACCUTHRESH_DARK", 50L);

	//          (Input, Output,   method,            dp, minDist,      param1, param2, minRad,  maxRad )
	HoughCircles(h, lightPieces, CV_HOUGH_GRADIENT, 3, minDistLight, 900, accuThreshLight, minRadLight, maxRadLight);
	HoughCircles(v, darkPieces,  CV_HOUGH_GRADIENT, 3, minDistDark,  900, accuThreshDark, minRadDark, maxRadDark);
#else

	int minDiameter = Evaluater::conf("PIECES_MINDIAMETER", 9L);
	int maxDiameter = Evaluater::conf("PIECES_MAXDIAMETER", 32L);
	double maxRatio = Evaluater::conf("PIECES_MAXRATIO", 1.5);
	double minRatio = Evaluater::conf("PIECES_MINRATIO", 0.5);
	int splitRatio = Evaluater::conf("PIECES_SPLITDIFFERENCE", 5L);

	bitwise_not(h, h);
	bitwise_not(v, v);

	vector<vector<Point> > contours;
	vector<Rect> boundings;
	findContours(v, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

	for(auto &c : contours){
		Rect bounding = boundingRect(c);
		if(abs(bounding.width - bounding.height/2) < splitRatio){
			bounding.height = bounding.height/2;
			boundings.push_back(bounding);
			bounding.y += bounding.height;
			boundings.push_back(bounding);
		}else if(abs(bounding.height - bounding.width/2) < splitRatio){
			bounding.width = bounding.width/2;
			boundings.push_back(bounding);
			bounding.x += bounding.width;
			boundings.push_back(bounding);
		}else{
			boundings.push_back(bounding);
		}
	}

	for (auto b : boundings) {
		float ratio = b.height!=0? (float) b.width / b.height : 99;
		if (b.height != 0 && ratio >= 0.05 &&
				((ratio > minRatio && ratio < maxRatio) ||  (1/ratio > minRatio && 1/ratio < maxRatio))) {

			int centerX = b.x + b.width / 2;
			int centerY = b.y + b.height / 2;
			int diameter = (b.width + b.height) / 4;

			if(diameter > minDiameter && diameter < maxDiameter)
				darkPieces.push_back(Point3f(centerX, centerY, diameter));
		}
	}

	findContours(h, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
	boundings.clear();

	for(auto &c : contours){
		Rect bounding = boundingRect(c);
		if(abs(bounding.width - bounding.height/2) < splitRatio){
			bounding.height = bounding.height/2;
			boundings.push_back(bounding);
			bounding.y += bounding.height;
			boundings.push_back(bounding);
		}else if(abs(bounding.height - bounding.width/2) < splitRatio){
			bounding.width = bounding.width/2;
			boundings.push_back(bounding);
			bounding.x += bounding.width;
			boundings.push_back(bounding);
		}else{
			boundings.push_back(bounding);
		}
	}
	for (auto c : contours) {
		Rect bounding = boundingRect(c);

		float ratio = bounding.height!=0? (float) bounding.width / bounding.height : 99;
		if (bounding.height != 0 && ratio >= 0.05 &&
				((ratio > minRatio && ratio < maxRatio) ||  (1/ratio > minRatio && 1/ratio < maxRatio))) {

			int centerX = bounding.x + bounding.width / 2;
			int centerY = bounding.y + bounding.height / 2;
			int diameter = (bounding.width + bounding.height) / 4;

			if(diameter > minDiameter && diameter < maxDiameter)
				lightPieces.push_back(Point3f(centerX, centerY, diameter));

		}
	}

#endif
}
