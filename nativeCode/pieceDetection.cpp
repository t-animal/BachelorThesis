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

void PieceDetector::detectPieces(Mat &src, vector<Point3f> &darkPieces, vector<Point3f> &lightPieces) {
	vector<Mat> channels;
	split(src, channels);
	Mat h = channels[0];
	Mat s = channels[1];
	Mat v = channels[2];

	//THE OPENCV DOCU HERE IS CRAP! => 0 ≤ v ≤ 255; 0 ≤ s ≤ 1; 0 ≤ h ≤ 360

	//parameter fuer V (dark Pieces) from currentTime: '2015-04-18 21:11:09.487894'
	int vGauss = Evaluater::conf("PIECES_GAUSS_V", 7L);
	int sGauss = Evaluater::conf("PIECES_GAUSS_S", 25L);

	int vThresh = Evaluater::conf("PIECES_THRESH_V", 76L);
	double sThresh = Evaluater::conf("PIECES_THRESH_S", 0.17);
	int hThresh = Evaluater::conf("PIECES_THRESH_H", 70L);

	int speckleSize = Evaluater::conf("PIECES_SPECKLES", 6L);

	GaussianBlur(v, v, Size(vGauss, vGauss), 0);
	GaussianBlur(s, s, Size(sGauss, sGauss), 0);
	GaussianBlur(h, h, Size(sGauss, sGauss), 0);

	threshold(v, v, vThresh, 255, THRESH_BINARY);
	threshold(s, s, sThresh, 1, THRESH_BINARY);
	threshold(h, h, hThresh, 360, THRESH_BINARY);

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

#ifdef CIRCLE_HOUGH

	int minDistDark = Evaluater::conf("PIECES_MINDIST_DARK", 15L);
	int minRadDark = Evaluater::conf("PIECES_MINRAD_DARK", 20L);
	int maxRadDark = Evaluater::conf("PIECES_MAXRAD_DARK", 11L);
	int minDistLight = Evaluater::conf("PIECES_MINDIST_LIGHT", 13L);
	int minRadLight = Evaluater::conf("PIECES_MINRAD_LIGHT", 30L);
	int maxRadLight = Evaluater::conf("PIECES_MAXRAD_LIGHT", 11L);

	//          (Input, Output,   method,            dp, minDist,      param1, param2, minRad,  maxRad )
	HoughCircles(h, lightPieces, CV_HOUGH_GRADIENT, 3, src.rows / minDistLight, 900, 50, src.rows / minRadLight, src.rows / maxRadLight);
	HoughCircles(v, darkPieces,  CV_HOUGH_GRADIENT, 3, src.rows / minDistDark,  900, 50, src.rows / minRadDark, src.rows / maxRadDark);
#else

	int minDiameter = Evaluater::conf("PIECES_MINDIAMETER", 9L);
	int maxDiameter = Evaluater::conf("PIECES_MAXDIAMETER", 32L);

	bitwise_not(h, h);
	bitwise_not(v, v);

	vector<vector<Point> > contours;
	findContours(v, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

	for (auto c : contours) {
		Rect bounding = boundingRect(c);

		if (bounding.height != 0 && abs(1.0 - (float) bounding.width / bounding.height) < 0.3) {

			int centerX = bounding.x + bounding.width / 2;
			int centerY = bounding.y + bounding.height / 2;
			int diameter = (bounding.width + bounding.height) / 4;

			if(diameter > minDiameter && diameter < maxDiameter)
				darkPieces.push_back(Point3f(centerX, centerY, diameter));
		}
	}

	findContours(h, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
	for (auto c : contours) {
		Rect bounding = boundingRect(c);

		if (bounding.height != 0 && abs(1.0 - (float) bounding.width / bounding.height) < 0.3) {

			int centerX = bounding.x + bounding.width / 2;
			int centerY = bounding.y + bounding.height / 2;
			int diameter = (bounding.width + bounding.height) / 4;

			if(diameter > minDiameter && diameter < maxDiameter)
				lightPieces.push_back(Point3f(centerX, centerY, diameter));

		}
	}

#endif
}
